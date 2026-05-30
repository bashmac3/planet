#include "planet/map/map_loader.h"
#include "planet/map/map_parser.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/render/mesh.h"
#include "planet/render/texture.h"
#include "planet/resource/resource_manager.h"
#include "planet/core/logger.h"
#include <lua.hpp>
#include <lauxlib.h>
#include <cstring>

namespace planet {

static std::unordered_map<std::string, std::unique_ptr<Mesh>> s_mapMeshes;

static Mesh* GetOrCreateMesh(const std::string& key, std::function<Mesh()> factory) {
    auto it = s_mapMeshes.find(key);
    if (it != s_mapMeshes.end()) return it->second.get();
    auto mesh = std::make_unique<Mesh>(factory());
    Mesh* ptr = mesh.get();
    s_mapMeshes[key] = std::move(mesh);
    return ptr;
}

MapLoader& MapLoader::Instance() {
    static MapLoader instance;
    return instance;
}

bool MapLoader::Load(const std::string& path) {
    // Detect format by extension
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".map") {
        return ParseMapFile(path);
    }
    return LoadLua(path);
}

bool MapLoader::LoadLua(const std::string& path) {
    lua_State* L = luaL_newstate();
    if (!L) {
        LOG_ERROR() << "[Map] Failed to create Lua state for loading: " << path;
        return false;
    }
    luaL_openlibs(L);

    if (luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 1, 0)) {
        LOG_ERROR() << "[Map] Error loading map file '" << path << "': " << lua_tostring(L, -1);
        lua_close(L);
        return false;
    }

    if (!lua_istable(L, -1)) {
        LOG_ERROR() << "[Map] Map file must return a table: " << path;
        lua_close(L);
        return false;
    }

    // Process environment section
    lua_getfield(L, -1, "environment");
    if (lua_istable(L, -1)) {
        ProcessEnvironment(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    // Process templates section
    lua_getfield(L, -1, "templates");
    if (lua_istable(L, -1)) {
        ProcessTemplates(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    lua_close(L);
    LOG_INFO() << "[Map] Loaded: " << path;
    return true;
}

bool MapLoader::ProcessEnvironment(lua_State* L, int idx) {
    auto& scene = Scene::Instance();

    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        if (!lua_istable(L, -1)) { lua_pop(L, 1); continue; }

        const char* name = "";
        lua_getfield(L, -1, "name");
        if (lua_isstring(L, -1)) name = lua_tostring(L, -1);
        lua_pop(L, 1);

        const char* type = "";
        lua_getfield(L, -1, "type");
        if (lua_isstring(L, -1)) type = lua_tostring(L, -1);
        lua_pop(L, 1);

        Entity* ent = scene.CreateEntity(name ? name : "EnvEntity");

        if (std::strcmp(type, "light") == 0) {
            auto* lc = ent->AddComponent<LightComponent>();

            const char* lt = "point";
            lua_getfield(L, -1, "lightType");
            if (lua_isstring(L, -1)) lt = lua_tostring(L, -1);
            lua_pop(L, 1);

            if (std::strcmp(lt, "directional") == 0) lc->type = LightComponent::Directional;
            else if (std::strcmp(lt, "spot") == 0) lc->type = LightComponent::Spot;
            else lc->type = LightComponent::Point;

            lua_getfield(L, -1, "color");
            if (lua_istable(L, -1)) {
                float r = 1, g = 1, b = 1;
                lua_rawgeti(L, -2, 1); r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_rawgeti(L, -2, 2); g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_rawgeti(L, -2, 3); b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lc->color = glm::vec3(r, g, b);
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "range");
            if (lua_isnumber(L, -1)) lc->range = (float)lua_tonumber(L, -1);
            lua_pop(L, 1);
        }

        if (std::strcmp(type, "mesh") == 0) {
            MapTemplate mt;
            ReadTemplateFromTable(L, lua_gettop(L), mt);
            if (mt.isMesh) {
                auto* mc = ent->AddComponent<MeshComponent>();
                mc->color = mt.color;
                if (mt.meshType == "cube")      mc->mesh = GetOrCreateMesh("cube_1", []{ return Mesh::CreateCube(1.0f); });
                else if (mt.meshType == "box")  mc->mesh = GetOrCreateMesh("box_1x1x1", []{ return Mesh::CreateBox(1, 1, 1); });
                else if (mt.meshType == "sphere") mc->mesh = GetOrCreateMesh("sphere_05_16", []{ return Mesh::CreateSphere(0.5f, 16); });
                else if (mt.meshType == "plane")  mc->mesh = GetOrCreateMesh("plane_1_1", []{ return Mesh::CreatePlane(1, 1); });
                else if (mt.meshType == "cylinder") mc->mesh = GetOrCreateMesh("cyl_05_1_16", []{ return Mesh::CreateCylinder(0.5f, 1.0f, 16); });
                else if (mt.meshType == "cone")    mc->mesh = GetOrCreateMesh("cone_05_1_16", []{ return Mesh::CreateCone(0.5f, 1.0f, 16); });
                else if (mt.meshType == "torus")   mc->mesh = GetOrCreateMesh("torus_05_02_16_8", []{ return Mesh::CreateTorus(0.5f, 0.2f, 16, 8); });
                else if (mt.meshType == "quad")    mc->mesh = GetOrCreateMesh("quad_1_1", []{ return Mesh::CreateQuad(1, 1); });
                if (!mt.texturePath.empty()) {
                    auto* tex = ResourceManager::Instance().LoadTexture(mt.texturePath);
                    if (tex) mc->texture = tex;
                }
                ent->SetScale(mt.scale);
            }
        }

        // Position
        lua_getfield(L, -1, "position");
        if (lua_istable(L, -1)) {
            float px = 0, py = 0, pz = 0;
            lua_rawgeti(L, -2, 1); px = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -2, 2); py = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -2, 3); pz = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            ent->SetPosition(glm::vec3(px, py, pz));
        }
        lua_pop(L, 1);

        // Rotation (euler angles)
        lua_getfield(L, -1, "rotation");
        if (lua_istable(L, -1)) {
            float rx = 0, ry = 0, rz = 0;
            lua_rawgeti(L, -2, 1); rx = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -2, 2); ry = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -2, 3); rz = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            ent->SetEulerAngles(glm::vec3(rx, ry, rz));
        }
        lua_pop(L, 1);

        lua_pop(L, 1);
    }
    return true;
}

