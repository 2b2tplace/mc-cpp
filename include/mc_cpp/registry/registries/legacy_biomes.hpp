#pragma once

#include <mc_cpp/registry/registries/biomes.hpp>

namespace mc {

    struct LegacyBiomeTypeEntry {
        NAMED_FIELD(uint8_t, id);
        NAMED_FIELD(std::string, name);

        DECLARE_ENTRY_BACKEND;
    };

    DECLARE_ENTRY_FROM_JSON(LegacyBiomeTypeEntry);
    DECLARE_ENTRY_TO_JSON(LegacyBiomeTypeEntry);

    using LegacyBiomeType = uint8_t;
    using LegacyBiomeTypeUpgradeMap = absl::flat_hash_map<LegacyBiomeType, BiomeType>;

    class LegacyBiomeTypeRegistry {
    public:
        absl::flat_hash_map<std::string, LegacyBiomeType> legacyBiomeByBiome;
        absl::flat_hash_map<LegacyBiomeType, std::string> biomeByLegacyBiome;

        explicit LegacyBiomeTypeRegistry(const Registry<LegacyBiomeTypeEntry> &registry);

        [[nodiscard]]
        auto populateUpgradeMap(const BiomeRegistry &biomeRegistry, LegacyBiomeTypeUpgradeMap &upgradeMap) const -> result::Result<std::monostate, std::string>;
    };

}
