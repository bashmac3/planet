#include "lua/lua_engine.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/render/renderer.h"
#include "planet/render/mesh.h"
#include "planet/render/texture.h"
#include "planet/resource/resource_manager.h"
#include <lauxlib.h>

#include <unordered_map>
#include <memory>

namespace planet {

static std::unordered_map<std::string, std::unique_ptr<Mesh>> g_sharedMeshes;

static Entity* GetEntityFromLua(lua_State* L, int idx) {
    lua_getfield(L, idx, "__ptr");
    Entity* entity = static_cast<Entity*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return entity;
}

static int lua_CreateEntity(lua_State* L) {
    const char* name = luaL_optstring(L, 1, "Entity");
    Entity* entity = Scene::Instance().CreateEntity(name);

    lua_newtable(L);
    lua_pushstring(L, entity->GetName().c_str());
    lua_setfield(L, -2, "name");
    lua_pushlightuserdata(L, entity);
    lua_setfield(L, -2, "__ptr");
    return 1;
}

static int lua_FindEntity(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    Entity* entity = Scene::Instance().FindEntity(name);
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, entity->GetName().c_str());
    lua_setfield(L, -2, "name");
    lua_pushlightuserdata(L, entity);
    lua_setfield(L, -2, "__ptr");
    return 1;
}

static int lua_DestroyEntity(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        Scene::Instance().DestroyEntity(entity);
    }
    return 0;
}

static int lua_EntitySetPosition(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->SetPosition(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntityGetPosition(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        auto pos = entity->GetPosition();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        lua_pushnumber(L, pos.z);
        return 3;
    }
    return 0;
}

static int lua_EntitySetScale(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->SetScale(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntitySetRotation(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->SetEulerAngles(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntityGetRotation(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        auto euler = entity->GetEulerAngles();
        lua_pushnumber(L, euler.x);
        lua_pushnumber(L, euler.y);
        lua_pushnumber(L, euler.z);
        return 3;
    }
    return 0;
}

static int lua_EntityAddTag(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) entity->AddTag(luaL_checkstring(L, 2));
    return 0;
}

static int lua_EntityHasTag(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        lua_pushboolean(L, entity->HasTag(luaL_checkstring(L, 2)));
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

static int lua_EntityRemoveTag(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) entity->RemoveTag(luaL_checkstring(L, 2));
    return 0;
}

static int lua_EntitySetActive(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) entity->SetActive(lua_toboolean(L, 2));
    return 0;
}

static int lua_EntityIsActive(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        lua_pushboolean(L, entity->IsActive());
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

static int lua_EntityTranslate(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->Translate(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntityRotate(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->Rotate(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntityLookAt(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        entity->LookAt(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4))));
    }
    return 0;
}

static int lua_EntityGetForward(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        auto fwd = entity->Forward();
        lua_pushnumber(L, fwd.x);
        lua_pushnumber(L, fwd.y);
        lua_pushnumber(L, fwd.z);
        return 3;
    }
    return 0;
}

static int lua_EntityGetRight(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        auto r = entity->Right();
        lua_pushnumber(L, r.x);
        lua_pushnumber(L, r.y);
        lua_pushnumber(L, r.z);
        return 3;
    }
    return 0;
}

static int lua_EntityGetUp(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity) {
        auto u = entity->Up();
        lua_pushnumber(L, u.x);
        lua_pushnumber(L, u.y);
        lua_pushnumber(L, u.z);
        return 3;
    }
    return 0;
}

static int lua_EntitySetParent(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    Entity* parent = GetEntityFromLua(L, 2);
    if (entity && parent) entity->SetParent(parent);
    return 0;
}