bool MapLoader::ProcessTemplates(lua_State* L, int idx) {
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
        if (!lua_istable(L, -1)) { lua_pop(L, 1); continue; }

        const char* tplName = lua_tostring(L, -2);
        if (!tplName) { lua_pop(L, 1); continue; }

        MapTemplate tmpl;
        if (ReadTemplateFromTable(L, lua_gettop(L), tmpl)) {
            m_templates[tplName] = tmpl;
        }
        lua_pop(L, 1);
    }
    return true;
}

bool MapLoader::ReadTemplateFromTable(lua_State* L, int idx, MapTemplate& out) {
    const char* mesh = nullptr;
    lua_getfield(L, idx, "mesh");
    if (lua_isstring(L, -1)) {
        mesh = lua_tostring(L, -1);
        out.isMesh = true;
    }
    lua_pop(L, 1);

    if (mesh) out.meshType = mesh;

    lua_getfield(L, idx, "scale");
    if (lua_istable(L, -1)) {
        float sx = 1, sy = 1, sz = 1;
        lua_rawgeti(L, -2, 1); sx = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); sy = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); sz = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        out.scale = glm::vec3(sx, sy, sz);
    }
    lua_pop(L, 1);

    lua_getfield(L, idx, "color");
    if (lua_istable(L, -1)) {
        float r = 1, g = 1, b = 1, a = 1;
        lua_rawgeti(L, -2, 1); r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 4); a = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        out.color = glm::vec4(r, g, b, a);
    }
    lua_pop(L, 1);

    lua_getfield(L, idx, "texture");
    if (lua_isstring(L, -1)) out.texturePath = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, idx, "lightType");
    if (lua_isstring(L, -1)) {
        out.lightType = lua_tostring(L, -1);
        out.isLight = true;
    }
    lua_pop(L, 1);

    lua_getfield(L, idx, "lightColor");
    if (lua_istable(L, -1)) {
        float r = 1, g = 1, b = 1;
        lua_rawgeti(L, -2, 1); r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        out.lightColor = glm::vec3(r, g, b);
    }
    lua_pop(L, 1);

    lua_getfield(L, idx, "lightRange");
    if (lua_isnumber(L, -1)) out.lightRange = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return true;
}

