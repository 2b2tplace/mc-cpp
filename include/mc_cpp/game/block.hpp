#pragma once

#include <cstdint>
#include <ostream>
#include <mc_cpp/common/color.hpp>

namespace mc {

    using BlockState = uint16_t;

    struct BlockType {
        BlockState min;
        BlockState max;

        friend std::ostream& operator<<(std::ostream &os, const BlockType &type);
    };

    struct BlockStateRenderProperties {
        BlockState id;
        std::string name;
        RGBA color;
        float textureBrightness;
        uint8_t lightLevel;
        RGBA illuminatedColor;
    };

    static constexpr auto MISSING_BLOCK_STATE = UINT16_MAX;
    static constexpr auto MISSING_BLOCK_TYPE = BlockType{MISSING_BLOCK_STATE, MISSING_BLOCK_STATE};
    static const std::string MISSING_BLOCK_NAME = ":unknown";

    static const absl::flat_hash_map<std::string, std::string> MISSING_BLOCK_PROPERTY_MAP = {};

    static const auto MISSING_BLOCK_RENDER_PROPERTIES = BlockStateRenderProperties {
        MISSING_BLOCK_STATE,
        "unknown",
        {},
        0.0f,
        0,
        {}
    };

    inline auto operator==(const BlockState state, const BlockType &type) -> bool {
        return state >= type.min && state <= type.max;
    }

    inline auto operator<<(std::ostream &os, const BlockType &type) -> std::ostream & {
        os << '{' << type.min << ", " << type.max << '}';
        return os;
    }

}
