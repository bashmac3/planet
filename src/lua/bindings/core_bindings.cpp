#include "lua/lua_engine.h"
#include "planet/core/engine.h"
#include "planet/core/window.h"
#include "planet/core/scene.h"
#include "planet/core/camera.h"
#include "planet/core/input.h"
#include "planet/core/console.h"
#include "planet/core/logger.h"
#include "planet/ecs/ecs.h"
#include "planet/core/timer.h"
#include "planet/core/random.h"
#include "planet/core/math.h"
#include <lauxlib.h>
#include <iostream>

namespace planet {

static int lua_Log(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    if (planet::g_termEnabled) std::cout << "[Lua] " << msg << std::endl;
    return 0;
}

static int lua_LogWarning(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Console::Instance().PrintWarning(msg);
    return 0;
}

static int lua_LogError(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Console::Instance().PrintError(msg);
    return 0;
}

static int lua_Quit(lua_State* L) {
    Engine::Instance().Quit();
    return 0;
}

static int lua_GetDeltaTime(lua_State* L) {
    lua_pushnumber(L, Engine::Instance().GetDeltaTime());
    return 1;
}

static int lua_GetElapsedTime(lua_State* L) {
    lua_pushnumber(L, Engine::Instance().GetElapsedTime());
    return 1;
}

static int lua_GetFrameCount(lua_State* L) {
    lua_pushinteger(L, static_cast<lua_Integer>(Engine::Instance().GetFrameCount()));
    return 1;
}

static int lua_GetAverageFPS(lua_State* L) {
    lua_pushnumber(L, Engine::Instance().GetAverageFPS());
    return 1;
}

static int lua_GetTimeSinceStart(lua_State* L) {
    lua_pushnumber(L, Engine::Instance().GetTimeSinceStart());
    return 1;
}

static int lua_SetTargetFPS(lua_State* L) {
    int fps = static_cast<int>(luaL_checkinteger(L, 1));
    Engine::Instance().SetTargetFPS(fps);
    return 0;
}

static int lua_GetTargetFPS(lua_State* L) {
    lua_pushinteger(L, Engine::Instance().GetTargetFPS());
    return 1;
}

static int lua_SetVSync(lua_State* L) {
    Engine::Instance().SetVSync(lua_toboolean(L, 1));
    return 0;
}

static int lua_IsVSync(lua_State* L) {
    lua_pushboolean(L, Engine::Instance().IsVSync());
    return 1;
}

static int lua_SetFullscreen(lua_State* L) {
    Engine::Instance().SetFullscreen(lua_toboolean(L, 1));
    return 0;
}

static int lua_IsFullscreen(lua_State* L) {
    lua_pushboolean(L, Engine::Instance().IsFullscreen());
    return 1;
}

static int lua_SetWindowTitle(lua_State* L) {
    const char* title = luaL_checkstring(L, 1);
    Window::Instance().SetTitle(title);
    return 0;
}

static int lua_SetWindowIcon(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Window::Instance().SetIcon(path);
    return 0;
}

static int lua_GetWindowSize(lua_State* L) {
    lua_pushinteger(L, Window::Instance().GetWidth());
    lua_pushinteger(L, Window::Instance().GetHeight());
    return 2;
}

static int lua_SetWindowSize(lua_State* L) {
    int w = static_cast<int>(luaL_checkinteger(L, 1));
    int h = static_cast<int>(luaL_checkinteger(L, 2));
    Window::Instance().SetSize(w, h);
    return 0;
}

static int lua_SetWindowFullscreen(lua_State* L) {
    Window::Instance().SetFullscreen(lua_toboolean(L, 1));
    return 0;
}

static int lua_MinimizeWindow(lua_State* L) {
    Window::Instance().Minimize();
    return 0;
}

static int lua_RestoreWindow(lua_State* L) {
    Window::Instance().Restore();
    return 0;
}

static int lua_GetNoclip(lua_State* L) {
    lua_pushboolean(L, Console::Instance().noclipEnabled);
    return 1;
}

static int lua_GetNoclipVerticalDelta(lua_State* L) {
    double dt = luaL_checknumber(L, 1);
    double speed = luaL_checknumber(L, 2);
    float delta = 0.0f;
    auto& input = Input::Instance();
    if (input.GetKey(KeyCode::Space)) delta += static_cast<float>(speed);
    if (input.GetKey(KeyCode::LeftShift)) delta -= static_cast<float>(speed);
    delta *= static_cast<float>(dt);
    lua_pushnumber(L, delta);
    return 1;
}

static int lua_CamSetPosition(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        cam->SetPosition(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 1)),
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3))));
    }
    return 0;
}

