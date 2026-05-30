#include "planet/ecs/system.h"
#include "planet/ecs/component.h"
#include "planet/core/scene.h"
#include "planet/render/model_renderer.h"
#include "planet/render/sprite_renderer.h"
#include "planet/physics/physics.h"
#include "planet/debug/debug_server.h"
#include "lua/lua_engine.h"

namespace planet {

SystemManager& SystemManager::Instance() {
    static SystemManager instance;
    return instance;
}

void SystemManager::RegisterSystem(std::unique_ptr<System> system) {
    m_systems.push_back(std::move(system));
}

void SystemManager::UpdateAll(double dt) {
    for (auto& sys : m_systems) {
        sys->Update(dt);
    }
}

void MeshRenderSystem::Update(double dt) {
    auto& entities = Scene::Instance().GetEntities();
    for (auto& entity : entities) {
        auto* meshComp = entity->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->visible || !meshComp->mesh) continue;

        if (meshComp->wireframe) {
            ModelRenderer::Instance().SubmitWireframe(
                meshComp->mesh, entity->GetTransformMatrix(), meshComp->color);
        } else {
            ModelRenderer::Instance().SubmitMesh(
                meshComp->mesh, meshComp->texture, entity->GetTransformMatrix(), meshComp->color, meshComp->unlit);
        }
    }
}

void SpriteRenderSystem::Update(double dt) {
    auto& entities = Scene::Instance().GetEntities();
    for (auto& entity : entities) {
        auto* spriteComp = entity->GetComponent<SpriteComponent>();
        if (!spriteComp || !spriteComp->visible) continue;

        glm::vec3 pos = entity->GetPosition();
        SpriteRenderer::Instance().DrawSprite(
            spriteComp->texture,
            glm::vec2(pos.x, pos.y),
            spriteComp->size,
            spriteComp->rotation,
            spriteComp->color
        );
    }
}

void PhysicsSystem::Update(double dt) {
    auto& entities = Scene::Instance().GetEntities();
    for (auto& entity : entities) {
        auto* rb = entity->GetComponent<RigidbodyComponent>();
        if (!rb || rb->isKinematic) continue;

        Physics::Instance().Integrate(rb, entity.get(), dt);
    }
}

void ScriptSystem::Update(double dt) {
    auto& lua = LuaRuntime::Instance();
    auto& entities = Scene::Instance().GetEntities();
    for (auto& entity : entities) {
        auto* scriptComp = entity->GetComponent<ScriptComponent>();
        if (!scriptComp) continue;
        lua.RunScriptComponent(scriptComp, entity.get(), dt);
        DebugServer::Instance().NotifyScriptInvoke(entity.get(), scriptComp->scriptPath);
    }
}

} // namespace planet
