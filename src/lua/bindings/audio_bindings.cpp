#include "lua/lua_engine.h"
#include "planet/audio/audio.h"
#include <lauxlib.h>

namespace planet {

static int lua_PlaySound(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float volume = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    bool loop = lua_toboolean(L, 3);
    int id = Audio::Instance().PlaySound(path, volume, loop);
    lua_pushinteger(L, id);
    return 1;
}

static int lua_StopSound(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    Audio::Instance().StopSound(id);
    return 0;
}

static int lua_IsSoundPlaying(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, Audio::Instance().IsSoundPlaying(id));
    return 1;
}

static int lua_SetSoundVolume(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    float vol = static_cast<float>(luaL_checknumber(L, 2));
    Audio::Instance().SetSoundVolume(id, vol);
    return 0;
}

static int lua_SetSoundPitch(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    float pitch = static_cast<float>(luaL_checknumber(L, 2));
    Audio::Instance().SetSoundPitch(id, pitch);
    return 0;
}

static int lua_PauseSound(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    Audio::Instance().PauseSound(id);
    return 0;
}

static int lua_ResumeSound(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    Audio::Instance().ResumeSound(id);
    return 0;
}

static int lua_StopAllSounds(lua_State* L) {
    Audio::Instance().StopAllSounds();
    return 0;
}

static int lua_PauseAllSounds(lua_State* L) {
    Audio::Instance().PauseAllSounds();
    return 0;
}

static int lua_ResumeAllSounds(lua_State* L) {
    Audio::Instance().ResumeAllSounds();
    return 0;
}

static int lua_PlayMusic(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float volume = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    Audio::Instance().PlayMusic(path, volume);
    return 0;
}

static int lua_StopMusic(lua_State* L) {
    Audio::Instance().StopMusic();
    return 0;
}

static int lua_IsMusicPlaying(lua_State* L) {
    lua_pushboolean(L, Audio::Instance().IsMusicPlaying());
    return 1;
}

static int lua_SetMasterVolume(lua_State* L) {
    Audio::Instance().SetMasterVolume(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_GetMasterVolume(lua_State* L) {
    lua_pushnumber(L, Audio::Instance().GetMasterVolume());
    return 1;
}

void RegisterAudioBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_PlaySound);
    lua_setfield(L, -2, "playSound");
    lua_pushcfunction(L, lua_StopSound);
    lua_setfield(L, -2, "stopSound");
    lua_pushcfunction(L, lua_IsSoundPlaying);
    lua_setfield(L, -2, "isSoundPlaying");
    lua_pushcfunction(L, lua_SetSoundVolume);
    lua_setfield(L, -2, "setSoundVolume");
    lua_pushcfunction(L, lua_SetSoundPitch);
    lua_setfield(L, -2, "setSoundPitch");
    lua_pushcfunction(L, lua_PauseSound);
    lua_setfield(L, -2, "pauseSound");
    lua_pushcfunction(L, lua_ResumeSound);
    lua_setfield(L, -2, "resumeSound");
    lua_pushcfunction(L, lua_StopAllSounds);
    lua_setfield(L, -2, "stopAllSounds");
    lua_pushcfunction(L, lua_PauseAllSounds);
    lua_setfield(L, -2, "pauseAllSounds");
    lua_pushcfunction(L, lua_ResumeAllSounds);
    lua_setfield(L, -2, "resumeAllSounds");
    lua_pushcfunction(L, lua_PlayMusic);
    lua_setfield(L, -2, "playMusic");
    lua_pushcfunction(L, lua_StopMusic);
    lua_setfield(L, -2, "stopMusic");
    lua_pushcfunction(L, lua_IsMusicPlaying);
    lua_setfield(L, -2, "isMusicPlaying");
    lua_pushcfunction(L, lua_SetMasterVolume);
    lua_setfield(L, -2, "setMasterVolume");
    lua_pushcfunction(L, lua_GetMasterVolume);
    lua_setfield(L, -2, "getMasterVolume");
    lua_setglobal(L, "Audio");
}

} // namespace planet
