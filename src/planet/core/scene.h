#pragma once

#include <vector>
#include <memory>
#include <string>
#include "planet/core/camera.h"

namespace planet {

class Entity;
class Camera;

class Scene {
public:
    static Scene& Instance();

    void Load();
    void Unload();
    void Update(double dt);

    Entity* CreateEntity(const std::string& name = "Entity");
    Entity* CreateEntityFromPrefab(const std::string& prefabPath);
    void DestroyEntity(Entity* entity);
    Entity* FindEntity(const std::string& name) const;

    Camera* GetActiveCamera() const { return m_activeCamera.get(); }
    void SetActiveCamera(std::unique_ptr<Camera> camera) { m_activeCamera = std::move(camera); }

    void Clear();

    const std::vector<std::unique_ptr<Entity>>& GetEntities() const { return m_entities; }
    std::vector<Entity*> GetEntitiesWithTag(const std::string& tag) const;
    std::vector<Entity*> FindEntitiesByName(const std::string& name) const;
    size_t GetEntityCount() const { return m_entities.size(); }
    void DestroyAllWithTag(const std::string& tag);
    Entity* GetEntityAtIndex(size_t index) const;

    void SetAmbientLight(const glm::vec3& color);
    glm::vec3 GetAmbientLight() const { return m_ambientLight; }

private:
    Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unique_ptr<Camera> m_activeCamera;
    glm::vec3 m_ambientLight{0.35f, 0.38f, 0.45f};
};

} // namespace planet
