#include "lua/lua_engine.h"
#include "planet/core/engine.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/resource/resource_manager.h"
#include <lualib.h>
#include <lauxlib.h>
#include "planet/core/logger.h"

namespace planet {

// Forward declare binding registration functions
extern void RegisterCoreBindings(lua_State* L);
extern void RegisterRenderBindings(lua_State* L);
extern void RegisterInputBindings(lua_State* L);
#ifdef PLANET_USE_FMOD
extern void RegisterFmodAudioBindings(lua_State* L);
#elif !defined(PLANET_NO_AUDIO)
extern void RegisterAudioBindings(lua_State* L);
#endif
extern void RegisterECSBindings(lua_State* L);
extern void RegisterPhysicsBindings(lua_State* L);
extern void RegisterDexeclBindings(lua_State* L);
extern void RegisterMapBindings(lua_State* L);

LuaRuntime& LuaRuntime::Instance() {
    static LuaRuntime instance;
    return instance;
}

bool LuaRuntime::Init(const std::string& scriptPath) {
    m_scriptPath = scriptPath;

    m_L = luaL_newstate();
    if (!m_L) {
        LOG_ERROR() << "[Lua] Failed to create Lua state!";
        return false;
    }

    luaL_openlibs(m_L);
    RegisterBindings();

    if (!LoadScript(m_scriptPath)) {
        LOG_ERROR() << "[Lua] Warning: Could not load main script: " << m_scriptPath;
    }

    LOG_INFO() << "[Lua] Lua engine initialized.";
    return true;
}

void LuaRuntime::Shutdown() {
    if (m_L) {
        lua_close(m_L);
        m_L = nullptr;
    }
}

void LuaRuntime::RegisterBindings() {
    RegisterCoreBindings(m_L);
    RegisterRenderBindings(m_L);
    RegisterInputBindings(m_L);
#ifdef PLANET_USE_FMOD
    RegisterFmodAudioBindings(m_L);
#elif !defined(PLANET_NO_AUDIO)
    RegisterAudioBindings(m_L);
#endif
    RegisterECSBindings(m_L);
    RegisterPhysicsBindings(m_L);
    RegisterDexeclBindings(m_L);
    RegisterMapBindings(m_L);
}

bool LuaRuntime::LoadScript(const std::string& path) {
    auto& res = ResourceManager::Instance();
    if (res.HasKerdata()) {
        std::vector<uint8_t> data;
        if (res.ReadKerdataFile(path, data)) {
            std::string source(data.begin(), data.end());
            if (luaL_loadbuffer(m_L, source.c_str(), source.size(), path.c_str()) ||
                lua_pcall(m_L, 0, 0, 0)) {
                LOG_ERROR() << "[Lua] Error loading '" << path + "' from kerdata: " << lua_tostring(m_L, -1);
                lua_pop(m_L, 1);
                return false;
            }
            return true;
        }
    }

    if (luaL_loadfile(m_L, path.c_str()) || lua_pcall(m_L, 0, 0, 0)) {
        LOG_ERROR() << "[Lua] Error loading script '" << path + "': " << lua_tostring(m_L, -1);
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

void LuaRuntime::DoString(const std::string& code) {
    if (luaL_dostring(m_L, code.c_str())) {
        LOG_ERROR() << "[Lua] Error: " << lua_tostring(m_L, -1);
        lua_pop(m_L, 1);
    }
}

void LuaRuntime::CallFunction(const std::string& funcName) {
    lua_getglobal(m_L, funcName.c_str());
    if (lua_isfunction(m_L, -1)) {
        if (lua_pcall(m_L, 0, 0, 0)) {
            LOG_ERROR() << "[Lua] Error calling " << funcName + ": " << lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
        }
    } else {
        lua_pop(m_L, 1);
    }
}

void LuaRuntime::CallFunction(const std::string& funcName, double value) {
    lua_getglobal(m_L, funcName.c_str());
    if (lua_isfunction(m_L, -1)) {
        lua_pushnumber(m_L, value);
        if (lua_pcall(m_L, 1, 0, 0)) {
            LOG_ERROR() << "[Lua] Error calling " << funcName + ": " << lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
        }
    } else {
        lua_pop(m_L, 1);
    }
}

void LuaRuntime::PushEntity(Entity* entity) {
    RegisterEntityTable(entity);
}

void LuaRuntime::RegisterEntityTable(Entity* entity) {
    lua_newtable(m_L);

    lua_pushstring(m_L, entity->GetName().c_str());
    lua_setfield(m_L, -2, "name");

    auto pos = entity->GetPosition();
    lua_newtable(m_L);
    lua_pushnumber(m_L, pos.x); lua_setfield(m_L, -2, "x");
    lua_pushnumber(m_L, pos.y); lua_setfield(m_L, -2, "y");
    lua_pushnumber(m_L, pos.z); lua_setfield(m_L, -2, "z");
    lua_setfield(m_L, -2, "position");

    lua_pushlightuserdata(m_L, entity);
    lua_setfield(m_L, -2, "__ptr");
}

void LuaRuntime::RunScriptComponent(ScriptComponent* comp, Entity* entity, double dt) {
    if (!comp || !entity) return;

    LoadScript(comp->scriptPath);

    lua_getglobal(m_L, "onUpdate");
    if (lua_isfunction(m_L, -1)) {
        RegisterEntityTable(entity);
        lua_pushnumber(m_L, dt);
        if (lua_pcall(m_L, 2, 0, 0)) {
            LOG_ERROR() << "[Lua] Script error in " << comp->scriptPath + ": " << lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
        }
    } else {
        lua_pop(m_L, 1);
    }
}

} // namespace planet
