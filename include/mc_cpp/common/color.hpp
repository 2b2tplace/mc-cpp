#pragma once

#include <algorithm>
#include <cstdint>
#include <array>
#include <cmath>
#include <vector>

namespace mc {

    using RGBA = std::array<uint8_t, 4>;
    using HSVA = std::array<float, 4>;

    static constexpr auto WHITE = RGBA {255, 255, 255, 255};

    [[nodiscard]]
    auto colorBrightness(const RGBA &rgba) -> float;

    [[nodiscard]]
    auto multiplyColor(const RGBA &rgba, float fr, float fg, float fb, float fa = 1.0f) -> RGBA;

    [[nodiscard]]
    auto multiplyColor(const RGBA &rgba, float frgb, float fa = 1.0f) -> RGBA;

    [[nodiscard]]
    auto toHSVA(const RGBA &rgba) -> HSVA;

    [[nodiscard]]
    auto toRGBA(const HSVA &hsva) -> RGBA;

    [[nodiscard]]
    auto packARGB(const RGBA &rgba) -> int32_t;

    [[nodiscard]]
    auto unpackARGB(int32_t argb) -> RGBA;

    [[nodiscard]]
    auto interpolate(const RGBA &rgba1, const RGBA &rgba2, float interpolationValue) -> RGBA;

    [[nodiscard]]
    auto interpolate(const std::vector<RGBA> &colors, float interpolationValue) -> RGBA;
}
