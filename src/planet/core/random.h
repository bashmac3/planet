#pragma once

#include <glm/glm.hpp>
#include <random>
#include <cstdint>

namespace planet {

class Random {
public:
    static Random& Instance();

    void SetSeed(uint64_t seed);
    uint64_t GetSeed() const { return m_seed; }

    float Range(float min, float max);
    int Range(int min, int max);
    double Range(double min, double max);

    float Value() { return Range(0.0f, 1.0f); }
    float Sign() { return Range(0, 2) == 0 ? -1.0f : 1.0f; }

    glm::vec3 Vec3(float min = -1.0f, float max = 1.0f);
    glm::vec3 UnitVector();
    glm::vec3 InsideUnitSphere();
    glm::vec3 InsideUnitCube();

    glm::vec2 Vec2(float min = -1.0f, float max = 1.0f);
    glm::vec2 InsideUnitCircle();

    float Gaussian(float mean = 0.0f, float stddev = 1.0f);

    template<typename T>
    T Choice(const std::vector<T>& items) {
        if (items.empty()) return T{};
        return items[Range(0, static_cast<int>(items.size()) - 1)];
    }

    void Shuffle(std::vector<int>& items);

private:
    Random() = default;
    std::mt19937_64 m_rng;
    uint64_t m_seed = 12345;
};

} // namespace planet
