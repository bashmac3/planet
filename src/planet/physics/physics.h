#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace planet {

class RigidbodyComponent;
class ColliderComponent;
class Entity;

struct RaycastHit {
    Entity* entity = nullptr;
    glm::vec3 point{0.0f};
    glm::vec3 normal{0.0f};
    float distance = 0.0f;
};

struct OverlapResult {
    Entity* entity = nullptr;
    float distance = 0.0f;
};

class Physics {
public:
    static Physics& Instance();

    void Init();
    void Shutdown();
    void Update(float dt);

    void SetGravity(const glm::vec3& gravity) { m_gravity = gravity; }
    glm::vec3 GetGravity() const { return m_gravity; }

    void Integrate(RigidbodyComponent* rb, Entity* entity, float dt);

    bool Raycast(const glm::vec3& origin, const glm::vec3& direction,
                 float maxDist, RaycastHit& hitInfo) const;
    bool RaycastAll(const glm::vec3& origin, const glm::vec3& direction,
                    float maxDist, std::vector<RaycastHit>& hits) const;

    std::vector<OverlapResult> OverlapSphere(const glm::vec3& center, float radius) const;
    std::vector<OverlapResult> OverlapBox(const glm::vec3& center, const glm::vec3& halfSize) const;

    bool CheckCollision(Entity* a, Entity* b) const;
    std::vector<std::pair<Entity*, Entity*>> GetAllCollisions() const;

    void SetSubSteps(int steps);
    int GetSubSteps() const { return m_subSteps; }

private:
    Physics() = default;

    Physics(const Physics&) = delete;
    Physics& operator=(const Physics&) = delete;

    void ResolveCollision(Entity* a, Entity* b);

    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
    int m_subSteps = 1;
};

} // namespace planet
