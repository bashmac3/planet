#include "lua/lua_engine.h"
#include "planet/render/renderer.h"
#include "planet/render/shader.h"
#include "planet/render/mesh.h"
#include "planet/render/texture.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/text_renderer.h"
#include "planet/render/post_processor.h"
#include "planet/resource/resource_manager.h"
#include <lauxlib.h>

namespace planet {

static int lua_ClearColor(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));
    float a = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    Renderer::Instance().ClearColor(r, g, b, a);
    return 0;
}

static int lua_GetClearColor(lua_State* L) {
    float r, g, b, a;
    Renderer::Instance().GetClearColor(r, g, b, a);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, a);
    return 4;
}

static int lua_SetWireframe(lua_State* L) {
    bool wireframe = lua_toboolean(L, 1);
    Renderer::Instance().SetRenderMode(wireframe);
    return 0;
}

static int lua_IsWireframe(lua_State* L) {
    lua_pushboolean(L, Renderer::Instance().IsWireframe());
    return 1;
}

static int lua_DrawSprite(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));

    SpriteRenderer::Instance().DrawSprite(
        glm::vec2(x, y), glm::vec2(w, h), glm::vec4(r, g, b, a));
    return 0;
}

static int lua_SetLightDir(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float z = static_cast<float>(luaL_checknumber(L, 3));
    Renderer::Instance().SetLightDirection(glm::vec3(x, y, z));
    return 0;
}

static int lua_SetLightColor(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));
    Renderer::Instance().SetLightColor(glm::vec3(r, g, b));
    return 0;
}

static int lua_SetAmbientColor(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));
    Renderer::Instance().SetAmbientColor(glm::vec3(r, g, b));
    return 0;
}

static int lua_GetLightDir(lua_State* L) {
    auto d = Renderer::Instance().GetLightDirection();
    lua_pushnumber(L, d.x);
    lua_pushnumber(L, d.y);
    lua_pushnumber(L, d.z);
    return 3;
}

static int lua_GetAmbientColor(lua_State* L) {
    auto c = Renderer::Instance().GetAmbientColor();
    lua_pushnumber(L, c.x);
    lua_pushnumber(L, c.y);
    lua_pushnumber(L, c.z);
    return 3;
}

static int lua_SetFogColor(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));
    Renderer::Instance().SetFogColor(glm::vec3(r, g, b));
    return 0;
}

static int lua_SetFogDensity(lua_State* L) {
    float density = static_cast<float>(luaL_checknumber(L, 1));
    Renderer::Instance().SetFogDensity(density);
    return 0;
}

static int lua_SetFogRange(lua_State* L) {
    float start = static_cast<float>(luaL_checknumber(L, 1));
    float end = static_cast<float>(luaL_checknumber(L, 2));
    Renderer::Instance().SetFogStart(start);
    Renderer::Instance().SetFogEnd(end);
    return 0;
}

static int lua_SetFogEnabled(lua_State* L) {
    Renderer::Instance().SetFogEnabled(lua_toboolean(L, 1));
    return 0;
}

static int lua_IsFogEnabled(lua_State* L) {
    lua_pushboolean(L, Renderer::Instance().IsFogEnabled());
    return 1;
}

static int lua_SetPostEffect(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    PostProcessor::Instance().SetEffectByName(name);
    return 0;
}

static int lua_GetPostEffect(lua_State* L) {
    auto effect = PostProcessor::Instance().GetEffect();
    lua_pushstring(L, PostProcessor::GetEffectName(effect).c_str());
    return 1;
}

static int lua_SetPostIntensity(lua_State* L) {
    float intensity = static_cast<float>(luaL_checknumber(L, 1));
    PostProcessor::Instance().SetEffectIntensity(intensity);
    return 0;
}

static int lua_GetPostIntensity(lua_State* L) {
    lua_pushnumber(L, PostProcessor::Instance().GetEffectIntensity());
    return 1;
}

