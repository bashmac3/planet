#pragma once

#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include <vector>

namespace planet {

class System {
public:
    virtual ~System() = default;
    virtual void Update(double dt) = 0;
};

class MeshRenderSystem : public System {
public:
    void Update(double dt) override;
};

class SpriteRenderSystem : public System {
public:
    void Update(double dt) override;
};

class PhysicsSystem : public System {
public:
    void Update(double dt) override;
};

class ScriptSystem : public System {
public:
    void Update(double dt) override;
};

class SystemManager {
public:
    static SystemManager& Instance();

    void RegisterSystem(std::unique_ptr<System> system);
    void UpdateAll(double dt);

    template<typename T>
    T* GetSystem() {
        for (auto& sys : m_systems) {
            T* ptr = dynamic_cast<T*>(sys.get());
            if (ptr) return ptr;
        }
        return nullptr;
    }

private:
    SystemManager() = default;
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace planet
