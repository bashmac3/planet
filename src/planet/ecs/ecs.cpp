#include "planet/ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cmath>

namespace planet {

Entity::Entity(const std::string& name) : m_name(name) {}

Entity::~Entity() {
    for (auto& [type, comp] : m_components) {
        comp->OnDestroy();
    }
    m_components.clear();
}

void Entity::Update(double dt) {
    if (!m_active) return;
    for (auto& [type, comp] : m_components) {
        if (comp->IsActive()) {
            comp->OnUpdate(dt);
        }
    }
}

void Entity::Render() {
    if (!m_active) return;
    for (auto& [type, comp] : m_components) {
        if (comp->IsActive()) {
            comp->OnRender();
        }
    }
}

void Entity::SetEulerAngles(const glm::vec3& euler) {
    m_rotation = glm::quat(glm::radians(euler));
    m_transformDirty = true;
}

glm::vec3 Entity::GetEulerAngles() const {
    return glm::degrees(glm::eulerAngles(m_rotation));
}

glm::mat4 Entity::GetTransformMatrix() {
    if (m_transformDirty) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
        glm::mat4 rotation = glm::mat4_cast(m_rotation);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);
        m_transformMatrix = translation * rotation * scale;
        m_transformDirty = false;
    }
    return m_transformMatrix;
}

glm::vec3 Entity::Forward() const {
    return m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Entity::Right() const {
    return m_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Entity::Up() const {
    return m_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void Entity::SetParent(Entity* parent) {
    if (m_parent == parent) return;
    if (m_parent) {
        auto& siblings = m_parent->m_children;
        auto it = std::find(siblings.begin(), siblings.end(), this);
        if (it != siblings.end()) siblings.erase(it);
    }
    m_parent = parent;
    if (m_parent) {
        m_parent->m_children.push_back(this);
    }
}

bool Entity::IsAncestorOf(Entity* other) const {
    while (other) {
        if (other->m_parent == this) return true;
        other = other->m_parent;
    }
    return false;
}

std::vector<Entity*> Entity::GetChildrenRecursive() const {
    std::vector<Entity*> result;
    for (auto* child : m_children) {
        result.push_back(child);
        auto grandkids = child->GetChildrenRecursive();
        result.insert(result.end(), grandkids.begin(), grandkids.end());
    }
    return result;
}

void Entity::DestroyChildren() {
    m_children.clear();
}

void Entity::DetachFromParent() {
    SetParent(nullptr);
}

void Entity::Translate(const glm::vec3& delta) {
    m_position += delta;
    m_transformDirty = true;
}

void Entity::Rotate(const glm::vec3& eulerDeg) {
    m_rotation = glm::quat(glm::radians(eulerDeg)) * m_rotation;
    m_transformDirty = true;
}

void Entity::RotateAround(const glm::vec3& point, const glm::vec3& axis, float angleDeg) {
    glm::vec3 offset = m_position - point;
    glm::quat q = glm::angleAxis(glm::radians(angleDeg), glm::normalize(axis));
    offset = q * offset;
    m_position = point + offset;
    m_rotation = q * m_rotation;
    m_transformDirty = true;
}

void Entity::LookAt(const glm::vec3& target) {
    glm::vec3 dir = glm::normalize(target - m_position);
    if (glm::length(dir) < 0.0001f) return;
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    if (std::fabs(glm::dot(dir, worldUp)) > 0.999f) {
        worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    m_rotation = glm::quatLookAt(dir, worldUp);
    m_transformDirty = true;
}

glm::vec3 Entity::GetWorldPosition() const {
    if (m_parent) {
        glm::mat4 parentMatrix = m_parent->GetTransformMatrix();
        return glm::vec3(parentMatrix * glm::vec4(m_position, 1.0f));
    }
    return m_position;
}

glm::quat Entity::GetWorldRotation() const {
    if (m_parent) {
        return m_parent->GetRotation() * m_rotation;
    }
    return m_rotation;
}

glm::vec3 Entity::GetWorldScale() const {
    if (m_parent) {
        return m_parent->GetScale() * m_scale;
    }
    return m_scale;
}

void Entity::SetWorldPosition(const glm::vec3& pos) {
    if (m_parent) {
        glm::mat4 invParent = glm::inverse(m_parent->GetTransformMatrix());
        m_position = glm::vec3(invParent * glm::vec4(pos, 1.0f));
    } else {
        m_position = pos;
    }
    m_transformDirty = true;
}

void Entity::SetWorldRotation(const glm::quat& rot) {
    if (m_parent) {
        m_rotation = glm::inverse(m_parent->GetRotation()) * rot;
    } else {
        m_rotation = rot;
    }
    m_transformDirty = true;
}

void Entity::SetWorldScale(const glm::vec3& scale) {
    if (m_parent) {
        auto ps = m_parent->GetScale();
        m_scale = scale / ps;
    } else {
        m_scale = scale;
    }
    m_transformDirty = true;
}

std::vector<std::type_index> Entity::GetComponentTypes() const {
    std::vector<std::type_index> types;
    for (const auto& [type, comp] : m_components) {
        types.push_back(type);
    }
    return types;
}

float Entity::DistanceTo(const Entity* other) const {
    return glm::distance(m_position, other->m_position);
}

float Entity::DistanceTo(const glm::vec3& point) const {
    return glm::distance(m_position, point);
}

} // namespace planet