static int lua_EntityGetParent(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (entity && entity->GetParent()) {
        Entity* p = entity->GetParent();
        lua_newtable(L);
        lua_pushstring(L, p->GetName().c_str());
        lua_setfield(L, -2, "name");
        lua_pushlightuserdata(L, p);
        lua_setfield(L, -2, "__ptr");
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

static int lua_EntityDistanceTo(lua_State* L) {
    Entity* a = GetEntityFromLua(L, 1);
    Entity* b = GetEntityFromLua(L, 2);
    if (a && b) {
        lua_pushnumber(L, a->DistanceTo(b));
        return 1;
    }
    lua_pushnumber(L, -1);
    return 1;
}

static Mesh* GetOrCreateMesh(const std::string& key, std::function<Mesh()> factory) {
    auto it = g_sharedMeshes.find(key);
    if (it != g_sharedMeshes.end()) return it->second.get();
    auto mesh = std::make_unique<Mesh>(factory());
    Mesh* ptr = mesh.get();
    g_sharedMeshes[key] = std::move(mesh);
    return ptr;
}

static int lua_AddMeshBox(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float size = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    float r = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    bool wireframe = lua_toboolean(L, 7);

    std::string key = "cube_" + std::to_string(size);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [size]() { return Mesh::CreateCube(size); });
    comp->color = glm::vec4(r, g, b, a);
    comp->wireframe = wireframe;
    comp->visible = true;
    return 0;
}

static int lua_AddMeshPlane(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float width = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    float depth = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    float r = static_cast<float>(luaL_optnumber(L, 4, 0.5));
    float g = static_cast<float>(luaL_optnumber(L, 5, 0.5));
    float b = static_cast<float>(luaL_optnumber(L, 6, 0.5));
    float a = static_cast<float>(luaL_optnumber(L, 7, 1.0));

    std::string key = "plane_" + std::to_string(width) + "_" + std::to_string(depth);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [width, depth]() { return Mesh::CreatePlane(width, depth); });
    comp->color = glm::vec4(r, g, b, a);
    comp->visible = true;
    return 0;
}

static int lua_AddMeshBoxWH(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));
    float d = static_cast<float>(luaL_checknumber(L, 4));

    std::string key = "box_" + std::to_string(w) + "_" + std::to_string(h) + "_" + std::to_string(d);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [w, h, d]() { return Mesh::CreateBox(w, h, d); });
    comp->color = glm::vec4(1, 1, 1, 1);
    comp->visible = true;
    return 0;
}

static int lua_AddMeshWallQuad(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));

    std::string key = "wallq_" + std::to_string(w) + "_" + std::to_string(h);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [w, h]() { return Mesh::CreateWallQuad(w, h); });
    comp->color = glm::vec4(1, 1, 1, 1);
    comp->visible = true;
    return 0;
}

static int lua_AddMeshSphere(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float radius = static_cast<float>(luaL_optnumber(L, 2, 0.5));
    int segments = static_cast<int>(luaL_optinteger(L, 3, 16));
    float r = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    bool wireframe = lua_toboolean(L, 8);

    std::string key = "sphere_" + std::to_string(radius) + "_" + std::to_string(segments);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [radius, segments]() { return Mesh::CreateSphere(radius, segments); });
    comp->color = glm::vec4(r, g, b, a);
    comp->wireframe = wireframe;
    comp->visible = true;
    return 0;
}

static int lua_AddMeshCylinder(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float radius = static_cast<float>(luaL_optnumber(L, 2, 0.5));
    float height = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    int segs = static_cast<int>(luaL_optinteger(L, 4, 16));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));

    std::string key = "cyl_" + std::to_string(radius) + "_" + std::to_string(height) + "_" + std::to_string(segs);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [radius, height, segs]() { return Mesh::CreateCylinder(radius, height, segs); });
    comp->color = glm::vec4(r, g, b, a);
    comp->visible = true;
    return 0;
}

