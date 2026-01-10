#include <mc_cpp/registry/registries/dimensions.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace mc {

    auto getDimensionRegionDirectory(const DimensionType type) -> const fs::path& {
        return DIMENSION_REGION_DIRECTORIES[type];
    }

    auto getDimensionType(const std::string_view name) -> DimensionType {
        auto nameStripped = name;
        stripMinecraftNamespace(&nameStripped);
        for (size_t i = 0; i < DIMENSION_NAMES.size(); i++) {
            if (DIMENSION_NAMES[i] == nameStripped)
                return static_cast<DimensionType>(i);
        }
        throw std::runtime_error("Unknown dimension '" + std::string{name} + "'");
    }

    auto getNamespacedDimension(const DimensionType type) -> std::string {
        auto dimensionName = std::string{getDimensionName(type)};
        prependMinecraftNamespace(&dimensionName);
        return dimensionName;
    }

}
