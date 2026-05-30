#include "planet/core/math.h"
#include <algorithm>
#include <random>

namespace planet {
namespace Math {

float Lerp(float a, float b, float t) { return a + (b - a) * t; }
double Lerp(double a, double b, double t) { return a + (b - a) * t; }
glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) { return a + (b - a) * t; }
glm::vec4 Lerp(const glm::vec4& a, const glm::vec4& b, float t) { return a + (b - a) * t; }

glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t) {
    return glm::slerp(a, b, t);
}

glm::quat LookAt(const glm::vec3& from, const glm::vec3& to) {
    glm::vec3 dir = glm::normalize(to - from);
    if (glm::length(dir) < EPSILON) return glm::quat(1, 0, 0, 0);
    glm::vec3 up(0, 1, 0);
    if (std::fabs(glm::dot(dir, up)) > 0.999f) up = glm::vec3(0, 0, 1);
    return glm::quatLookAt(dir, up);
}

float Clamp(float value, float min, float max) { return std::max(min, std::min(max, value)); }
int Clamp(int value, int min, int max) { return std::max(min, std::min(max, value)); }
double Clamp(double value, double min, double max) { return std::max(min, std::min(max, value)); }
glm::vec3 Clamp(const glm::vec3& value, float min, float max) {
    return glm::vec3(Clamp(value.x, min, max), Clamp(value.y, min, max), Clamp(value.z, min, max));
}

float SmoothStep(float edge0, float edge1, float x) {
    float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float SmootherStep(float edge0, float edge1, float x) {
    float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Map(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

glm::vec3 Map(const glm::vec3& value, float inMin, float inMax, float outMin, float outMax) {
    return glm::vec3(Map(value.x, inMin, inMax, outMin, outMax),
                     Map(value.y, inMin, inMax, outMin, outMax),
                     Map(value.z, inMin, inMax, outMin, outMax));
}

float Repeat(float t, float length) {
    return Clamp(t - std::floor(t / length) * length, 0.0f, length);
}

float PingPong(float t, float length) {
    t = Repeat(t, length * 2.0f);
    return length - std::fabs(t - length);
}

float DeltaAngle(float current, float target) {
    float delta = std::fmod((target - current), 360.0f);
    if (delta > 180.0f) delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    return delta;
}

float MoveTowards(float current, float target, float maxDelta) {
    if (std::fabs(target - current) <= maxDelta) return target;
    return current + std::copysign(maxDelta, target - current);
}

float MoveTowardsAngle(float current, float target, float maxDelta) {
    float delta = DeltaAngle(current, target);
    if (std::fabs(delta) <= maxDelta) return target;
    return current + std::copysign(maxDelta, delta);
}

glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDelta) {
    glm::vec3 delta = target - current;
    float dist = glm::length(delta);
    if (dist <= maxDelta || dist < EPSILON) return target;
    return current + delta / dist * maxDelta;
}

float EaseInQuad(float t) { return t * t; }
float EaseOutQuad(float t) { return t * (2.0f - t); }
float EaseInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
float EaseInCubic(float t) { return t * t * t; }
float EaseOutCubic(float t) { return (t - 1.0f) * (t - 1.0f) * (t - 1.0f) + 1.0f; }
float EaseInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f; }

float EaseInElastic(float t) {
    const float c4 = TWO_PI / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f :
           -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
}

float EaseOutElastic(float t) {
    const float c4 = TWO_PI / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f :
           std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float EaseInBounce(float t) { return 1.0f - EaseOutBounce(1.0f - t); }

float EaseOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1.0f / d1) return n1 * t * t;
    else if (t < 2.0f / d1) return n1 * (t -= 1.5f / d1) * t + 0.75f;
    else if (t < 2.5f / d1) return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    else return n1 * (t -= 2.625f / d1) * t + 0.984375f;
}

float DistancePointToLine(const glm::vec3& point, const glm::vec3& lineStart, const glm::vec3& lineEnd) {
    glm::vec3 lineVec = lineEnd - lineStart;
    float len = glm::length(lineVec);
    if (len < EPSILON) return glm::distance(point, lineStart);
    glm::vec3 toPoint = point - lineStart;
    float t = glm::dot(toPoint, lineVec) / (len * len);
    t = Clamp(t, 0.0f, 1.0f);
    return glm::distance(point, lineStart + t * lineVec);
}

