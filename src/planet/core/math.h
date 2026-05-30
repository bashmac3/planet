#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>

namespace planet {

namespace Math {

    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 6.28318530717958647692f;
    constexpr float HALF_PI = 1.57079632679489661923f;
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;
    constexpr float EPSILON = 1e-6f;

    float Lerp(float a, float b, float t);
    double Lerp(double a, double b, double t);
    glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
    glm::vec4 Lerp(const glm::vec4& a, const glm::vec4& b, float t);
    glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t);
    glm::quat LookAt(const glm::vec3& from, const glm::vec3& to);

    float Clamp(float value, float min, float max);
    int Clamp(int value, int min, int max);
    double Clamp(double value, double min, double max);
    glm::vec3 Clamp(const glm::vec3& value, float min, float max);

    float SmoothStep(float edge0, float edge1, float x);
    float SmootherStep(float edge0, float edge1, float x);

    float Map(float value, float inMin, float inMax, float outMin, float outMax);
    glm::vec3 Map(const glm::vec3& value, float inMin, float inMax, float outMin, float outMax);

    float Repeat(float t, float length);
    float PingPong(float t, float length);
    float DeltaAngle(float current, float target);
    float MoveTowards(float current, float target, float maxDelta);
    float MoveTowardsAngle(float current, float target, float maxDelta);
    glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDelta);

    float EaseInQuad(float t);
    float EaseOutQuad(float t);
    float EaseInOutQuad(float t);
    float EaseInCubic(float t);
    float EaseOutCubic(float t);
    float EaseInOutCubic(float t);
    float EaseInElastic(float t);
    float EaseOutElastic(float t);
    float EaseInBounce(float t);
    float EaseOutBounce(float t);

    float DistancePointToLine(const glm::vec3& point, const glm::vec3& lineStart, const glm::vec3& lineEnd);
    float DistancePointToPlane(const glm::vec3& point, const glm::vec3& planeNormal, float planeD);

    bool RayIntersectsSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                const glm::vec3& sphereCenter, float sphereRadius, float& t);
    bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                            const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& t);

    glm::vec3 ProjectOnPlane(const glm::vec3& vector, const glm::vec3& planeNormal);
    glm::vec3 Reflect(const glm::vec3& direction, const glm::vec3& normal);
    glm::vec3 Refract(const glm::vec3& direction, const glm::vec3& normal, float eta);

    float AngleBetween(const glm::vec3& a, const glm::vec3& b);
    glm::vec3 RandomOnUnitSphere();
    glm::vec3 RandomInUnitHemisphere(const glm::vec3& normal);

    bool IsPowerOfTwo(int value);
    int NextPowerOfTwo(int value);
    float SmoothDamp(float current, float target, float& currentVelocity,
                      float smoothTime, float maxSpeed, float dt);
    glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target,
                          glm::vec3& currentVelocity, float smoothTime,
                          float maxSpeed, float dt);

} // namespace Math

} // namespace planet