static int lua_SetFlashlight(lua_State* L) {
    bool enabled = lua_toboolean(L, 1);
    if (enabled) {
        float px = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        float py = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        float pz = static_cast<float>(luaL_optnumber(L, 4, 0.0));
        float dx = static_cast<float>(luaL_optnumber(L, 5, 0.0));
        float dy = static_cast<float>(luaL_optnumber(L, 6, 0.0));
        float dz = static_cast<float>(luaL_optnumber(L, 7, -1.0));
        float r = static_cast<float>(luaL_optnumber(L, 8, 1.0));
        float g = static_cast<float>(luaL_optnumber(L, 9, 0.95));
        float b = static_cast<float>(luaL_optnumber(L, 10, 0.8));
        Renderer::Instance().SetFlashlightEnabled(true);
        Renderer::Instance().SetFlashlightPos(glm::vec3(px, py, pz));
        Renderer::Instance().SetFlashlightDir(glm::vec3(dx, dy, dz));
        Renderer::Instance().SetFlashlightColor(glm::vec3(r, g, b));
    } else {
        Renderer::Instance().SetFlashlightEnabled(false);
    }
    return 0;
}

static int lua_DrawRect(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));

    SpriteRenderer::Instance().DrawSprite(
        glm::vec2(x, y), glm::vec2(w, h), glm::vec4(r, g, b, a));
    return 0;
}

static int lua_DrawText(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float scale = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));
    TextRenderer::Instance().DrawString(text, x, y, scale, glm::vec4(r, g, b, a));
    return 0;
}

static int lua_DrawTextCentered(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    float cx = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float scale = static_cast<float>(luaL_optnumber(L, 4, 1.0));
    float r = static_cast<float>(luaL_optnumber(L, 5, 1.0));
    float g = static_cast<float>(luaL_optnumber(L, 6, 1.0));
    float b = static_cast<float>(luaL_optnumber(L, 7, 1.0));
    float a = static_cast<float>(luaL_optnumber(L, 8, 1.0));
    TextRenderer::Instance().DrawStringCentered(text, cx, y, scale, glm::vec4(r, g, b, a));
    return 0;
}

static int lua_AddPointLight(lua_State* L) {
    float px = static_cast<float>(luaL_checknumber(L, 1));
    float py = static_cast<float>(luaL_checknumber(L, 2));
    float pz = static_cast<float>(luaL_checknumber(L, 3));
    float r = static_cast<float>(luaL_checknumber(L, 4));
    float g = static_cast<float>(luaL_checknumber(L, 5));
    float b = static_cast<float>(luaL_checknumber(L, 6));
    float range = static_cast<float>(luaL_optnumber(L, 7, 5.0));
    int idx = Renderer::Instance().AddPointLight(glm::vec3(px, py, pz), glm::vec3(r, g, b), range);
    lua_pushinteger(L, idx);
    return 1;
}

static int lua_RemovePointLight(lua_State* L) {
    int idx = static_cast<int>(luaL_checkinteger(L, 1));
    Renderer::Instance().RemovePointLight(idx);
    return 0;
}

static int lua_ClearPointLights(lua_State* L) {
    Renderer::Instance().ClearPointLights();
    return 0;
}

static int lua_SetPointLightPosition(lua_State* L) {
    int idx = static_cast<int>(luaL_checkinteger(L, 1));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float z = static_cast<float>(luaL_checknumber(L, 4));
    Renderer::Instance().SetPointLightPosition(idx, glm::vec3(x, y, z));
    return 0;
}

static int lua_SetPointLightColor(lua_State* L) {
    int idx = static_cast<int>(luaL_checkinteger(L, 1));
    float r = static_cast<float>(luaL_checknumber(L, 2));
    float g = static_cast<float>(luaL_checknumber(L, 3));
    float b = static_cast<float>(luaL_checknumber(L, 4));
    Renderer::Instance().SetPointLightColor(idx, glm::vec3(r, g, b));
    return 0;
}

static int lua_GetPointLightCount(lua_State* L) {
    lua_pushinteger(L, Renderer::Instance().GetPointLightCount());
    return 1;
}