static int lua_CamGetPosition(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        const auto& pos = cam->GetPosition();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        lua_pushnumber(L, pos.z);
        return 3;
    }
    return 0;
}

static int lua_CamSetTarget(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        cam->SetTarget(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 1)),
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3))));
    }
    return 0;
}

static int lua_CamSetFOV(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) cam->SetFOV(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_CamGetFOV(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        lua_pushnumber(L, cam->GetFOV());
        return 1;
    }
    return 0;
}

static int lua_CamOrbit(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        float yaw = static_cast<float>(luaL_checknumber(L, 1));
        float pitch = static_cast<float>(luaL_checknumber(L, 2));
        cam->Orbit(yaw, pitch);
    }
    return 0;
}

static int lua_CamLookAt(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        cam->LookAt(glm::vec3(
            static_cast<float>(luaL_checknumber(L, 1)),
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3))));
    }
    return 0;
}

static int lua_CamSetZoom(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) cam->SetZoom(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_CamGetZoom(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        lua_pushnumber(L, cam->GetZoom());
        return 1;
    }
    return 0;
}

static int lua_CamSetPitch(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) cam->SetPitch(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_CamSetYaw(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) cam->SetYaw(static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int lua_CamSetMouseLock(lua_State* L) {
    Input::Instance().SetMouseLocked(lua_toboolean(L, 1));
    return 0;
}

static int lua_CamScreenToWorld(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        float sx = static_cast<float>(luaL_checknumber(L, 1));
        float sy = static_cast<float>(luaL_checknumber(L, 2));
        float sz = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        float vw = static_cast<float>(luaL_optnumber(L, 4, 1280));
        float vh = static_cast<float>(luaL_optnumber(L, 5, 720));
        auto world = cam->ScreenToWorldPoint(sx, sy, sz, vw, vh);
        lua_pushnumber(L, world.x);
        lua_pushnumber(L, world.y);
        lua_pushnumber(L, world.z);
        return 3;
    }
    return 0;
}

static int lua_CamWorldToScreen(lua_State* L) {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (cam) {
        float wx = static_cast<float>(luaL_checknumber(L, 1));
        float wy = static_cast<float>(luaL_checknumber(L, 2));
        float wz = static_cast<float>(luaL_checknumber(L, 3));
        float vw = static_cast<float>(luaL_optnumber(L, 4, 1280));
        float vh = static_cast<float>(luaL_optnumber(L, 5, 720));
        auto screen = cam->WorldToScreenPoint(glm::vec3(wx, wy, wz), vw, vh);
        lua_pushnumber(L, screen.x);
        lua_pushnumber(L, screen.y);
        return 2;
    }
    return 0;
}

static int lua_SceneCreateEntityFromPrefab(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Entity* entity = Scene::Instance().CreateEntityFromPrefab(path);
    if (entity) {
        lua_newtable(L);
        lua_pushstring(L, entity->GetName().c_str());
        lua_setfield(L, -2, "name");
        lua_pushlightuserdata(L, entity);
        lua_setfield(L, -2, "__ptr");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_SceneClear(lua_State* L) {
    Scene::Instance().Clear();
    return 0;
}

static int lua_SceneGetEntityCount(lua_State* L) {
    lua_pushinteger(L, static_cast<lua_Integer>(Scene::Instance().GetEntityCount()));
    return 1;
}

static int lua_SceneFindEntities(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    auto entities = Scene::Instance().FindEntitiesByName(name);
    lua_createtable(L, static_cast<int>(entities.size()), 0);
    for (size_t i = 0; i < entities.size(); i++) {
        lua_newtable(L);
        lua_pushstring(L, entities[i]->GetName().c_str());
        lua_setfield(L, -2, "name");
        lua_pushlightuserdata(L, entities[i]);
        lua_setfield(L, -2, "__ptr");
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
}

static int lua_SceneSetAmbientLight(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));
    Scene::Instance().SetAmbientLight(glm::vec3(r, g, b));
    return 0;
}

static int lua_AddCommand(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* prefix = luaL_checkstring(L, 2);
    const char* args = luaL_checkstring(L, 3);
    const char* desc = luaL_checkstring(L, 4);
    luaL_checktype(L, 5, LUA_TFUNCTION);

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    Console::Instance().AddCommand({
        name, prefix, args, desc,
        [L, ref](const std::string& cmdArgs) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            lua_pushstring(L, cmdArgs.c_str());
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                std::cerr << "[Lua] Console command error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        }
    });

    return 0;
}

static int lua_PrintToConsole(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    Console::Instance().Print(msg);
    return 0;
}

static int lua_TimerAdd(lua_State* L) {
    double interval = luaL_checknumber(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    bool looping = lua_toboolean(L, 3);

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    int id = TimerManager::Instance().AddTimer(interval, [L, ref]() {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "[Lua] Timer error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }, looping);

    lua_pushinteger(L, id);
    return 1;
}

static int lua_TimerRemove(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    TimerManager::Instance().RemoveTimer(id);
    return 0;
}

static int lua_TimerDelay(lua_State* L) {
    double delay = luaL_checknumber(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    int id = TimerManager::Instance().AddDelayedCall(delay, [L, ref]() {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "[Lua] Delay call error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    });

    lua_pushinteger(L, id);
    return 1;
}

static int lua_MathLerp(lua_State* L) {
    float a = static_cast<float>(luaL_checknumber(L, 1));
    float b = static_cast<float>(luaL_checknumber(L, 2));
    float t = static_cast<float>(luaL_checknumber(L, 3));
    lua_pushnumber(L, Math::Lerp(a, b, t));
    return 1;
}

static int lua_MathClamp(lua_State* L) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    float min = static_cast<float>(luaL_checknumber(L, 2));
    float max = static_cast<float>(luaL_checknumber(L, 3));
    lua_pushnumber(L, Math::Clamp(v, min, max));
    return 1;
}

static int lua_MathSmoothStep(lua_State* L) {
    float e0 = static_cast<float>(luaL_checknumber(L, 1));
    float e1 = static_cast<float>(luaL_checknumber(L, 2));
    float x = static_cast<float>(luaL_checknumber(L, 3));
    lua_pushnumber(L, Math::SmoothStep(e0, e1, x));
    return 1;
}

static int lua_MathMap(lua_State* L) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    float iMin = static_cast<float>(luaL_checknumber(L, 2));
    float iMax = static_cast<float>(luaL_checknumber(L, 3));
    float oMin = static_cast<float>(luaL_checknumber(L, 4));
    float oMax = static_cast<float>(luaL_checknumber(L, 5));
    lua_pushnumber(L, Math::Map(v, iMin, iMax, oMin, oMax));
    return 1;
}

static int lua_MathPingPong(lua_State* L) {
    float t = static_cast<float>(luaL_checknumber(L, 1));
    float len = static_cast<float>(luaL_checknumber(L, 2));
    lua_pushnumber(L, Math::PingPong(t, len));
    return 1;
}

static int lua_MathMoveTowards(lua_State* L) {
    float cur = static_cast<float>(luaL_checknumber(L, 1));
    float target = static_cast<float>(luaL_checknumber(L, 2));
    float maxDelta = static_cast<float>(luaL_checknumber(L, 3));
    lua_pushnumber(L, Math::MoveTowards(cur, target, maxDelta));
    return 1;
}

static int lua_MathDeltaAngle(lua_State* L) {
    float cur = static_cast<float>(luaL_checknumber(L, 1));
    float target = static_cast<float>(luaL_checknumber(L, 2));
    lua_pushnumber(L, Math::DeltaAngle(cur, target));
    return 1;
}

static int lua_RandomRange(lua_State* L) {
    float min = static_cast<float>(luaL_optnumber(L, 1, 0.0));
    float max = static_cast<float>(luaL_optnumber(L, 2, 1.0));
    lua_pushnumber(L, Random::Instance().Range(min, max));
    return 1;
}

static int lua_RandomInt(lua_State* L) {
    int min = static_cast<int>(luaL_checkinteger(L, 1));
    int max = static_cast<int>(luaL_checkinteger(L, 2));
    lua_pushinteger(L, Random::Instance().Range(min, max));
    return 1;
}

static int lua_RandomUnitVector(lua_State* L) {
    auto v = Random::Instance().UnitVector();
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    return 3;
}

static int lua_RandomInsideSphere(lua_State* L) {
    auto v = Random::Instance().InsideUnitSphere();
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    return 3;
}

static int lua_RandomSetSeed(lua_State* L) {
    uint64_t seed = static_cast<uint64_t>(luaL_checkinteger(L, 1));
    Random::Instance().SetSeed(seed);
    return 0;
}

void RegisterCoreBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, lua_Log);
    lua_setfield(L, -2, "log");
    lua_pushcfunction(L, lua_LogWarning);
    lua_setfield(L, -2, "logWarning");
    lua_pushcfunction(L, lua_LogError);
    lua_setfield(L, -2, "logError");
    lua_pushcfunction(L, lua_Quit);
    lua_setfield(L, -2, "quit");
    lua_pushcfunction(L, lua_GetDeltaTime);
    lua_setfield(L, -2, "getDeltaTime");
    lua_pushcfunction(L, lua_GetElapsedTime);
    lua_setfield(L, -2, "getElapsedTime");
    lua_pushcfunction(L, lua_GetFrameCount);
    lua_setfield(L, -2, "getFrameCount");
    lua_pushcfunction(L, lua_GetAverageFPS);
    lua_setfield(L, -2, "getAverageFPS");
    lua_pushcfunction(L, lua_GetTimeSinceStart);
    lua_setfield(L, -2, "getTimeSinceStart");
    lua_pushcfunction(L, lua_SetTargetFPS);
    lua_setfield(L, -2, "setTargetFPS");
    lua_pushcfunction(L, lua_GetTargetFPS);
    lua_setfield(L, -2, "getTargetFPS");
    lua_pushcfunction(L, lua_SetVSync);
    lua_setfield(L, -2, "setVSync");
    lua_pushcfunction(L, lua_IsVSync);
    lua_setfield(L, -2, "isVSync");
    lua_pushcfunction(L, lua_SetFullscreen);
    lua_setfield(L, -2, "setFullscreen");
    lua_pushcfunction(L, lua_IsFullscreen);
    lua_setfield(L, -2, "isFullscreen");
    lua_pushcfunction(L, lua_SetWindowTitle);
    lua_setfield(L, -2, "setWindowTitle");
    lua_pushcfunction(L, lua_SetWindowIcon);
    lua_setfield(L, -2, "setWindowIcon");
    lua_pushcfunction(L, lua_GetWindowSize);
    lua_setfield(L, -2, "getWindowSize");
    lua_pushcfunction(L, lua_SetWindowSize);
    lua_setfield(L, -2, "setWindowSize");
    lua_pushcfunction(L, lua_SetWindowFullscreen);
    lua_setfield(L, -2, "setWindowFullscreen");
    lua_pushcfunction(L, lua_MinimizeWindow);
    lua_setfield(L, -2, "minimizeWindow");
    lua_pushcfunction(L, lua_RestoreWindow);
    lua_setfield(L, -2, "restoreWindow");
    lua_pushcfunction(L, lua_GetNoclip);
    lua_setfield(L, -2, "getNoclip");
    lua_pushcfunction(L, lua_GetNoclipVerticalDelta);
    lua_setfield(L, -2, "getNoclipVerticalDelta");

    lua_setglobal(L, "Engine");

    lua_newtable(L);
    lua_pushcfunction(L, lua_CamSetPosition);
    lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, lua_CamGetPosition);
    lua_setfield(L, -2, "getPosition");
    lua_pushcfunction(L, lua_CamSetTarget);
    lua_setfield(L, -2, "setTarget");
    lua_pushcfunction(L, lua_CamSetFOV);
    lua_setfield(L, -2, "setFOV");
    lua_pushcfunction(L, lua_CamGetFOV);
    lua_setfield(L, -2, "getFOV");
    lua_pushcfunction(L, lua_CamOrbit);
    lua_setfield(L, -2, "orbit");
    lua_pushcfunction(L, lua_CamLookAt);
    lua_setfield(L, -2, "lookAt");
    lua_pushcfunction(L, lua_CamSetZoom);
    lua_setfield(L, -2, "setZoom");
    lua_pushcfunction(L, lua_CamGetZoom);
    lua_setfield(L, -2, "getZoom");
    lua_pushcfunction(L, lua_CamSetPitch);
    lua_setfield(L, -2, "setPitch");
    lua_pushcfunction(L, lua_CamSetYaw);
    lua_setfield(L, -2, "setYaw");
    lua_pushcfunction(L, lua_CamSetMouseLock);
    lua_setfield(L, -2, "setMouseLock");
    lua_pushcfunction(L, lua_CamScreenToWorld);
    lua_setfield(L, -2, "screenToWorld");
    lua_pushcfunction(L, lua_CamWorldToScreen);
    lua_setfield(L, -2, "worldToScreen");
    lua_setglobal(L, "Camera");

    lua_newtable(L);
    lua_pushcfunction(L, lua_SceneCreateEntityFromPrefab);
    lua_setfield(L, -2, "createEntityFromPrefab");
    lua_pushcfunction(L, lua_SceneClear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, lua_SceneGetEntityCount);
    lua_setfield(L, -2, "getEntityCount");
    lua_pushcfunction(L, lua_SceneFindEntities);
    lua_setfield(L, -2, "findEntities");
    lua_pushcfunction(L, lua_SceneSetAmbientLight);
    lua_setfield(L, -2, "setAmbientLight");
    lua_setglobal(L, "Scene");

    lua_newtable(L);
    lua_pushcfunction(L, lua_AddCommand);
    lua_setfield(L, -2, "addCommand");
    lua_pushcfunction(L, lua_PrintToConsole);
    lua_setfield(L, -2, "print");
    lua_setglobal(L, "Console");

    // Math helpers
    lua_newtable(L);
    lua_pushcfunction(L, lua_MathLerp);
    lua_setfield(L, -2, "lerp");
    lua_pushcfunction(L, lua_MathClamp);
    lua_setfield(L, -2, "clamp");
    lua_pushcfunction(L, lua_MathSmoothStep);
    lua_setfield(L, -2, "smoothStep");
    lua_pushcfunction(L, lua_MathMap);
    lua_setfield(L, -2, "map");
    lua_pushcfunction(L, lua_MathPingPong);
    lua_setfield(L, -2, "pingPong");
    lua_pushcfunction(L, lua_MathMoveTowards);
    lua_setfield(L, -2, "moveTowards");
    lua_pushcfunction(L, lua_MathDeltaAngle);
    lua_setfield(L, -2, "deltaAngle");
    lua_setglobal(L, "Math");

    // Random
    lua_newtable(L);
    lua_pushcfunction(L, lua_RandomRange);
    lua_setfield(L, -2, "range");
    lua_pushcfunction(L, lua_RandomInt);
    lua_setfield(L, -2, "int");
    lua_pushcfunction(L, lua_RandomUnitVector);
    lua_setfield(L, -2, "unitVector");
    lua_pushcfunction(L, lua_RandomInsideSphere);
    lua_setfield(L, -2, "insideUnitSphere");
    lua_pushcfunction(L, lua_RandomSetSeed);
    lua_setfield(L, -2, "setSeed");
    lua_setglobal(L, "Random");

    // Timer
    lua_newtable(L);
    lua_pushcfunction(L, lua_TimerAdd);
    lua_setfield(L, -2, "add");
    lua_pushcfunction(L, lua_TimerRemove);
    lua_setfield(L, -2, "remove");
    lua_pushcfunction(L, lua_TimerDelay);
    lua_setfield(L, -2, "delay");
    lua_setglobal(L, "Timer");
}

} // namespace planet
