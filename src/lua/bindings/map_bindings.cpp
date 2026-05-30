#include "lua/lua_engine.h"
#include "planet/map/map_loader.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include <lauxlib.h>

namespace planet {

static int lua_MapLoad(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool ok = MapLoader::Instance().Load(path);
    lua_pushboolean(L, ok);
    return 1;
}

static int lua_MapCreateObject(lua_State* L) {
    const char* templateName = luaL_checkstring(L, 1);
    Entity* ent = MapLoader::Instance().CreateFromTemplate(templateName);
    if (!ent) {
        lua_pushnil(L);
        return 1;
    }

    // Optional position override
    if (lua_gettop(L) >= 4) {
        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float z = (float)luaL_checknumber(L, 4);
        ent->SetPosition(glm::vec3(x, y, z));
    }

    // Push entity table
    lua_newtable(L);
    lua_pushstring(L, ent->GetName().c_str());
    lua_setfield(L, -2, "name");
    lua_pushlightuserdata(L, ent);
    lua_setfield(L, -2, "__ptr");
    return 1;
}

static int lua_MapSetTemplate(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    if (!lua_istable(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    MapTemplate tmpl;

    lua_getfield(L, 2, "mesh");
    if (lua_isstring(L, -1)) { tmpl.meshType = lua_tostring(L, -1); tmpl.isMesh = true; }
    lua_pop(L, 1);

    lua_getfield(L, 2, "scale");
    if (lua_istable(L, -1)) {
        float sx = 1, sy = 1, sz = 1;
        lua_rawgeti(L, -2, 1); sx = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); sy = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); sz = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        tmpl.scale = glm::vec3(sx, sy, sz);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "color");
    if (lua_istable(L, -1)) {
        float r = 1, g = 1, b = 1, a = 1;
        lua_rawgeti(L, -2, 1); r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 4); a = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        tmpl.color = glm::vec4(r, g, b, a);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "texture");
    if (lua_isstring(L, -1)) tmpl.texturePath = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "lightType");
    if (lua_isstring(L, -1)) { tmpl.lightType = lua_tostring(L, -1); tmpl.isLight = true; }
    lua_pop(L, 1);

    lua_getfield(L, 2, "lightColor");
    if (lua_istable(L, -1)) {
        float r = 1, g = 1, b = 1;
        lua_rawgeti(L, -2, 1); r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 2); g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -2, 3); b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        tmpl.lightColor = glm::vec3(r, g, b);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "lightRange");
    if (lua_isnumber(L, -1)) tmpl.lightRange = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    MapLoader::Instance().SetTemplate(name, tmpl);
    lua_pushboolean(L, true);
    return 1;
}

void RegisterMapBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, lua_MapLoad);
    lua_setfield(L, -2, "load");

    lua_pushcfunction(L, lua_MapCreateObject);
    lua_setfield(L, -2, "createObject");

    lua_pushcfunction(L, lua_MapSetTemplate);
    lua_setfield(L, -2, "setTemplate");

    lua_setglobal(L, "Map");
}

} // namespace planet