static int lua_LoadTexture(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    auto* tex = ResourceManager::Instance().LoadTexture(path);
    if (tex) {
        lua_newtable(L);
        lua_pushlightuserdata(L, tex);
        lua_setfield(L, -2, "__ptr");
        lua_pushinteger(L, tex->GetWidth());
        lua_setfield(L, -2, "width");
        lua_pushinteger(L, tex->GetHeight());
        lua_setfield(L, -2, "height");
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

void RegisterRenderBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_ClearColor);
    lua_setfield(L, -2, "clearColor");
    lua_pushcfunction(L, lua_GetClearColor);
    lua_setfield(L, -2, "getClearColor");
    lua_pushcfunction(L, lua_SetWireframe);
    lua_setfield(L, -2, "setWireframe");
    lua_pushcfunction(L, lua_IsWireframe);
    lua_setfield(L, -2, "isWireframe");
    lua_pushcfunction(L, lua_DrawSprite);
    lua_setfield(L, -2, "drawSprite");
    lua_pushcfunction(L, lua_DrawRect);
    lua_setfield(L, -2, "drawRect");
    lua_pushcfunction(L, lua_SetLightDir);
    lua_setfield(L, -2, "setLightDir");
    lua_pushcfunction(L, lua_SetLightColor);
    lua_setfield(L, -2, "setLightColor");
    lua_pushcfunction(L, lua_SetAmbientColor);
    lua_setfield(L, -2, "setAmbientColor");
    lua_pushcfunction(L, lua_GetLightDir);
    lua_setfield(L, -2, "getLightDir");
    lua_pushcfunction(L, lua_GetAmbientColor);
    lua_setfield(L, -2, "getAmbientColor");
    lua_pushcfunction(L, lua_SetFogColor);
    lua_setfield(L, -2, "setFogColor");
    lua_pushcfunction(L, lua_SetFogDensity);
    lua_setfield(L, -2, "setFogDensity");
    lua_pushcfunction(L, lua_SetFogRange);
    lua_setfield(L, -2, "setFogRange");
    lua_pushcfunction(L, lua_SetFogEnabled);
    lua_setfield(L, -2, "setFogEnabled");
    lua_pushcfunction(L, lua_IsFogEnabled);
    lua_setfield(L, -2, "isFogEnabled");
    lua_pushcfunction(L, lua_SetPostEffect);
    lua_setfield(L, -2, "setPostEffect");
    lua_pushcfunction(L, lua_GetPostEffect);
    lua_setfield(L, -2, "getPostEffect");
    lua_pushcfunction(L, lua_SetPostIntensity);
    lua_setfield(L, -2, "setPostIntensity");
    lua_pushcfunction(L, lua_GetPostIntensity);
    lua_setfield(L, -2, "getPostIntensity");
    lua_pushcfunction(L, lua_SetFlashlight);
    lua_setfield(L, -2, "setFlashlight");
    lua_pushcfunction(L, lua_DrawText);
    lua_setfield(L, -2, "drawText");
    lua_pushcfunction(L, lua_DrawTextCentered);
    lua_setfield(L, -2, "drawTextCentered");
    lua_pushcfunction(L, lua_AddPointLight);
    lua_setfield(L, -2, "addPointLight");
    lua_pushcfunction(L, lua_RemovePointLight);
    lua_setfield(L, -2, "removePointLight");
    lua_pushcfunction(L, lua_ClearPointLights);
    lua_setfield(L, -2, "clearPointLights");
    lua_pushcfunction(L, lua_SetPointLightPosition);
    lua_setfield(L, -2, "setPointLightPosition");
    lua_pushcfunction(L, lua_SetPointLightColor);
    lua_setfield(L, -2, "setPointLightColor");
    lua_pushcfunction(L, lua_GetPointLightCount);
    lua_setfield(L, -2, "getPointLightCount");
    lua_pushcfunction(L, lua_LoadTexture);
    lua_setfield(L, -2, "loadTexture");
    lua_setglobal(L, "Renderer");
}

} // namespace planet
