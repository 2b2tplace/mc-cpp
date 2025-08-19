#pragma once

#include <mc_cpp/registry/registries/biome_colors.hpp>
#include <mc_cpp/registry/registries/biome_properties.hpp>
#include <mc_cpp/game/constants.hpp>

namespace mc {

    class BiomeRegistry {

        std::array<RGBA, UINT16_MAX> biomeFoliageColors{};
        std::array<RGBA, UINT16_MAX> biomeGrassColors{};

        std::array<bool, UINT8_MAX> biomeFoliageOverrides{};
        std::array<bool, UINT8_MAX> biomeGrassOverrides{};

    public:
        BiomeColorRegistry biomeColors;
        BiomePropertyRegistry biomeProperties;

        explicit BiomeRegistry(const Registry<BiomeColorEntry> &foliageColors,
                               const Registry<BiomeColorEntry> &grassColors,
                               const Registry<BiomeColorEntry> &waterColors,
                               const Registry<ColorTriangleEntry> &colorTriangles,
                               const Registry<BiomePropertyEntry> &biomePropertyRegistry):
            biomeColors(BiomeColorRegistry {foliageColors, grassColors, waterColors, colorTriangles }),
            biomeProperties(BiomePropertyRegistry { biomePropertyRegistry }) {

            for (const auto&[id, properties] : biomeProperties.properties) {
                const auto environment = properties.environment;
                const auto env = static_cast<size_t>(environment);
                if (biomeColors.grassOverrides[env])
                    biomeGrassColors[id] = biomeColors.grass.at(env);

                if (biomeColors.foliageOverrides[env])
                    biomeFoliageColors[id] = biomeColors.foliage.at(env);

                for (int16_t y = 0; y < UINT8_MAX; y++) {
                    biomeGrassColors  [id + UINT8_MAX * y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, true);
                    biomeFoliageColors[id + UINT8_MAX * y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, false);
                }
            }
        }

        [[nodiscard]]
        auto biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const bool isGrass) const -> const RGBA& {
            if (isGrass ? biomeGrassOverrides[biomeType] : biomeFoliageOverrides[biomeType])
                // biome overrides only store y = 0 as they don't depend on y
                return isGrass ? biomeGrassColors  [biomeType] : biomeFoliageColors[biomeType];

            const auto y = std::clamp(yLevel - SEA_LEVEL, 0, UINT8_MAX);
            return isGrass ? biomeGrassColors  [biomeType + UINT8_MAX * y]
                           : biomeFoliageColors[biomeType + UINT8_MAX * y];
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeType biomeType) const -> const RGBA& {
            return biomeWaterColor(biomeProperties.biomeProperties(biomeType).environment);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeEnvironment biomeEnv) const -> const RGBA& {
            return biomeColors.water[static_cast<size_t>(biomeEnv)];
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeProperties &properties) const -> const RGBA& {
            return biomeWaterColor(properties.environment);
        }

    };

}