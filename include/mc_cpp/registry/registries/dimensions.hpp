#pragma once

#include <array>
#include <string_view>
#include <filesystem>

namespace mc {

    namespace fs = std::filesystem;

    enum DimensionType {
        OVERWORLD = 0,
        NETHER = 1,
        THE_END = 2,
    };

    static constexpr std::array<std::string_view, 3> DIMENSION_NAMES {
        "overworld",
        "the_nether",
        "the_end"
    };

    static const std::array<fs::path, 3> DIMENSION_REGION_DIRECTORIES {
        "region",
        fs::path("DIM-1") / "region",
        fs::path("DIM1") / "region",
    };

    struct DimensionProperties {
        bool hasSkyLight;
        int32_t minY;
        uint32_t height;
    };

    static constexpr std::array DIMENSION_PROPERTIES = {
        DimensionProperties {true, -64, 384},
        DimensionProperties {false, 0, 256},
        DimensionProperties {false, 0, 256}
    };

    [[nodiscard]]
    inline auto getDimensionRegionDirectory(const DimensionType type) -> const fs::path& {
        return DIMENSION_REGION_DIRECTORIES[type];
    }

    [[nodiscard]]
    constexpr auto getDimensionProperties(const DimensionType type) -> const DimensionProperties& {
        return DIMENSION_PROPERTIES[type];
    }

    [[nodiscard]]
    constexpr auto getDimensionName(const DimensionType type) -> std::string_view {
        return DIMENSION_NAMES[type];
    }

    [[nodiscard]]
    inline auto getNamespacedDimension(const DimensionType type) -> std::string {
        auto dimensionName = std::string{getDimensionName(type)};
        prependMinecraftNamespace(&dimensionName);
        return dimensionName;
    }

}
