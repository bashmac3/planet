#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace planet {

struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
};

class Camera {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    Camera() { UpdateMatrices(); }

    void SetProjectionType(ProjectionType type) { m_projectionType = type; UpdateMatrices(); }
    ProjectionType GetProjectionType() const { return m_projectionType; }

    void SetPosition(const glm::vec3& pos) { m_position = pos; m_viewDirty = true; }
    glm::vec3 GetPosition() const { return m_position; }

    void SetTarget(const glm::vec3& target) { m_target = target; m_viewDirty = true; }
    glm::vec3 GetTarget() const { return m_target; }

    void SetUp(const glm::vec3& up) { m_up = up; m_viewDirty = true; }

    void SetFOV(float fov) { m_fov = fov; m_projDirty = true; }
    float GetFOV() const { return m_fov; }

    void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; m_projDirty = true; }
    float GetNearPlane() const { return m_nearPlane; }

    void SetFarPlane(float farPlane) { m_farPlane = farPlane; m_projDirty = true; }
    float GetFarPlane() const { return m_farPlane; }

    void SetOrthoSize(float size) { m_orthoSize = size; m_projDirty = true; }
    float GetOrthoSize() const { return m_orthoSize; }

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(float aspectRatio);

    glm::vec3 Forward() const { return glm::normalize(m_target - m_position); }
    glm::vec3 Right() const { return glm::normalize(glm::cross(Forward(), m_up)); }
    glm::vec3 Up() const { return glm::normalize(glm::cross(Right(), Forward())); }

    void SetRotation(const glm::quat& rot);
    glm::quat GetRotation() const;
    void Orbit(float yawDeg, float pitchDeg);
    void LookAt(const glm::vec3& point);
    void LookAtEntity(class Entity* entity);
    void SetPitch(float pitchDeg);
    void SetYaw(float yawDeg);
    float GetPitch() const { return m_pitch; }
    float GetYaw() const { return m_yaw; }

    glm::vec3 ScreenToWorldPoint(float screenX, float screenY, float screenZ, float viewW, float viewH);
    glm::vec2 WorldToScreenPoint(const glm::vec3& worldPos, float viewW, float viewH);
    Ray ScreenToRay(float screenX, float screenY, float viewW, float viewH);
    float GetFrustumWidthAtDistance(float dist, float aspectRatio) const;
    float GetFrustumHeightAtDistance(float dist) const;

    void SetZoom(float zoom);
    float GetZoom() const { return m_zoom; }

private:
    void UpdateMatrices() {
        m_viewDirty = true;
        m_projDirty = true;
    }

    ProjectionType m_projectionType = ProjectionType::Perspective;

    glm::vec3 m_position{0.0f, 2.0f, 5.0f};
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    float m_fov = 60.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    float m_orthoSize = 10.0f;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_zoom = 1.0f;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};

    bool m_viewDirty = true;
    bool m_projDirty = true;
};

} // namespace planet
