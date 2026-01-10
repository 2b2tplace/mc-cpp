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
                               const Registry<BiomePropertyEntry> &biomePropertyRegistry);

        [[nodiscard]]
        auto biomeFoliageColor(BiomeType biomeType, int16_t yLevel, bool isGrass) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(BiomeType biomeType) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(BiomeEnvironment biomeEnv) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(const BiomeProperties &properties) const -> const RGBA&;

    };
}