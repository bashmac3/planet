#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <functional>
#include <any>
#include <typeindex>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace planet {

class Component;

class Entity {
public:
    Entity(const std::string& name = "Entity");
    ~Entity();

    void Update(double dt);
    void Render();

    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    void AddTag(const std::string& tag) { m_tags.insert(tag); }
    void RemoveTag(const std::string& tag) { m_tags.erase(tag); }
    bool HasTag(const std::string& tag) const { return m_tags.count(tag) > 0; }
    const std::unordered_set<std::string>& GetTags() const { return m_tags; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = comp.get();
        comp->SetOwner(this);
        m_components[std::type_index(typeid(T))] = std::move(comp);
        return ptr;
    }

    template<typename T>
    T* GetComponent() {
        auto it = m_components.find(std::type_index(typeid(T)));
        return (it != m_components.end()) ? static_cast<T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    bool HasComponent() {
        return m_components.find(std::type_index(typeid(T))) != m_components.end();
    }

    template<typename T>
    void RemoveComponent() {
        m_components.erase(std::type_index(typeid(T)));
    }

    glm::vec3 GetPosition() const { return m_position; }
    void SetPosition(const glm::vec3& pos) { m_position = pos; m_transformDirty = true; }

    glm::quat GetRotation() const { return m_rotation; }
    void SetRotation(const glm::quat& rot) { m_rotation = rot; m_transformDirty = true; }

    glm::vec3 GetScale() const { return m_scale; }
    void SetScale(const glm::vec3& scale) { m_scale = scale; m_transformDirty = true; }

    void SetEulerAngles(const glm::vec3& euler);
    glm::vec3 GetEulerAngles() const;

    glm::mat4 GetTransformMatrix();

    glm::vec3 Forward() const;
    glm::vec3 Right() const;
    glm::vec3 Up() const;

    void SetParent(Entity* parent);
    Entity* GetParent() const { return m_parent; }
    const std::vector<Entity*>& GetChildren() const { return m_children; }
    bool IsRoot() const { return m_parent == nullptr; }
    bool HasParent() const { return m_parent != nullptr; }
    bool IsAncestorOf(Entity* other) const;
    std::vector<Entity*> GetChildrenRecursive() const;
    void DestroyChildren();
    void DetachFromParent();

    void Translate(const glm::vec3& delta);
    void Rotate(const glm::vec3& eulerDeg);
    void RotateAround(const glm::vec3& point, const glm::vec3& axis, float angleDeg);
    void LookAt(const glm::vec3& target);
    glm::vec3 GetWorldPosition() const;
    glm::quat GetWorldRotation() const;
    glm::vec3 GetWorldScale() const;
    void SetWorldPosition(const glm::vec3& pos);
    void SetWorldRotation(const glm::quat& rot);
    void SetWorldScale(const glm::vec3& scale);

    template<typename T>
    std::vector<T*> GetComponentsInChildren() {
        std::vector<T*> results;
        auto comp = GetComponent<T>();
        if (comp) results.push_back(comp);
        for (auto* child : m_children) {
            auto childComps = child->GetComponentsInChildren<T>();
            results.insert(results.end(), childComps.begin(), childComps.end());
        }
        return results;
    }

    std::vector<std::type_index> GetComponentTypes() const;

    // Utility
    float DistanceTo(const Entity* other) const;
    float DistanceTo(const glm::vec3& point) const;

private:
    std::string m_name;
    bool m_active = true;
    std::unordered_set<std::string> m_tags;

    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_scale{1.0f};

    glm::mat4 m_transformMatrix{1.0f};
    bool m_transformDirty = true;

    Entity* m_parent = nullptr;
    std::vector<Entity*> m_children;

    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};

class Component {
public:
    virtual ~Component() = default;

    Entity* GetOwner() const { return m_owner; }

    virtual void OnInit() {}
    virtual void OnUpdate(double dt) {}
    virtual void OnRender() {}
    virtual void OnDestroy() {}

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

protected:
    friend class Entity;
    void SetOwner(Entity* owner) { m_owner = owner; }

    Entity* m_owner = nullptr;
    bool m_active = true;
};

} // namespace planet
