#include "lua/lua_engine.h"
#include "planet/core/dexecl_engine.h"
#include "planet/core/console.h"
#include <lauxlib.h>

namespace planet {

static int lua_dexeclInvoke(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    bool result = DexeclEngine::Instance().ExecuteFile(filename);
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

static int lua_consoleInvoke(lua_State* L) {
    const char* command = luaL_checkstring(L, 1);
    Console::Instance().Execute(command);
    return 0;
}

static int lua_dexeclSetVar(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    float value = static_cast<float>(luaL_checknumber(L, 2));
    DexeclEngine::Instance().SetVariable(name, value);
    return 0;
}

static int lua_dexeclGetVar(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    float defaultVal = static_cast<float>(luaL_optnumber(L, 2, 0.0));
    float val = DexeclEngine::Instance().GetVariable(name, defaultVal);
    lua_pushnumber(L, val);
    return 1;
}

void RegisterDexeclBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, lua_dexeclInvoke);
    lua_setfield(L, -2, "invoke");

    lua_pushcfunction(L, lua_consoleInvoke);
    lua_setfield(L, -2, "consoleInvoke");

    lua_pushcfunction(L, lua_dexeclSetVar);
    lua_setfield(L, -2, "setVar");

    lua_pushcfunction(L, lua_dexeclGetVar);
    lua_setfield(L, -2, "getVar");

    lua_setglobal(L, "Dexecl");

    lua_pushcfunction(L, lua_dexeclInvoke);
    lua_setglobal(L, "dexeclInvoke");

    lua_pushcfunction(L, lua_consoleInvoke);
    lua_setglobal(L, "consoleInvoke");
}

} // namespace planet
