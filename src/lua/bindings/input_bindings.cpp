#include "lua/lua_engine.h"
#include "planet/core/input.h"
#include <lauxlib.h>
#include <unordered_map>

namespace planet {

static KeyCode LuaToKeyCode(const char* name) {
    static std::unordered_map<std::string, KeyCode> keyMap = {
        {"space", KeyCode::Space}, {"a", KeyCode::A}, {"b", KeyCode::B}, {"c", KeyCode::C},
        {"d", KeyCode::D}, {"e", KeyCode::E}, {"f", KeyCode::F}, {"g", KeyCode::G},
        {"h", KeyCode::H}, {"i", KeyCode::I}, {"j", KeyCode::J}, {"k", KeyCode::K},
        {"l", KeyCode::L}, {"m", KeyCode::M}, {"n", KeyCode::N}, {"o", KeyCode::O},
        {"p", KeyCode::P}, {"q", KeyCode::Q}, {"r", KeyCode::R}, {"s", KeyCode::S},
        {"t", KeyCode::T}, {"u", KeyCode::U}, {"v", KeyCode::V}, {"w", KeyCode::W},
        {"x", KeyCode::X}, {"y", KeyCode::Y}, {"z", KeyCode::Z},
        {"0", KeyCode::Num0}, {"1", KeyCode::Num1}, {"2", KeyCode::Num2}, {"3", KeyCode::Num3},
        {"4", KeyCode::Num4}, {"5", KeyCode::Num5}, {"6", KeyCode::Num6}, {"7", KeyCode::Num7},
        {"8", KeyCode::Num8}, {"9", KeyCode::Num9},
        {"escape", KeyCode::Escape}, {"enter", KeyCode::Enter}, {"tab", KeyCode::Tab},
        {"backspace", KeyCode::Backspace}, {"delete", KeyCode::Delete}, {"insert", KeyCode::Insert},
        {"up", KeyCode::Up}, {"down", KeyCode::Down}, {"left", KeyCode::Left}, {"right", KeyCode::Right},
        {"pageup", KeyCode::PageUp}, {"pagedown", KeyCode::PageDown},
        {"home", KeyCode::Home}, {"end", KeyCode::End},
        {"left_shift", KeyCode::LeftShift}, {"right_shift", KeyCode::RightShift},
        {"left_ctrl", KeyCode::LeftControl}, {"right_ctrl", KeyCode::RightControl},
        {"left_alt", KeyCode::LeftAlt}, {"right_alt", KeyCode::RightAlt},
        {"left_super", KeyCode::LeftSuper}, {"right_super", KeyCode::RightSuper},
        {"f1", KeyCode::F1}, {"f2", KeyCode::F2}, {"f3", KeyCode::F3}, {"f4", KeyCode::F4},
        {"f5", KeyCode::F5}, {"f6", KeyCode::F6}, {"f7", KeyCode::F7}, {"f8", KeyCode::F8},
        {"f9", KeyCode::F9}, {"f10", KeyCode::F10}, {"f11", KeyCode::F11}, {"f12", KeyCode::F12},
    };
    auto it = keyMap.find(name);
    return (it != keyMap.end()) ? it->second : KeyCode::Unknown;
}

static int lua_GetKey(lua_State* L) {
    const char* keyName = luaL_checkstring(L, 1);
    lua_pushboolean(L, Input::Instance().GetKey(LuaToKeyCode(keyName)));
    return 1;
}

static int lua_GetKeyDown(lua_State* L) {
    const char* keyName = luaL_checkstring(L, 1);
    lua_pushboolean(L, Input::Instance().GetKeyDown(LuaToKeyCode(keyName)));
    return 1;
}

static int lua_GetKeyUp(lua_State* L) {
    const char* keyName = luaL_checkstring(L, 1);
    lua_pushboolean(L, Input::Instance().GetKeyUp(LuaToKeyCode(keyName)));
    return 1;
}

static int lua_GetMousePos(lua_State* L) {
    auto pos = Input::Instance().GetMousePosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

static int lua_GetMouseDelta(lua_State* L) {
    auto delta = Input::Instance().GetMouseDelta();
    lua_pushnumber(L, delta.x);
    lua_pushnumber(L, delta.y);
    return 2;
}

static int lua_GetScrollDelta(lua_State* L) {
    lua_pushnumber(L, Input::Instance().GetScrollDelta());
    return 1;
}

static int lua_GetMouseButton(lua_State* L) {
    int button = static_cast<int>(luaL_checkinteger(L, 1));
    MouseButton mb = static_cast<MouseButton>(button);
    lua_pushboolean(L, Input::Instance().GetMouseButton(mb));
    return 1;
}

static int lua_GetMouseButtonDown(lua_State* L) {
    int button = static_cast<int>(luaL_checkinteger(L, 1));
    MouseButton mb = static_cast<MouseButton>(button);
    lua_pushboolean(L, Input::Instance().GetMouseButtonDown(mb));
    return 1;
}

static int lua_GetMouseButtonUp(lua_State* L) {
    int button = static_cast<int>(luaL_checkinteger(L, 1));
    MouseButton mb = static_cast<MouseButton>(button);
    lua_pushboolean(L, Input::Instance().GetMouseButtonUp(mb));
    return 1;
}

static int lua_SetMouseLocked(lua_State* L) {
    Input::Instance().SetMouseLocked(lua_toboolean(L, 1));
    return 0;
}

static int lua_IsMouseLocked(lua_State* L) {
    lua_pushboolean(L, Input::Instance().IsMouseLocked());
    return 1;
}

static int lua_GetAnyKeyDown(lua_State* L) {
    lua_pushboolean(L, Input::Instance().GetAnyKeyDown());
    return 1;
}

static int lua_GetAnyKey(lua_State* L) {
    lua_pushboolean(L, Input::Instance().GetAnyKey());
    return 1;
}

void RegisterInputBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_GetKey);
    lua_setfield(L, -2, "getKey");
    lua_pushcfunction(L, lua_GetKeyDown);
    lua_setfield(L, -2, "getKeyDown");
    lua_pushcfunction(L, lua_GetKeyUp);
    lua_setfield(L, -2, "getKeyUp");
    lua_pushcfunction(L, lua_GetMousePos);
    lua_setfield(L, -2, "getMousePosition");
    lua_pushcfunction(L, lua_GetMouseDelta);
    lua_setfield(L, -2, "getMouseDelta");
    lua_pushcfunction(L, lua_GetScrollDelta);
    lua_setfield(L, -2, "getScrollDelta");
    lua_pushcfunction(L, lua_GetMouseButton);
    lua_setfield(L, -2, "getMouseButton");
    lua_pushcfunction(L, lua_GetMouseButtonDown);
    lua_setfield(L, -2, "getMouseButtonDown");
    lua_pushcfunction(L, lua_GetMouseButtonUp);
    lua_setfield(L, -2, "getMouseButtonUp");
    lua_pushcfunction(L, lua_SetMouseLocked);
    lua_setfield(L, -2, "setMouseLocked");
    lua_pushcfunction(L, lua_IsMouseLocked);
    lua_setfield(L, -2, "isMouseLocked");
    lua_pushcfunction(L, lua_GetAnyKeyDown);
    lua_setfield(L, -2, "getAnyKeyDown");
    lua_pushcfunction(L, lua_GetAnyKey);
    lua_setfield(L, -2, "getAnyKey");
    lua_setglobal(L, "Input");
}

} // namespace planet
