#include "lua/lua_engine.h"
#include "planet/editor/editor.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include <lauxlib.h>

namespace planet {

static int lua_EditorIsActive(lua_State* L) {
    lua_pushboolean(L, Editor::Instance().IsActive());
    return 1;
}

static int lua_EditorSetActive(lua_State* L) {
    Editor::Instance().SetActive(lua_toboolean(L, 1));
    return 0;
}

static int lua_EditorToggle(lua_State* L) {
    Editor::Instance().SetActive(!Editor::Instance().IsActive());
    return 0;
}

static int lua_EditorGetSelected(lua_State* L) {
    Entity* entity = Editor::Instance().GetSelectedEntity();
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

static int lua_EditorSelectEntity(lua_State* L) {
    if (lua_isnil(L, 1)) {
        Editor::Instance().Deselect();
        return 0;
    }
    // Expect entity table
    lua_getfield(L, 1, "__ptr");
    Entity* entity = static_cast<Entity*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (entity) Editor::Instance().SelectEntity(entity);
    return 0;
}

static int lua_EditorDeselect(lua_State* L) {
    Editor::Instance().Deselect();
    return 0;
}

static int lua_EditorGetEntityCount(lua_State* L) {
    lua_pushinteger(L, Editor::Instance().GetEntityCount());
    return 1;
}

static int lua_EditorGetEntity(lua_State* L) {
    int index = static_cast<int>(luaL_checkinteger(L, 1));
    Entity* entity = Editor::Instance().GetEntityByIndex(index);
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, entity->GetName().c_str());
    lua_setfield(L, -2, "name");

    auto pos = entity->GetPosition();
    lua_newtable(L);
    lua_pushnumber(L, pos.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, pos.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, pos.z); lua_setfield(L, -2, "z");
    lua_setfield(L, -2, "position");

    auto euler = entity->GetEulerAngles();
    lua_newtable(L);
    lua_pushnumber(L, euler.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, euler.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, euler.z); lua_setfield(L, -2, "z");
    lua_setfield(L, -2, "rotation");

    auto s = entity->GetScale();
    lua_newtable(L);
    lua_pushnumber(L, s.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, s.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, s.z); lua_setfield(L, -2, "z");
    lua_setfield(L, -2, "scale");

    auto tags = entity->GetTags();
    lua_newtable(L);
    int idx = 1;
    for (const auto& tag : tags) {
        lua_pushstring(L, tag.c_str());
        lua_rawseti(L, -2, idx++);
    }
    lua_setfield(L, -2, "tags");

    lua_pushboolean(L, entity->IsActive());
    lua_setfield(L, -2, "active");

    lua_pushlightuserdata(L, entity);
    lua_setfield(L, -2, "__ptr");
    return 1;
}

static int lua_EditorSetGridSnap(lua_State* L) {
    Editor::Instance().SetGridSnap(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_EditorGetGridSnap(lua_State* L) {
    lua_pushnumber(L, Editor::Instance().GetGridSnap());
    return 1;
}

static int lua_EditorSnapToGrid(lua_State* L) {
    Editor::Instance().SnapToGrid();
    return 0;
}

static int lua_EditorSetShowGrid(lua_State* L) {
    Editor::Instance().SetShowGrid(lua_toboolean(L, 1));
    return 0;
}

static int lua_EditorGetShowGrid(lua_State* L) {
    lua_pushboolean(L, Editor::Instance().IsShowGrid());
    return 1;
}

static int lua_EditorGetCameraTarget(lua_State* L) {
    auto t = Editor::Instance().GetCameraTarget();
    lua_pushnumber(L, t.x);
    lua_pushnumber(L, t.y);
    lua_pushnumber(L, t.z);
    return 3;
}

static int lua_EditorSetCameraTarget(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float z = static_cast<float>(luaL_checknumber(L, 3));
    Editor::Instance().SetCameraTarget(glm::vec3(x, y, z));
    return 0;
}

void RegisterEditorBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, lua_EditorIsActive);
    lua_setfield(L, -2, "isActive");
    lua_pushcfunction(L, lua_EditorSetActive);
    lua_setfield(L, -2, "setActive");
    lua_pushcfunction(L, lua_EditorToggle);
    lua_setfield(L, -2, "toggle");
    lua_pushcfunction(L, lua_EditorGetSelected);
    lua_setfield(L, -2, "getSelected");
    lua_pushcfunction(L, lua_EditorSelectEntity);
    lua_setfield(L, -2, "selectEntity");
    lua_pushcfunction(L, lua_EditorDeselect);
    lua_setfield(L, -2, "deselect");
    lua_pushcfunction(L, lua_EditorGetEntityCount);
    lua_setfield(L, -2, "getEntityCount");
    lua_pushcfunction(L, lua_EditorGetEntity);
    lua_setfield(L, -2, "getEntity");
    lua_pushcfunction(L, lua_EditorSetGridSnap);
    lua_setfield(L, -2, "setGridSnap");
    lua_pushcfunction(L, lua_EditorGetGridSnap);
    lua_setfield(L, -2, "getGridSnap");
    lua_pushcfunction(L, lua_EditorSnapToGrid);
    lua_setfield(L, -2, "snapToGrid");
    lua_pushcfunction(L, lua_EditorSetShowGrid);
    lua_setfield(L, -2, "setShowGrid");
    lua_pushcfunction(L, lua_EditorGetShowGrid);
    lua_setfield(L, -2, "getShowGrid");
    lua_pushcfunction(L, lua_EditorGetCameraTarget);
    lua_setfield(L, -2, "getCameraTarget");
    lua_pushcfunction(L, lua_EditorSetCameraTarget);
    lua_setfield(L, -2, "setCameraTarget");

    lua_setglobal(L, "Editor");
}

} // namespace planet