static int lua_AddMeshCone(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float radius = static_cast<float>(luaL_optnumber(L, 2, 0.5));
    float height = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    int segs = static_cast<int>(luaL_optinteger(L, 4, 16));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));

    std::string key = "cone_" + std::to_string(radius) + "_" + std::to_string(height) + "_" + std::to_string(segs);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [radius, height, segs]() { return Mesh::CreateCone(radius, height, segs); });
    comp->color = glm::vec4(r, g, b, a);
    comp->visible = true;
    return 0;
}

static int lua_AddMeshTorus(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;

    float radius = static_cast<float>(luaL_optnumber(L, 2, 0.5));
    float tube = static_cast<float>(luaL_optnumber(L, 3, 0.2));
    int segs = static_cast<int>(luaL_optinteger(L, 4, 16));
    int tubeSegs = static_cast<int>(luaL_optinteger(L, 5, 8));
    float r = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 8, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 9, 1.0));

    std::string key = "torus_" + std::to_string(radius) + "_" + std::to_string(tube) + "_" + std::to_string(segs) + "_" + std::to_string(tubeSegs);
    auto* comp = entity->AddComponent<MeshComponent>();
    comp->mesh = GetOrCreateMesh(key, [radius, tube, segs, tubeSegs]() { return Mesh::CreateTorus(radius, tube, segs, tubeSegs); });
    comp->color = glm::vec4(r, g, b, a);
    comp->visible = true;
    return 0;
}

static int lua_EntitySetColor(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<MeshComponent>();
    if (comp) {
        comp->color = glm::vec4(
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
            static_cast<float>(luaL_checknumber(L, 4)),
            static_cast<float>(luaL_optnumber(L, 5, 1.0)));
    }
    return 0;
}

static int lua_EntitySetUnlit(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<MeshComponent>();
    if (comp) {
        comp->unlit = lua_toboolean(L, 2);
    }
    return 0;
}

static int lua_EntityGetName(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    lua_pushstring(L, entity->GetName().c_str());
    return 1;
}

static int lua_EntityGetScale(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto s = entity->GetScale();
    lua_pushnumber(L, s.x);
    lua_pushnumber(L, s.y);
    lua_pushnumber(L, s.z);
    return 3;
}

static int lua_EntitySetTexture(lua_State* L) {
    if (!lua_istable(L, 1)) { lua_pushstring(L, "arg1 not entity table"); lua_error(L); return 0; }
    lua_getfield(L, 1, "__ptr");
    Entity* entity = static_cast<Entity*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (!entity) { lua_pushstring(L, "null entity"); lua_error(L); return 0; }
    const char* path = luaL_checkstring(L, 2);
    auto* comp = entity->GetComponent<MeshComponent>();
    if (!comp) { lua_pushstring(L, "no MeshComponent"); lua_error(L); return 0; }

    auto* tex = ResourceManager::Instance().LoadTexture(path);
    if (tex) {
        comp->texture = tex;
    }
    return 0;
}

static int lua_EntityAddScript(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    const char* scriptPath = luaL_checkstring(L, 2);
    auto* comp = entity->AddComponent<ScriptComponent>();
    comp->scriptPath = scriptPath;
    return 0;
}

static int lua_EntityAddRigidbody(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    float mass = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    auto* comp = entity->AddComponent<RigidbodyComponent>();
    comp->mass = mass;
    return 0;
}

static int lua_EntityAddLight(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    const char* type = luaL_optstring(L, 2, "point");
    float r = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float range = static_cast<float>(luaL_optnumber(L, 6, 10.0));
    auto* comp = entity->AddComponent<LightComponent>();
    if (std::string(type) == "directional") comp->type = LightComponent::Directional;
    else if (std::string(type) == "spot") comp->type = LightComponent::Spot;
    else comp->type = LightComponent::Point;
    comp->color = glm::vec3(r, g, b);
    comp->range = range;
    return 0;
}

