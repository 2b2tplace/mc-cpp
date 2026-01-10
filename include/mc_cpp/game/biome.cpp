#include <mc_cpp/game/biome.hpp>

namespace mc {
    auto getBiomeType(const std::string &biomeTypeName) -> BiomeEnvironment {
        auto biomeType = BiomeEnvironment::DEFAULT;
        for (size_t i = 0; i < BIOME_ENVIRONMENT_COUNT; i++) {
            if (BiomeTypeNames.at(i) == biomeTypeName) {
                biomeType = static_cast<BiomeEnvironment>(i);
                break;
            }
        }
        return biomeType;
    }
}
