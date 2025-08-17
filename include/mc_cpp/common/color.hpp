#pragma once

#include <algorithm>
#include <cstdint>
#include <array>
#include <cmath>
#include <vector>

namespace mc {

    using RGBA = std::array<uint8_t, 4>;
    using HSVA = std::array<float, 4>;

    [[nodiscard]]
    inline auto colorBrightness(const RGBA& rgba) -> float {
        const auto componentSum = static_cast<uint32_t>(rgba[0])
                                 + static_cast<uint32_t>(rgba[1])
                                 + static_cast<uint32_t>(rgba[2]);

        const auto componentAverage = componentSum / 3;
        return static_cast<float>(componentAverage) / 255.0f;
    }

    [[nodiscard]]
    inline auto multiplyColor(const RGBA& rgba, const float fr, const float fg, const float fb, const float fa = 1.0f) -> RGBA {
        return RGBA {
            static_cast<uint8_t>(std::clamp(static_cast<float>(rgba[0]) * fr, 0.0f, 255.0f)),
            static_cast<uint8_t>(std::clamp(static_cast<float>(rgba[1]) * fg, 0.0f, 255.0f)),
            static_cast<uint8_t>(std::clamp(static_cast<float>(rgba[2]) * fb, 0.0f, 255.0f)),
            static_cast<uint8_t>(std::clamp(static_cast<float>(rgba[3]) * fa, 0.0f, 255.0f))
        };
    }

    [[nodiscard]]
    inline auto multiplyColor(const RGBA& rgba, const float frgb, const float fa = 1.0f) -> RGBA {
        return multiplyColor(rgba, frgb, frgb, frgb, fa);
    }

    [[nodiscard]]
    inline auto toHSVA(const RGBA& rgba) -> HSVA {
        const auto r = static_cast<float>(rgba[0]) / 255.0f;
        const auto g = static_cast<float>(rgba[1]) / 255.0f;
        const auto b = static_cast<float>(rgba[2]) / 255.0f;
        const auto a = static_cast<float>(rgba[3]) / 255.0f;

        const auto max = std::max(r, std::max(g, b));
        const auto min = std::min(r, std::min(g, b));
        const auto delta = max - min;

        HSVA hsv{0.0f, 0.0f, max, a};
        if (max == 0)
            return hsv;

        hsv[1]  = delta / max;

        auto& h = hsv[0];

        if (r == max) {
            h = 60 * ((g - b) / delta);
        } else if (g == max) {
            h = 60 * ((b - r) / delta + 2);
        } else {
            h = 60 * ((r - g) / delta + 4);
        }
        if (h < 0) h += 360;

        return hsv;
    }

    [[nodiscard]]
    inline auto toRGBA(const HSVA& hsva) -> RGBA {
        const auto h = hsva[0];
        const auto s = hsva[1];
        const auto v = hsva[2];

        const auto c = v * s;
        const auto x = c * static_cast<float>(1 - std::fabs(fmod(h / 60.0f, 2) - 1));
        const auto m = v - c;

        float r, g, b;

        if (h >= 0 && h < 60) {
            r = c; g = x; b = 0;
        } else if (h >= 60 && h < 120) {
            r = x; g = c; b = 0;
        } else if (h >= 120 && h < 180) {
            r = 0; g = c; b = x;
        } else if (h >= 180 && h < 240) {
            r = 0; g = x; b = c;
        } else if (h >= 240 && h < 300) {
            r = x; g = 0; b = c;
        } else {
            r = c; g = 0; b = x;
        }
        return RGBA {
            static_cast<uint8_t>((r + m) * 255),
            static_cast<uint8_t>((g + m) * 255),
            static_cast<uint8_t>((b + m) * 255),
            static_cast<uint8_t>(hsva[3] * 255)
        };
    }

    [[nodiscard]]
    inline auto packARGB(const RGBA& rgba) -> int32_t {
        return (rgba[3] & 0xFF) << 24 | (rgba[0] & 0xFF) << 16 | (rgba[1] & 0xFF) << 8 | rgba[2] & 0xFF;
    }

    [[nodiscard]]
    inline auto unpackARGB(const int32_t argb) -> RGBA {
        return RGBA {
            static_cast<uint8_t>(argb >> 16 & 0xFF),
            static_cast<uint8_t>(argb >> 8 & 0xFF),
            static_cast<uint8_t>(argb & 0xFF),
            static_cast<uint8_t>(argb >> 24 & 0xFF)
        };
    }

    [[nodiscard]]
    inline auto interpolate(const RGBA& rgba1, const RGBA& rgba2, const double interpolationValue) -> RGBA {
        const auto t = std::clamp(interpolationValue, 0.0, 1.0);

        auto [r1, g1, b1, a1] = rgba1;
        auto [r2, g2, b2, a2] = rgba2;

        return RGBA {
            static_cast<uint8_t>((1.0 - t) * r1 + t * r2),
            static_cast<uint8_t>((1.0 - t) * g1 + t * g2),
            static_cast<uint8_t>((1.0 - t) * b1 + t * b2),
            static_cast<uint8_t>((1.0 - t) * a1 + t * a2)
        };
    }

    [[nodiscard]]
    inline auto interpolate(const std::vector<RGBA>& colors, const double interpolationValue) -> RGBA {
        const auto n = colors.size();
        const auto clampedInterpolationValue = std::clamp(interpolationValue, 0.0, 1.0);
        const auto relativeInterpolationValue = clampedInterpolationValue * static_cast<double>(n - 1);

        const auto i = static_cast<size_t>(floor(relativeInterpolationValue));
        const auto t = relativeInterpolationValue - static_cast<double>(i);

        if (i >= n - 1) return colors[n - 1];

        return interpolate(colors[i], colors[i + 1], t);
    }

}
