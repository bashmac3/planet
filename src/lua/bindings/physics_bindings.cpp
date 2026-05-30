#include "lua/lua_engine.h"
#include "planet/physics/physics.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include <algorithm>
#include <lauxlib.h>

namespace planet {

static int lua_SetGravity(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
    Physics::Instance().SetGravity(glm::vec3(x, y, z));
    return 0;
}

static int lua_GetGravity(lua_State* L) {
    auto g = Physics::Instance().GetGravity();
    lua_pushnumber(L, g.x);
    lua_pushnumber(L, g.y);
    lua_pushnumber(L, g.z);
    return 3;
}

static int lua_Raycast(lua_State* L) {
    float ox = static_cast<float>(luaL_checknumber(L, 1));
    float oy = static_cast<float>(luaL_checknumber(L, 2));
    float oz = static_cast<float>(luaL_checknumber(L, 3));
    float dx = static_cast<float>(luaL_checknumber(L, 4));
    float dy = static_cast<float>(luaL_checknumber(L, 5));
    float dz = static_cast<float>(luaL_checknumber(L, 6));
    float maxDist = static_cast<float>(luaL_optnumber(L, 7, 100.0));

    glm::vec3 origin(ox, oy, oz);
    glm::vec3 dir = glm::normalize(glm::vec3(dx, dy, dz));

    Entity* closest = nullptr;
    float closestDist = maxDist;
    glm::vec3 hitPoint;

    for (auto& entity : Scene::Instance().GetEntities()) {
        auto* meshComp = entity->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->visible) continue;

        auto pos = entity->GetPosition();
        auto scale = entity->GetScale();
        float radius = std::max({scale.x, scale.y, scale.z}) * 0.7f;

        glm::vec3 oc = origin - pos;
        float a = glm::dot(dir, dir);
        float b = 2.0f * glm::dot(oc, dir);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant >= 0) {
            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t > 0.01f && t < closestDist) {
                closestDist = t;
                closest = entity.get();
                hitPoint = origin + dir * t;
            }
        }
    }

    if (closest) {
        lua_pushboolean(L, true);
        lua_pushstring(L, closest->GetName().c_str());
        lua_pushnumber(L, hitPoint.x);
        lua_pushnumber(L, hitPoint.y);
        lua_pushnumber(L, hitPoint.z);
        lua_pushnumber(L, closestDist);
        lua_pushlightuserdata(L, closest);
        return 7;
    }

    lua_pushboolean(L, false);
    return 1;
}

static int lua_OverlapSphere(lua_State* L) {
    float cx = static_cast<float>(luaL_checknumber(L, 1));
    float cy = static_cast<float>(luaL_checknumber(L, 2));
    float cz = static_cast<float>(luaL_checknumber(L, 3));
    float radius = static_cast<float>(luaL_checknumber(L, 4));

    auto results = Physics::Instance().OverlapSphere(glm::vec3(cx, cy, cz), radius);
    lua_createtable(L, static_cast<int>(results.size()), 0);
    for (size_t i = 0; i < results.size(); i++) {
        lua_newtable(L);
        lua_pushstring(L, results[i].entity->GetName().c_str());
        lua_setfield(L, -2, "name");
        lua_pushlightuserdata(L, results[i].entity);
        lua_setfield(L, -2, "__ptr");
        lua_pushnumber(L, results[i].distance);
        lua_setfield(L, -2, "distance");
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
}

static int lua_SetSubSteps(lua_State* L) {
    int steps = static_cast<int>(luaL_checkinteger(L, 1));
    Physics::Instance().SetSubSteps(steps);
    return 0;
}

void RegisterPhysicsBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_SetGravity);
    lua_setfield(L, -2, "setGravity");
    lua_pushcfunction(L, lua_GetGravity);
    lua_setfield(L, -2, "getGravity");
    lua_pushcfunction(L, lua_Raycast);
    lua_setfield(L, -2, "raycast");
    lua_pushcfunction(L, lua_OverlapSphere);
    lua_setfield(L, -2, "overlapSphere");
    lua_pushcfunction(L, lua_SetSubSteps);
    lua_setfield(L, -2, "setSubSteps");
    lua_setglobal(L, "Physics");
}

} // namespace planet
