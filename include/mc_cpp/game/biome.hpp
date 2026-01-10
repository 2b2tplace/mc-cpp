#pragma once

#include <array>
#include <string>

namespace mc {

    using BiomeType = uint8_t;

    enum class BiomeEnvironment {
        DEFAULT = 0,
        SWAMP,
        DARK_FOREST,
        BADLANDS,
        WARM_OCEAN,
        LUKEWARM_OCEAN,
        COLD_OCEAN,
        FROZEN_OCEAN,
        PALE_GARDEN,
        MEADOW,
        MANGROVE_SWAMP,
        CHERRY_GROVE,
        COUNT_
    };

    static constexpr auto BIOME_ENVIRONMENT_COUNT = static_cast<size_t>(BiomeEnvironment::COUNT_);

    inline const std::array<std::string, BIOME_ENVIRONMENT_COUNT> BiomeTypeNames = {
        "Default",
        "Swamp",
        "DarkForest",
        "Badlands",
        "WarmOcean",
        "LukewarmOcean",
        "ColdOcean",
        "FrozenOcean",
        "PaleGarden",
        "Meadow",
        "MangroveSwamp",
        "CherryGrove"
    };

    [[nodiscard]]
    auto getBiomeType(const std::string &biomeTypeName) -> BiomeEnvironment;

    struct BiomeProperties {
        BiomeEnvironment environment{};
        float temperature{};
        float downfall{};
    };

}
