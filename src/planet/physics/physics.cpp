#include "planet/physics/physics.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/core/scene.h"
#include "planet/core/logger.h"
#include <algorithm>
#include <cmath>

namespace planet {

Physics& Physics::Instance() {
    static Physics instance;
    return instance;
}

void Physics::Init() {
    LOG_INFO() << "[Physics] Initialized (gravity: " << m_gravity.y << " m/s^2)";
}

void Physics::Shutdown() {
    LOG_INFO() << "[Physics] Shutdown";
}

void Physics::Update(float dt) {
    if (m_subSteps > 1) {
        float subDt = dt / static_cast<float>(m_subSteps);
        for (int i = 0; i < m_subSteps; i++) {
            for (auto& entity : Scene::Instance().GetEntities()) {
                auto* rb = entity->GetComponent<RigidbodyComponent>();
                if (!rb || rb->isKinematic) continue;
                Integrate(rb, entity.get(), subDt);
            }
        }
    }
}

void Physics::Integrate(RigidbodyComponent* rb, Entity* entity, float dt) {
    if (!rb || !entity) return;

    if (rb->useGravity) {
        rb->velocity += m_gravity * dt;
    }

    rb->velocity *= (1.0f - rb->drag * dt);

    entity->SetPosition(entity->GetPosition() + rb->velocity * dt);

    rb->angularVelocity *= (1.0f - rb->angularDrag * dt);
}

bool Physics::Raycast(const glm::vec3& origin, const glm::vec3& direction,
                       float maxDist, RaycastHit& hitInfo) const {
    glm::vec3 dir = glm::normalize(direction);
    Entity* closest = nullptr;
    float closestDist = maxDist;

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
        float disc = b * b - 4 * a * c;

        if (disc >= 0) {
            float t = (-b - std::sqrt(disc)) / (2.0f * a);
            if (t > 0.01f && t < closestDist) {
                closestDist = t;
                closest = entity.get();
                hitInfo.entity = entity.get();
                hitInfo.point = origin + dir * t;
                hitInfo.distance = t;
            }
        }
    }

    return closest != nullptr;
}

bool Physics::RaycastAll(const glm::vec3& origin, const glm::vec3& direction,
                          float maxDist, std::vector<RaycastHit>& hits) const {
    glm::vec3 dir = glm::normalize(direction);
    hits.clear();

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
        float disc = b * b - 4 * a * c;

        if (disc >= 0) {
            float t = (-b - std::sqrt(disc)) / (2.0f * a);
            if (t > 0.01f && t < maxDist) {
                RaycastHit hit;
                hit.entity = entity.get();
                hit.point = origin + dir * t;
                hit.distance = t;
                hits.push_back(hit);
            }
        }
    }

    std::sort(hits.begin(), hits.end(), [](const RaycastHit& a, const RaycastHit& b) {
        return a.distance < b.distance;
    });

    return !hits.empty();
}

std::vector<OverlapResult> Physics::OverlapSphere(const glm::vec3& center, float radius) const {
    std::vector<OverlapResult> results;
    for (auto& entity : Scene::Instance().GetEntities()) {
        float dist = glm::distance(center, entity->GetPosition());
        auto scale = entity->GetScale();
        float entityRadius = std::max({scale.x, scale.y, scale.z}) * 0.5f;
        if (dist < radius + entityRadius) {
            results.push_back({entity.get(), dist});
        }
    }
    std::sort(results.begin(), results.end(), [](const OverlapResult& a, const OverlapResult& b) {
        return a.distance < b.distance;
    });
    return results;
}

std::vector<OverlapResult> Physics::OverlapBox(const glm::vec3& center, const glm::vec3& halfSize) const {
    std::vector<OverlapResult> results;
    for (auto& entity : Scene::Instance().GetEntities()) {
        auto pos = entity->GetPosition();
        auto scale = entity->GetScale();
        float entityRadius = std::max({scale.x, scale.y, scale.z}) * 0.5f;
        glm::vec3 local = pos - center;
        if (std::fabs(local.x) < halfSize.x + entityRadius &&
            std::fabs(local.y) < halfSize.y + entityRadius &&
            std::fabs(local.z) < halfSize.z + entityRadius) {
            results.push_back({entity.get(), glm::length(local)});
        }
    }
    std::sort(results.begin(), results.end(), [](const OverlapResult& a, const OverlapResult& b) {
        return a.distance < b.distance;
    });
    return results;
}

bool Physics::CheckCollision(Entity* a, Entity* b) const {
    auto* collA = a->GetComponent<ColliderComponent>();
    auto* collB = b->GetComponent<ColliderComponent>();
    if (!collA || !collB) return false;

    float dist = glm::distance(a->GetPosition(), b->GetPosition());
    float combinedRadius = std::max({a->GetScale().x, a->GetScale().y, a->GetScale().z}) * 0.5f +
                           std::max({b->GetScale().x, b->GetScale().y, b->GetScale().z}) * 0.5f;
    return dist < combinedRadius;
}

std::vector<std::pair<Entity*, Entity*>> Physics::GetAllCollisions() const {
    std::vector<std::pair<Entity*, Entity*>> collisions;
    auto& entities = Scene::Instance().GetEntities();
    for (size_t i = 0; i < entities.size(); i++) {
        for (size_t j = i + 1; j < entities.size(); j++) {
            if (CheckCollision(entities[i].get(), entities[j].get())) {
                collisions.emplace_back(entities[i].get(), entities[j].get());
            }
        }
    }
    return collisions;
}

void Physics::SetSubSteps(int steps) {
    m_subSteps = std::max(1, steps);
}

void Physics::ResolveCollision(Entity* a, Entity* b) {
    // Simple collision response stub
}

} // namespace planet
