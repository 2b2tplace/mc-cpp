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

    DECLARE_ENTRY_FROM_JSON(ColorTriangleEntry);
    DECLARE_ENTRY_TO_JSON(ColorTriangleEntry);

    struct BiomeColorEntry {
        NAMED_FIELD(std::string, biomeType);
        NAMED_FIELD(int32_t, color);

        DECLARE_ENTRY_BACKEND;
    };

    DECLARE_ENTRY_FROM_JSON(BiomeColorEntry);
    DECLARE_ENTRY_TO_JSON(BiomeColorEntry);

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
                                    const Registry<ColorTriangleEntry> &colorTriangles);

        [[nodiscard]]
        auto biomeColorMultiplier(float temperature, float downfall, int y, bool isGrass) const -> RGBA;

    private:
        static auto setBiomeColors(BiomeColors &colors, BiomeColorOverrides &overrides, const Registry<BiomeColorEntry> &registry) -> void;
    };
    
}
