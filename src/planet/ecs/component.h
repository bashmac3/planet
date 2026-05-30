#pragma once

#include "planet/ecs/ecs.h"
#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>

namespace planet {

class TransformComponent : public Component {
public:
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    glm::vec3 Forward() const;
    glm::vec3 Right() const;
    glm::vec3 Up() const;
    glm::mat4 GetMatrix() const;
};

class MeshComponent : public Component {
public:
    class Mesh* mesh = nullptr;
    class Texture* texture = nullptr;
    glm::vec4 color{1.0f};
    bool wireframe = false;
    bool visible = true;
    bool unlit = false;
};

class SpriteComponent : public Component {
public:
    class Texture* texture = nullptr;
    glm::vec2 size{1.0f};
    glm::vec4 color{1.0f};
    float rotation = 0.0f;
    int layer = 0;
    bool visible = true;
};

class RigidbodyComponent : public Component {
public:
    float mass = 1.0f;
    glm::vec3 velocity{0.0f};
    glm::vec3 angularVelocity{0.0f};
    bool useGravity = true;
    bool isKinematic = false;
    float drag = 0.0f;
    float angularDrag = 0.05f;
};

class ColliderComponent : public Component {
public:
    enum Type { Box, Sphere, Capsule, Mesh };
    Type type = Box;
    glm::vec3 size{1.0f};
    glm::vec3 center{0.0f};
    bool isTrigger = false;
};

class ScriptComponent : public Component {
public:
    std::string scriptPath;
};

class LightComponent : public Component {
public:
    enum Type { Directional, Point, Spot };
    Type type = Point;
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
    float spotAngle = 30.0f;
};

class CameraComponent : public Component {
public:
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isOrthographic = false;
    float orthoSize = 10.0f;
    bool isMain = false;
};

class AudioSourceComponent : public Component {
public:
    std::string soundPath;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnStart = true;
    float minDistance = 1.0f;
    float maxDistance = 50.0f;
    bool is3D = true;
    int sourceId = -1;
};

struct Particle {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec4 color{1.0f};
    float size = 0.1f;
    float lifetime = 1.0f;
    float maxLifetime = 1.0f;
};

class ParticleEmitterComponent : public Component {
public:
    int maxParticles = 100;
    float emissionRate = 10.0f;
    float lifetime = 1.0f;
    float speed = 5.0f;
    glm::vec4 startColor{1.0f};
    glm::vec4 endColor{0.0f, 0.0f, 0.0f, 0.0f};
    float startSize = 0.1f;
    float endSize = 0.0f;
    glm::vec3 gravityModifier{0.0f, -0.5f, 0.0f};
    bool looping = true;
    bool playing = false;
    class Texture* texture = nullptr;

    std::vector<Particle> particles;
    float emissionTimer = 0.0f;

    void Play();
    void Stop();
    void Emit(int count);

    void OnInit() override;
    void OnUpdate(double dt) override;
    void OnRender() override;
    void OnDestroy() override;

private:
    void InitGPUResources();
    void DestroyGPUResources();
    class Shader* m_shader = nullptr;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    bool m_gpuInitialized = false;
};

class AnimationComponent : public Component {
public:
    std::string animationName;
    float time = 0.0f;
    float speed = 1.0f;
    bool looping = true;
    bool playing = false;
    struct Keyframe {
        float time;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };
    std::vector<Keyframe> keyframes;
};

class BillboardComponent : public Component {
public:
    class Texture* texture = nullptr;
    glm::vec2 size{1.0f};
    glm::vec4 color{1.0f};
    bool lockY = true;
};

class TriggerComponent : public Component {
public:
    glm::vec3 size{1.0f};
    bool oneShot = false;
    std::string onEnterScript;
    std::string onExitScript;
    bool fired = false;
    std::unordered_set<Entity*> overlapping;
};

} // namespace planet
