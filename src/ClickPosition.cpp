#include "ClickPosition.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>
#include <random>

namespace phosophurs::pathfinder {
namespace {
    // Thread-local RNG avoids reseeding for every click and is safe if the
    // pathfinder later moves input scheduling onto a dedicated thread.
    std::mt19937& randomEngine() {
        thread_local std::mt19937 engine(std::random_device{}());
        return engine;
    }
}

MouseOffset getHumanisedMouseOffset(bool framePerfect) {
    // Exact timing/position is sacred for frame-perfect inputs.
    if (framePerfect) return {};

    auto mod = geode::Mod::get();
    if (!mod->getSettingValue<bool>("humanised-clicks-enabled")) return {};

    const float configuredRadius =
        mod->getSettingValue<float>("humanised-click-strength");
    const float radius = std::clamp(configuredRadius, 0.f, 12.f);
    if (radius <= 0.f) return {};

    // Uniform distribution over a disk, rather than a square, so the maximum
    // Euclidean displacement is exactly the configured radius.
    std::uniform_real_distribution<float> angleDistribution(
        0.f, 6.2831853071795864769f
    );
    std::uniform_real_distribution<float> radialDistribution(0.f, 1.f);

    const float angle = angleDistribution(randomEngine());
    const float distance = radius * std::sqrt(radialDistribution(randomEngine()));

    return {
        std::cos(angle) * distance,
        std::sin(angle) * distance
    };
}
}