Entity* MapLoader::CreateEntityFromTemplate(const MapTemplate& tmpl, const std::string& entName) {
    auto& scene = Scene::Instance();
    Entity* ent = scene.CreateEntity(entName);

    if (tmpl.isLight) {
        auto* lc = ent->AddComponent<LightComponent>();
        if (tmpl.lightType == "directional") lc->type = LightComponent::Directional;
        else if (tmpl.lightType == "spot") lc->type = LightComponent::Spot;
        else lc->type = LightComponent::Point;
        lc->color = tmpl.lightColor;
        lc->range = tmpl.lightRange;
    }

    if (tmpl.isMesh) {
        auto* mc = ent->AddComponent<MeshComponent>();
        mc->color = tmpl.color;

        if (tmpl.meshType == "cube")      mc->mesh = GetOrCreateMesh("cube_1", []{ return Mesh::CreateCube(1.0f); });
        else if (tmpl.meshType == "box")  mc->mesh = GetOrCreateMesh("box_1x1x1", []{ return Mesh::CreateBox(1, 1, 1); });
        else if (tmpl.meshType == "sphere") mc->mesh = GetOrCreateMesh("sphere_05_16", []{ return Mesh::CreateSphere(0.5f, 16); });
        else if (tmpl.meshType == "plane")  mc->mesh = GetOrCreateMesh("plane_1_1", []{ return Mesh::CreatePlane(1, 1); });
        else if (tmpl.meshType == "cylinder") mc->mesh = GetOrCreateMesh("cyl_05_1_16", []{ return Mesh::CreateCylinder(0.5f, 1.0f, 16); });
        else if (tmpl.meshType == "cone")    mc->mesh = GetOrCreateMesh("cone_05_1_16", []{ return Mesh::CreateCone(0.5f, 1.0f, 16); });
        else if (tmpl.meshType == "torus")   mc->mesh = GetOrCreateMesh("torus_05_02_16_8", []{ return Mesh::CreateTorus(0.5f, 0.2f, 16, 8); });
        else if (tmpl.meshType == "quad")    mc->mesh = GetOrCreateMesh("quad_1_1", []{ return Mesh::CreateQuad(1, 1); });

        if (!tmpl.texturePath.empty()) {
            auto* tex = ResourceManager::Instance().LoadTexture(tmpl.texturePath);
            if (tex) mc->texture = tex;
        }
    }

    ent->SetScale(tmpl.scale);
    return ent;
}

Entity* MapLoader::CreateFromTemplate(const std::string& name) {
    auto it = m_templates.find(name);
    if (it == m_templates.end()) {
        LOG_INFO() << "[Map] Unknown template: " << name;
        return nullptr;
    }
    Entity* ent = CreateEntityFromTemplate(it->second, name);
    LOG_INFO() << "[Map] Created from template: " << name;
    return ent;
}

void MapLoader::SetTemplate(const std::string& name, const MapTemplate& tmpl) {
    m_templates[name] = tmpl;
}

bool MapLoader::HasTemplate(const std::string& name) const {
    return m_templates.find(name) != m_templates.end();
}

} // namespace planet
