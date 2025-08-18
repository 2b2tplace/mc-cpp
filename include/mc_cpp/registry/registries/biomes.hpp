#pragma once

#include <mc_cpp/registry/registries/biome_colors.hpp>
#include <mc_cpp/registry/registries/biome_properties.hpp>
#include <mc_cpp/game/constants.hpp>

namespace mc {

    class BiomeRegistry {

        absl::flat_hash_map<BiomeType, absl::flat_hash_map<int16_t, RGBA>> biomeFoliageColors;
        absl::flat_hash_map<BiomeType, absl::flat_hash_map<int16_t, RGBA>> biomeGrassColors;

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
                const auto env = properties.environment;
                if (biomeColors.grass.contains(env))
                    biomeGrassColors[id][0] = biomeColors.grass.at(env);

                if (biomeColors.foliage.contains(env))
                    biomeFoliageColors[id][0] = biomeColors.foliage.at(env);

                for (int16_t y = 0; y < MAX_Y_LEVEL - SEA_LEVEL; y++) {
                    biomeGrassColors  [id][y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, true);
                    biomeFoliageColors[id][y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, false);
                }
            }
        }

        [[nodiscard]]
        auto biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const bool isGrass) const -> const RGBA& {
            const auto &cache = isGrass ? biomeGrassColors.at(biomeType) : biomeFoliageColors.at(biomeType);
            if (cache.size() == 1) return cache.at(0); // biome overrides only store y = 0 as they don't depend on y

            const auto y = std::clamp(yLevel - SEA_LEVEL, 0, MAX_Y_LEVEL - SEA_LEVEL - 1);
            return cache.at(static_cast<int16_t>(y));
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeType biomeType) const -> const RGBA& {
            return biomeWaterColor(biomeProperties.biomeProperties(biomeType).environment);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeEnvironment biomeEnv) const -> const RGBA& {
            return biomeColors.water.contains(biomeEnv)
                 ? biomeColors.water.at(biomeEnv)
                 : biomeColors.water.at(BiomeEnvironment::DEFAULT);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeProperties &properties) const -> const RGBA& {
            return biomeWaterColor(properties.environment);
        }

    };

}