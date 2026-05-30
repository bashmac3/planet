#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

struct lua_State;

namespace planet {

class Entity;

struct MapTemplate {
    std::string meshType;
    glm::vec3 scale{1.0f};
    glm::vec4 color{1.0f};
    std::string texturePath;
    std::string lightType;
    glm::vec3 lightColor{1.0f};
    float lightRange = 10.0f;
    bool isLight = false;
    bool isMesh = false;
};

class MapLoader {
public:
    static MapLoader& Instance();

    bool Load(const std::string& path);
    bool LoadLua(const std::string& path);
    Entity* CreateFromTemplate(const std::string& name);
    void SetTemplate(const std::string& name, const MapTemplate& tmpl);
    bool HasTemplate(const std::string& name) const;

private:
    MapLoader() = default;
    MapLoader(const MapLoader&) = delete;
    MapLoader& operator=(const MapLoader&) = delete;

    bool ProcessEnvironment(lua_State* L, int idx);
    bool ProcessTemplates(lua_State* L, int idx);
    bool ReadTemplateFromTable(lua_State* L, int idx, MapTemplate& out);
    Entity* CreateEntityFromTemplate(const MapTemplate& tmpl, const std::string& entName);

    std::unordered_map<std::string, MapTemplate> m_templates;
};

} // namespace planet
