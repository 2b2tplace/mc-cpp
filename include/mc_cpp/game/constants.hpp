#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

namespace mc {

    static constexpr int16_t SEA_LEVEL = 62;
    static constexpr int16_t MAX_Y_LEVEL = 320;

    static const auto FOLIAGE_BLOCK_TYPES = std::vector<std::string_view> {
        "short_grass",
        "tall_grass",
        "fern",
        "large_fern",
        "sugar_cane",
        "vine",
        "lily_pad",
        "oak_leaves",
        "spruce_leaves",
        "birch_leaves",
        "jungle_leaves",
        "acacia_leaves",
        "dark_oak_leaves",
        "mangrove_leaves"
    };

    static constexpr auto MAX_LIGHT_LEVEL = 15.0f;
    static constexpr auto MAX_LIGHT_LEVEL_SQ = MAX_LIGHT_LEVEL * MAX_LIGHT_LEVEL;
    static constexpr auto LUMINANCE_DIV = MAX_LIGHT_LEVEL / 5;

}