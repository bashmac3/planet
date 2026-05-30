#pragma once

#include <string>
#include <lua.hpp>

namespace planet {

class Entity;
class ScriptComponent;

class LuaRuntime {
public:
    static LuaRuntime& Instance();

    bool Init(const std::string& scriptPath);
    void Shutdown();

    void CallFunction(const std::string& funcName);
    void CallFunction(const std::string& funcName, double value);
    void RunScriptComponent(ScriptComponent* comp, Entity* entity, double dt);

    lua_State* GetState() { return m_L; }

    bool LoadScript(const std::string& path);
    void DoString(const std::string& code);

private:
    LuaRuntime() = default;
    ~LuaRuntime() = default;

    LuaRuntime(const LuaRuntime&) = delete;
    LuaRuntime& operator=(const LuaRuntime&) = delete;

    void RegisterBindings();
    void RegisterEntityTable(Entity* entity);

    void PushEntity(Entity* entity);

    lua_State* m_L = nullptr;
    std::string m_scriptPath;
};

} // namespace planet
