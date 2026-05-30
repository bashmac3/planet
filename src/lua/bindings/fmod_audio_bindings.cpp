#include "lua/lua_engine.h"
#include "planet/audio/fmod_audio.h"
#include <lauxlib.h>

namespace planet {

static int lua_PlaySound(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float volume = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    bool loop = lua_toboolean(L, 3);
    int id = FmodAudio::Instance().PlaySound(path, volume, loop);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_StopSound(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    FmodAudio::Instance().StopSound(id);
    return 0;
}

static int lua_PlayMusic(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float volume = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    FmodAudio::Instance().PlayMusic(path, volume);
    return 0;
}

static int lua_StopMusic(lua_State* L) {
    FmodAudio::Instance().StopMusic();
    return 0;
}

static int lua_SetMasterVolume(lua_State* L) {
    FmodAudio::Instance().SetMasterVolume(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

void RegisterFmodAudioBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_PlaySound);
    lua_setfield(L, -2, "playSound");
    lua_pushcfunction(L, lua_StopSound);
    lua_setfield(L, -2, "stopSound");
    lua_pushcfunction(L, lua_PlayMusic);
    lua_setfield(L, -2, "playMusic");
    lua_pushcfunction(L, lua_StopMusic);
    lua_setfield(L, -2, "stopMusic");
    lua_pushcfunction(L, lua_SetMasterVolume);
    lua_setfield(L, -2, "setMasterVolume");
    lua_setglobal(L, "Audio");
}

} // namespace planet
