#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>
#include <mc_cpp/registry/registry.hpp>
#include <mc_cpp/common/color.hpp>
#include <mc_cpp/game/biome.hpp>

namespace mc {

    struct ColorTriangleEntry {
        NAMED_FIELD(std::string, type);
        NAMED_FIELD(int32_t, topLeft);
        NAMED_FIELD(int32_t, bottomLeft);
        NAMED_FIELD(int32_t, bottomRight);

        DECLARE_ENTRY_BACKEND;
    };

    DEFINE_ENTRY_FROM_JSON(ColorTriangleEntry);
    DEFINE_ENTRY_TO_JSON(ColorTriangleEntry);

    struct BiomeColorEntry {
        NAMED_FIELD(std::string, biomeType);
        NAMED_FIELD(int32_t, color);

        DECLARE_ENTRY_BACKEND;
    };

    DEFINE_ENTRY_FROM_JSON(BiomeColorEntry);
    DEFINE_ENTRY_TO_JSON(BiomeColorEntry);

    using ColorTriangle = std::array<RGBA, 3>;
    using BiomeColors = absl::flat_hash_map<BiomeEnvironment, RGBA>;

    struct BiomeColorRegistry {
        ColorTriangle grassTriangle{};
        ColorTriangle foliageTriangle{};

        BiomeColors foliage;
        BiomeColors grass;
        BiomeColors water;

        explicit BiomeColorRegistry(const Registry<BiomeColorEntry> &foliageColors,
                                    const Registry<BiomeColorEntry> &grassColors,
                                    const Registry<BiomeColorEntry> &waterColors,
                                    const Registry<ColorTriangleEntry> &colorTriangles) {
            for (const auto&[type, topLeft, bottomLeft, bottomRight] : colorTriangles.entries) {
                auto& triangle = grassTriangle;
                if (type == "foliage")
                    triangle = foliageTriangle;

                triangle[0] = unpackARGB(topLeft);
                triangle[1] = unpackARGB(bottomLeft);
                triangle[2] = unpackARGB(bottomRight);
            }
            setBiomeColors(foliage, foliageColors);
            setBiomeColors(grass, grassColors);
            setBiomeColors(water, waterColors);
        }

        [[nodiscard]]
        auto biomeColorMultiplier(const float temperature, const float downfall, const int y, const bool isGrass) const -> RGBA {
            const float localTemperature = std::clamp(temperature - static_cast<float>(y) / 600.0f, 0.0f, 1.0f);
            const float localDownfall = std::clamp(downfall, 0.0f, 1.0f) * localTemperature;

            std::array<float, 3> triangleCoordinates{};
            triangleCoordinates[0] = localDownfall;
            triangleCoordinates[1] = localTemperature - localDownfall;
            triangleCoordinates[2] = 1.0f - localTemperature;

            std::array<float, 4> color{};

            for (size_t i = 0; i < 3; ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    const auto multiplier = (isGrass ? grassTriangle : foliageTriangle)[i][j];
                    const auto component = triangleCoordinates[i] * static_cast<float>(multiplier);
                    color[j] += std::max(0.0f, std::min(255.0f, component));
                }
            }
            return RGBA {
                static_cast<uint8_t>(color[0]),
                static_cast<uint8_t>(color[1]),
                static_cast<uint8_t>(color[2]),
                static_cast<uint8_t>(color[3]),
            };
        }

    private:
        static auto setBiomeColors(BiomeColors &colors, const Registry<BiomeColorEntry> &registry) -> void {
            colors.reserve(registry.entries.size());
            for (const auto&[biomeType, color] : registry.entries) {
                colors[getBiomeType(biomeType)] = unpackARGB(color);
            }
        }
    };
    
}