float DistancePointToPlane(const glm::vec3& point, const glm::vec3& planeNormal, float planeD) {
    return std::fabs(glm::dot(point, planeNormal) - planeD);
}

bool RayIntersectsSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                          const glm::vec3& sphereCenter, float sphereRadius, float& t) {
    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) return false;
    float sqrtDisc = std::sqrt(disc);
    float t0 = (-b - sqrtDisc) / (2.0f * a);
    float t1 = (-b + sqrtDisc) / (2.0f * a);
    t = (t0 < t1) ? t0 : t1;
    return t >= 0.0f;
}

bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                        const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& t) {
    float tmin = 0.0f, tmax = 1e30f;
    for (int i = 0; i < 3; i++) {
        float invD = 1.0f / rayDir[i];
        float t0 = (aabbMin[i] - rayOrigin[i]) * invD;
        float t1 = (aabbMax[i] - rayOrigin[i]) * invD;
        if (invD < 0.0f) std::swap(t0, t1);
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
        if (tmax < tmin) return false;
    }
    t = tmin;
    return true;
}

glm::vec3 ProjectOnPlane(const glm::vec3& vector, const glm::vec3& planeNormal) {
    return vector - glm::dot(vector, planeNormal) * planeNormal;
}

glm::vec3 Reflect(const glm::vec3& direction, const glm::vec3& normal) {
    return direction - 2.0f * glm::dot(direction, normal) * normal;
}

glm::vec3 Refract(const glm::vec3& direction, const glm::vec3& normal, float eta) {
    float cosI = -glm::dot(normal, direction);
    float sinI2 = 1.0f - cosI * cosI;
    float sinT2 = eta * eta * sinI2;
    if (sinT2 > 1.0f) return glm::vec3(0.0f); // total internal reflection
    float cosT = std::sqrt(1.0f - sinT2);
    return eta * direction + (eta * cosI - cosT) * normal;
}

float AngleBetween(const glm::vec3& a, const glm::vec3& b) {
    float dot = glm::clamp(glm::dot(glm::normalize(a), glm::normalize(b)), -1.0f, 1.0f);
    return std::acos(dot);
}

glm::vec3 RandomOnUnitSphere() {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    glm::vec3 v;
    do { v = glm::vec3(dist(rng), dist(rng), dist(rng)); } while (glm::length(v) < EPSILON);
    return glm::normalize(v);
}

glm::vec3 RandomInUnitHemisphere(const glm::vec3& normal) {
    glm::vec3 v = RandomOnUnitSphere();
    if (glm::dot(v, normal) < 0.0f) v = -v;
    return v;
}

bool IsPowerOfTwo(int value) { return value > 0 && (value & (value - 1)) == 0; }

int NextPowerOfTwo(int value) {
    int v = value;
    v--;
    v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16;
    return v + 1;
}

float SmoothDamp(float current, float target, float& currentVelocity,
                  float smoothTime, float maxSpeed, float dt) {
    smoothTime = std::max(0.0001f, smoothTime);
    float omega = 2.0f / smoothTime;
    float x = omega * dt;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float maxChange = maxSpeed * smoothTime;
    change = Clamp(change, -maxChange, maxChange);
    float targetTemp = current - change;
    float temp = (currentVelocity + omega * change) * dt;
    currentVelocity = (currentVelocity - omega * temp) * exp;
    return targetTemp + (change + temp) * exp;
}

glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target,
                      glm::vec3& currentVelocity, float smoothTime,
                      float maxSpeed, float dt) {
    glm::vec3 result;
    result.x = SmoothDamp(current.x, target.x, currentVelocity.x, smoothTime, maxSpeed, dt);
    result.y = SmoothDamp(current.y, target.y, currentVelocity.y, smoothTime, maxSpeed, dt);
    result.z = SmoothDamp(current.z, target.z, currentVelocity.z, smoothTime, maxSpeed, dt);
    return result;
}

} // namespace Math
} // namespace planet
