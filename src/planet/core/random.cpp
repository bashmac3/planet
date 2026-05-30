#include "planet/core/random.h"
#include <algorithm>

namespace planet {

Random& Random::Instance() {
    static Random instance;
    instance.m_rng.seed(instance.m_seed);
    return instance;
}

void Random::SetSeed(uint64_t seed) {
    m_seed = seed;
    m_rng.seed(seed);
}

float Random::Range(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

int Random::Range(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

double Random::Range(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(m_rng);
}

glm::vec3 Random::Vec3(float min, float max) {
    return glm::vec3(Range(min, max), Range(min, max), Range(min, max));
}

glm::vec3 Random::UnitVector() {
    return glm::normalize(InsideUnitSphere());
}

glm::vec3 Random::InsideUnitSphere() {
    float theta = Range(0.0f, static_cast<float>(2.0 * M_PI));
    float phi = std::acos(Range(-1.0f, 1.0f));
    float r = std::cbrt(Range(0.0f, 1.0f));
    return glm::vec3(
        r * std::sin(phi) * std::cos(theta),
        r * std::cos(phi),
        r * std::sin(phi) * std::sin(theta)
    );
}

glm::vec3 Random::InsideUnitCube() {
    return Vec3(-1.0f, 1.0f);
}

glm::vec2 Random::Vec2(float min, float max) {
    return glm::vec2(Range(min, max), Range(min, max));
}

glm::vec2 Random::InsideUnitCircle() {
    float angle = Range(0.0f, static_cast<float>(2.0 * M_PI));
    float r = std::sqrt(Range(0.0f, 1.0f));
    return glm::vec2(r * std::cos(angle), r * std::sin(angle));
}

float Random::Gaussian(float mean, float stddev) {
    std::normal_distribution<float> dist(mean, stddev);
    return dist(m_rng);
}

void Random::Shuffle(std::vector<int>& items) {
    std::shuffle(items.begin(), items.end(), m_rng);
}

} // namespace planet
