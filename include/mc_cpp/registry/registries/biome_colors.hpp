#pragma once

#include <string>
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

    inline DEFINE_ENTRY_FROM_JSON(ColorTriangleEntry);
    inline DEFINE_ENTRY_TO_JSON(ColorTriangleEntry);

    struct BiomeColorEntry {
        NAMED_FIELD(std::string, biomeType);
        NAMED_FIELD(int32_t, color);

        DECLARE_ENTRY_BACKEND;
    };

    inline DEFINE_ENTRY_FROM_JSON(BiomeColorEntry);
    inline DEFINE_ENTRY_TO_JSON(BiomeColorEntry);

    using ColorTriangle = std::array<RGBA, 3>;
    using BiomeColors = std::array<RGBA, BIOME_ENVIRONMENT_COUNT>;
    using BiomeColorOverrides = std::array<bool, BIOME_ENVIRONMENT_COUNT>;

    struct BiomeColorRegistry {
        ColorTriangle grassTriangle{};
        ColorTriangle foliageTriangle{};

        BiomeColors foliage{};
        BiomeColors grass{};
        BiomeColors water{};

        BiomeColorOverrides foliageOverrides{};
        BiomeColorOverrides grassOverrides{};
        BiomeColorOverrides waterOverrides{};

        explicit BiomeColorRegistry(const Registry<BiomeColorEntry> &foliageColors,
                                    const Registry<BiomeColorEntry> &grassColors,
                                    const Registry<BiomeColorEntry> &waterColors,
                                    const Registry<ColorTriangleEntry> &colorTriangles) {
            for (const auto&[type, topLeft, bottomLeft, bottomRight] : colorTriangles.entries) {
                auto &triangle = type == "foliage" ? foliageTriangle : grassTriangle;

                triangle[0] = unpackARGB(topLeft);
                triangle[1] = unpackARGB(bottomLeft);
                triangle[2] = unpackARGB(bottomRight);
            }
            setBiomeColors(foliage, foliageOverrides, foliageColors);
            setBiomeColors(grass, grassOverrides, grassColors);
            setBiomeColors(water, waterOverrides, waterColors);
        }

        [[nodiscard]]
        auto biomeColorMultiplier(const float temperature, const float downfall, const int y, const bool isGrass) const -> RGBA {
            const float localTemperature = std::clamp(temperature - static_cast<float>(y) / 600.0f, 0.0f, 1.0f);
            const float localDownfall = std::clamp(downfall, 0.0f, 1.0f) * localTemperature;

            std::array<float, 3> triangleCoordinates{};
            triangleCoordinates[0] = localDownfall;
            triangleCoordinates[1] = localTemperature - localDownfall;
            triangleCoordinates[2] = 1.0f - localTemperature;

            std::array<float, 3> color{};

            for (size_t i = 0; i < 3; ++i) {
                for (size_t j = 0; j < 3; ++j) {
                    const auto multiplier = (isGrass ? grassTriangle : foliageTriangle)[i][j];
                    const auto component = triangleCoordinates[i] * static_cast<float>(multiplier);
                    color[j] += std::max(0.0f, std::min(255.0f, component));
                }
            }
            return RGBA {
                static_cast<uint8_t>(color[0]),
                static_cast<uint8_t>(color[1]),
                static_cast<uint8_t>(color[2]),
                0xFF,
            };
        }

    private:
        static auto setBiomeColors(BiomeColors &colors, BiomeColorOverrides &overrides, const Registry<BiomeColorEntry> &registry) -> void {
            RGBA defaultColor;

            for (size_t biomeEnv = 0; biomeEnv < BIOME_ENVIRONMENT_COUNT; biomeEnv++) {
                RGBA color = defaultColor;
                bool useOverride{};
                for (const auto&[biomeTypeStr, colorPacked] : registry.entries) {
                    if (const auto biomeEnvEntry = getBiomeType(biomeTypeStr); static_cast<size_t>(biomeEnvEntry) == biomeEnv) {
                        color = unpackARGB(colorPacked);
                        useOverride = true;
                        break;
                    }
                }
                if (biomeEnv == 0) defaultColor = color;
                colors[biomeEnv] = color;
                overrides[biomeEnv] = useOverride;
            }
        }
    };
    
}
