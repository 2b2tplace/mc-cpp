#include <mc_cpp/registry/registries/biomes.hpp>

namespace mc {
    BiomeRegistry::BiomeRegistry(const Registry<BiomeColorEntry> &foliageColors,
                           const Registry<BiomeColorEntry> &grassColors,
                           const Registry<BiomeColorEntry> &waterColors,
                           const Registry<ColorTriangleEntry> &colorTriangles,
                           const Registry<BiomePropertyEntry> &biomePropertyRegistry):
        biomeColors(BiomeColorRegistry {foliageColors, grassColors, waterColors, colorTriangles }),
        biomeProperties(BiomePropertyRegistry { biomePropertyRegistry }) {

        for (const auto&[id, properties] : biomeProperties.properties) {
            const auto environment = properties.environment;
            const auto env = static_cast<size_t>(environment);

            for (int16_t y = 0; y < UINT8_MAX; y++) {
                biomeGrassColors  [id + UINT8_MAX * y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, true);
                biomeFoliageColors[id + UINT8_MAX * y] = biomeColors.biomeColorMultiplier(properties.temperature, properties.downfall, y, false);
            }
            if (biomeColors.grassOverrides[env]) {
                biomeGrassOverrides[id] = true;
                biomeGrassColors[id] = biomeColors.grass.at(env);
            }
            if (biomeColors.foliageOverrides[env]) {
                biomeFoliageOverrides[id] = true;
                biomeFoliageColors[id] = biomeColors.foliage.at(env);
            }
        }
    }

    auto BiomeRegistry::biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const bool isGrass) const -> const RGBA& {
        if (isGrass ? biomeGrassOverrides[biomeType] : biomeFoliageOverrides[biomeType])
            // biome overrides only store y = 0 as they don't depend on y
                return isGrass ? biomeGrassColors[biomeType] : biomeFoliageColors[biomeType];

        const auto y = std::clamp(yLevel - SEA_LEVEL, 0, UINT8_MAX);
        return isGrass ? biomeGrassColors  [biomeType + UINT8_MAX * y]
                       : biomeFoliageColors[biomeType + UINT8_MAX * y];
    }

    auto BiomeRegistry::biomeWaterColor(const BiomeType biomeType) const -> const RGBA& {
        return biomeWaterColor(biomeProperties.biomeProperties(biomeType).environment);
    }

    auto BiomeRegistry::biomeWaterColor(const BiomeEnvironment biomeEnv) const -> const RGBA& {
        const auto biomeIndex = static_cast<size_t>(biomeEnv);
        if (biomeIndex < 0 || biomeIndex >= biomeColors.water.size())
            return WHITE;

        return biomeColors.water[biomeIndex];
    }

    auto BiomeRegistry::biomeWaterColor(const BiomeProperties &properties) const -> const RGBA& {
        return biomeWaterColor(properties.environment);
    }
}