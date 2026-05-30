#include "planet/core/camera.h"
#include "planet/ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>

namespace planet {

glm::mat4 Camera::GetViewMatrix() {
    if (m_viewDirty) {
        m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
        m_viewDirty = false;
    }
    return m_viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) {
    if (m_projDirty) {
        if (m_projectionType == ProjectionType::Perspective) {
            m_projectionMatrix = glm::perspective(glm::radians(m_fov * m_zoom), aspectRatio,
                                                   m_nearPlane, m_farPlane);
        } else {
            float halfSize = (m_orthoSize * 0.5f) / m_zoom;
            m_projectionMatrix = glm::ortho(-halfSize * aspectRatio, halfSize * aspectRatio,
                                             -halfSize, halfSize,
                                             m_nearPlane, m_farPlane);
        }
        m_projDirty = false;
    }
    return m_projectionMatrix;
}

void Camera::SetRotation(const glm::quat& rot) {
    glm::vec3 fwd = rot * glm::vec3(0.0f, 0.0f, -1.0f);
    m_target = m_position + fwd;
    m_up = rot * glm::vec3(0.0f, 1.0f, 0.0f);
    m_viewDirty = true;
}

glm::quat Camera::GetRotation() const {
    glm::vec3 fwd = glm::normalize(m_target - m_position);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    if (std::fabs(glm::dot(fwd, worldUp)) > 0.999f) {
        worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::vec3 right = glm::normalize(glm::cross(fwd, worldUp));
    glm::vec3 up = glm::cross(right, fwd);
    return glm::quatLookAt(fwd, up);
}

void Camera::Orbit(float yawDeg, float pitchDeg) {
    m_yaw += yawDeg;
    m_pitch += pitchDeg;
    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

    glm::vec3 dir;
    dir.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    dir.y = sin(glm::radians(m_pitch));
    dir.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    dir = glm::normalize(dir);

    float dist = glm::distance(m_position, m_target);
    m_position = m_target - dir * dist;
    m_viewDirty = true;
}

void Camera::LookAt(const glm::vec3& point) {
    m_target = point;
    m_viewDirty = true;
}

void Camera::LookAtEntity(Entity* entity) {
    if (entity) {
        m_target = entity->GetPosition();
        m_viewDirty = true;
    }
}

void Camera::SetPitch(float pitchDeg) {
    m_pitch = glm::clamp(pitchDeg, -89.0f, 89.0f);
    Orbit(0.0f, 0.0f); // recompute
}

void Camera::SetYaw(float yawDeg) {
    m_yaw = yawDeg;
    Orbit(0.0f, 0.0f);
}

glm::vec3 Camera::ScreenToWorldPoint(float screenX, float screenY, float screenZ, float viewW, float viewH) {
    float aspect = viewW / viewH;
    glm::mat4 proj = GetProjectionMatrix(aspect);
    glm::mat4 view = GetViewMatrix();
    glm::mat4 inv = glm::inverse(proj * view);

    float ndcX = (2.0f * screenX) / viewW - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / viewH;
    glm::vec4 clip(ndcX, ndcY, screenZ * 2.0f - 1.0f, 1.0f);
    glm::vec4 world = inv * clip;
    return glm::vec3(world) / world.w;
}

glm::vec2 Camera::WorldToScreenPoint(const glm::vec3& worldPos, float viewW, float viewH) {
    float aspect = viewW / viewH;
    glm::mat4 viewProj = GetProjectionMatrix(aspect) * GetViewMatrix();
    glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);

    if (clip.w == 0.0f) return glm::vec2(-1.0f);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    float sx = (ndc.x + 1.0f) * 0.5f * viewW;
    float sy = (1.0f - ndc.y) * 0.5f * viewH;
    return glm::vec2(sx, sy);
}

Ray Camera::ScreenToRay(float screenX, float screenY, float viewW, float viewH) {
    Ray ray;
    ray.origin = m_position;
    glm::vec3 nearPoint = ScreenToWorldPoint(screenX, screenY, 0.0f, viewW, viewH);
    glm::vec3 farPoint = ScreenToWorldPoint(screenX, screenY, 1.0f, viewW, viewH);
    ray.direction = glm::normalize(farPoint - nearPoint);
    return ray;
}

float Camera::GetFrustumWidthAtDistance(float dist, float aspectRatio) const {
    if (m_projectionType == ProjectionType::Perspective) {
        return 2.0f * dist * tan(glm::radians(m_fov * m_zoom) * 0.5f) * aspectRatio;
    }
    return m_orthoSize * aspectRatio / m_zoom;
}

float Camera::GetFrustumHeightAtDistance(float dist) const {
    if (m_projectionType == ProjectionType::Perspective) {
        return 2.0f * dist * tan(glm::radians(m_fov * m_zoom) * 0.5f);
    }
    return m_orthoSize / m_zoom;
}

void Camera::SetZoom(float zoom) {
    m_zoom = glm::clamp(zoom, 0.1f, 100.0f);
    m_projDirty = true;
}

} // namespace planet
