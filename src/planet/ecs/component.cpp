#include "planet/ecs/component.h"
#include "planet/render/renderer.h"
#include "planet/render/shader.h"
#include "planet/core/random.h"
#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

#ifndef GL_POINTS
#define GL_POINTS 0x0000
#endif
#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE 0x8642
#endif

namespace planet {

static const char* PARTICLE_VERTEX = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

uniform mat4 uView;
uniform mat4 uProjection;

out vec4 vColor;

void main() {
    vColor = aColor;
    vec4 viewPos = uView * vec4(aPos, 1.0);
    gl_PointSize = aSize * (300.0 / -viewPos.z);
    gl_PointSize = clamp(gl_PointSize, 1.0, 200.0);
    gl_Position = uProjection * viewPos;
}
)";

static const char* PARTICLE_FRAGMENT = R"(
#version 410 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;
    float alpha = smoothstep(0.5, 0.2, dist) * vColor.a;
    FragColor = vec4(vColor.rgb, alpha);
}
)";

struct ParticleVertex {
    float px, py, pz;
    float r, g, b, a;
    float size;
};

glm::vec3 TransformComponent::Forward() const {
    return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 TransformComponent::Right() const {
    return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 TransformComponent::Up() const {
    return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 TransformComponent::GetMatrix() const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
    m *= glm::mat4_cast(rotation);
    m = glm::scale(m, scale);
    return m;
}

void ParticleEmitterComponent::Play() {
    playing = true;
}

void ParticleEmitterComponent::Stop() {
    playing = false;
}

void ParticleEmitterComponent::Emit(int count) {
    Entity* owner = GetOwner();
    glm::vec3 pos = owner ? owner->GetPosition() : glm::vec3(0.0f);

    for (int i = 0; i < count && (int)particles.size() < maxParticles; i++) {
        Particle p;
        p.position = pos;
        p.velocity = Random::Instance().InsideUnitSphere() * speed;
        p.color = startColor;
        p.size = startSize;
        p.maxLifetime = lifetime;
        p.lifetime = lifetime;
        particles.push_back(p);
    }
}

void ParticleEmitterComponent::OnInit() {
}

void ParticleEmitterComponent::OnUpdate(double dt) {
    if (!playing) return;

    Entity* owner = GetOwner();
    glm::vec3 emitterPos = owner ? owner->GetPosition() : glm::vec3(0.0f);

    float emitInterval = 1.0f / emissionRate;
    emissionTimer += static_cast<float>(dt);
    while (emissionTimer >= emitInterval && (int)particles.size() < maxParticles) {
        emissionTimer -= emitInterval;
        Particle p;
        p.position = emitterPos;
        p.velocity = Random::Instance().InsideUnitSphere() * speed;
        p.color = startColor;
        p.size = startSize;
        p.maxLifetime = lifetime;
        p.lifetime = lifetime;
        particles.push_back(p);
    }

    float dtf = static_cast<float>(dt);
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->lifetime -= dtf;
        if (it->lifetime <= 0.0f) {
            it = particles.erase(it);
            continue;
        }

        it->velocity += gravityModifier * dtf;
        it->position += it->velocity * dtf;

        float t = 1.0f - (it->lifetime / it->maxLifetime);
        it->color = glm::mix(startColor, endColor, t);
        it->size = glm::mix(startSize, endSize, t);
        ++it;
    }

    if (particles.empty() && !looping) {
        playing = false;
    }
}

void ParticleEmitterComponent::OnRender() {
    if (particles.empty()) return;

    if (!m_gpuInitialized) {
        InitGPUResources();
    }

    std::vector<ParticleVertex> verts;
    verts.reserve(particles.size());
    for (auto& p : particles) {
        ParticleVertex pv;
        pv.px = p.position.x; pv.py = p.position.y; pv.pz = p.position.z;
        pv.r = p.color.r; pv.g = p.color.g; pv.b = p.color.b; pv.a = p.color.a;
        pv.size = p.size;
        verts.push_back(pv);
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(ParticleVertex), verts.data());

    m_shader->Bind();
    auto& renderer = Renderer::Instance();
    m_shader->SetMat4("uView", renderer.GetViewMatrix());
    m_shader->SetMat4("uProjection", renderer.GetProjectionMatrix());

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(verts.size()));

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_PROGRAM_POINT_SIZE);

    m_shader->Unbind();
    glBindVertexArray(0);
}

void ParticleEmitterComponent::OnDestroy() {
    DestroyGPUResources();
}

void ParticleEmitterComponent::InitGPUResources() {
    m_shader = new Shader();
    m_shader->LoadFromSource(PARTICLE_VERTEX, PARTICLE_FRAGMENT);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(ParticleVertex), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, px));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, r));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, size));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    m_gpuInitialized = true;
}

void ParticleEmitterComponent::DestroyGPUResources() {
    if (m_gpuInitialized) {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        delete m_shader;
        m_shader = nullptr;
        m_vao = 0;
        m_vbo = 0;
        m_gpuInitialized = false;
    }
}

} // namespace planet