static int lua_EntityAddParticleEmitter(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->AddComponent<ParticleEmitterComponent>();
    comp->maxParticles = static_cast<int>(luaL_optinteger(L, 2, 100));
    comp->emissionRate = static_cast<float>(luaL_optnumber(L, 3, 10.0f));
    comp->lifetime = static_cast<float>(luaL_optnumber(L, 4, 1.0f));
    comp->speed = static_cast<float>(luaL_optnumber(L, 5, 5.0f));
    comp->looping = lua_toboolean(L, 6);
    comp->playing = true;
    return 0;
}

static int lua_ParticleSetColors(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (!comp) return 0;
    comp->startColor = glm::vec4(
        static_cast<float>(luaL_checknumber(L, 2)),
        static_cast<float>(luaL_checknumber(L, 3)),
        static_cast<float>(luaL_checknumber(L, 4)),
        static_cast<float>(luaL_optnumber(L, 5, 1.0)));
    comp->endColor = glm::vec4(
        static_cast<float>(luaL_optnumber(L, 6, comp->startColor.r)),
        static_cast<float>(luaL_optnumber(L, 7, comp->startColor.g)),
        static_cast<float>(luaL_optnumber(L, 8, comp->startColor.b)),
        static_cast<float>(luaL_optnumber(L, 9, 0.0)));
    return 0;
}

static int lua_ParticleSetSizes(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (!comp) return 0;
    comp->startSize = static_cast<float>(luaL_optnumber(L, 2, 0.1f));
    comp->endSize = static_cast<float>(luaL_optnumber(L, 3, comp->startSize * 0.5f));
    return 0;
}

static int lua_ParticleSetGravity(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (!comp) return 0;
    comp->gravityModifier = glm::vec3(
        static_cast<float>(luaL_optnumber(L, 2, 0.0f)),
        static_cast<float>(luaL_optnumber(L, 3, -0.5f)),
        static_cast<float>(luaL_optnumber(L, 4, 0.0f)));
    return 0;
}

static int lua_ParticlePlay(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (comp) comp->Play();
    return 0;
}

static int lua_ParticleStop(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (comp) comp->Stop();
    return 0;
}

static int lua_ParticleEmit(lua_State* L) {
    Entity* entity = GetEntityFromLua(L, 1);
    if (!entity) return 0;
    auto* comp = entity->GetComponent<ParticleEmitterComponent>();
    if (comp) {
        int count = static_cast<int>(luaL_optinteger(L, 2, 1));
        comp->Emit(count);
    }
    return 0;
}

void RegisterECSBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_CreateEntity);
    lua_setfield(L, -2, "createEntity");
    lua_pushcfunction(L, lua_FindEntity);
    lua_setfield(L, -2, "findEntity");
    lua_pushcfunction(L, lua_DestroyEntity);
    lua_setfield(L, -2, "destroyEntity");
    lua_pushcfunction(L, lua_EntitySetPosition);
    lua_setfield(L, -2, "entitySetPosition");
    lua_pushcfunction(L, lua_EntityGetPosition);
    lua_setfield(L, -2, "entityGetPosition");
    lua_pushcfunction(L, lua_EntitySetScale);
    lua_setfield(L, -2, "entitySetScale");
    lua_pushcfunction(L, lua_EntitySetRotation);
    lua_setfield(L, -2, "entitySetRotation");
    lua_pushcfunction(L, lua_EntityGetRotation);
    lua_setfield(L, -2, "entityGetRotation");
    lua_pushcfunction(L, lua_EntityAddTag);
    lua_setfield(L, -2, "entityAddTag");
    lua_pushcfunction(L, lua_EntityHasTag);
    lua_setfield(L, -2, "entityHasTag");
    lua_pushcfunction(L, lua_EntityRemoveTag);
    lua_setfield(L, -2, "entityRemoveTag");
    lua_pushcfunction(L, lua_EntitySetActive);
    lua_setfield(L, -2, "entitySetActive");
    lua_pushcfunction(L, lua_EntityIsActive);
    lua_setfield(L, -2, "entityIsActive");
    lua_pushcfunction(L, lua_EntityTranslate);
    lua_setfield(L, -2, "entityTranslate");
    lua_pushcfunction(L, lua_EntityRotate);
    lua_setfield(L, -2, "entityRotate");
    lua_pushcfunction(L, lua_EntityLookAt);
    lua_setfield(L, -2, "entityLookAt");
    lua_pushcfunction(L, lua_EntityGetForward);
    lua_setfield(L, -2, "entityGetForward");
    lua_pushcfunction(L, lua_EntityGetRight);
    lua_setfield(L, -2, "entityGetRight");
    lua_pushcfunction(L, lua_EntityGetUp);
    lua_setfield(L, -2, "entityGetUp");
    lua_pushcfunction(L, lua_EntitySetParent);
    lua_setfield(L, -2, "entitySetParent");
    lua_pushcfunction(L, lua_EntityGetParent);
    lua_setfield(L, -2, "entityGetParent");
    lua_pushcfunction(L, lua_EntityDistanceTo);
    lua_setfield(L, -2, "entityDistanceTo");
    lua_pushcfunction(L, lua_EntitySetColor);
    lua_setfield(L, -2, "entitySetColor");
    lua_pushcfunction(L, lua_EntitySetUnlit);
    lua_setfield(L, -2, "entitySetUnlit");
    lua_pushcfunction(L, lua_EntityGetName);
    lua_setfield(L, -2, "entityGetName");
    lua_pushcfunction(L, lua_EntityGetScale);
    lua_setfield(L, -2, "entityGetScale");
    lua_pushcfunction(L, lua_EntitySetTexture);
    lua_setfield(L, -2, "entitySetTexture");
    lua_pushcfunction(L, lua_EntityAddScript);
    lua_setfield(L, -2, "entityAddScript");
    lua_pushcfunction(L, lua_EntityAddRigidbody);
    lua_setfield(L, -2, "entityAddRigidbody");
    lua_pushcfunction(L, lua_EntityAddLight);
    lua_setfield(L, -2, "entityAddLight");

    lua_pushcfunction(L, lua_EntityAddParticleEmitter);
    lua_setfield(L, -2, "entityAddParticleEmitter");
    lua_pushcfunction(L, lua_ParticleSetColors);
    lua_setfield(L, -2, "particleSetColors");
    lua_pushcfunction(L, lua_ParticleSetSizes);
    lua_setfield(L, -2, "particleSetSizes");
    lua_pushcfunction(L, lua_ParticleSetGravity);
    lua_setfield(L, -2, "particleSetGravity");
    lua_pushcfunction(L, lua_ParticlePlay);
    lua_setfield(L, -2, "particlePlay");
    lua_pushcfunction(L, lua_ParticleStop);
    lua_setfield(L, -2, "particleStop");
    lua_pushcfunction(L, lua_ParticleEmit);
    lua_setfield(L, -2, "particleEmit");

    lua_pushcfunction(L, lua_AddMeshBox);
    lua_setfield(L, -2, "addMeshBox");
    lua_pushcfunction(L, lua_AddMeshPlane);
    lua_setfield(L, -2, "addMeshPlane");
    lua_pushcfunction(L, lua_AddMeshBoxWH);
    lua_setfield(L, -2, "addMeshBoxEx");
    lua_pushcfunction(L, lua_AddMeshWallQuad);
    lua_setfield(L, -2, "addMeshWallQuad");
    lua_pushcfunction(L, lua_AddMeshSphere);
    lua_setfield(L, -2, "addMeshSphere");
    lua_pushcfunction(L, lua_AddMeshCylinder);
    lua_setfield(L, -2, "addMeshCylinder");
    lua_pushcfunction(L, lua_AddMeshCone);
    lua_setfield(L, -2, "addMeshCone");
    lua_pushcfunction(L, lua_AddMeshTorus);
    lua_setfield(L, -2, "addMeshTorus");
    lua_setglobal(L, "ECS");
}

} // namespace planet
