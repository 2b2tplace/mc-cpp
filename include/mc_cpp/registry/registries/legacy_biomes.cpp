#include <mc_cpp/registry/registries/legacy_biomes.hpp>
#include <fmt/format.h>

namespace mc {

    DEFINE_ENTRY_FROM_JSON(LegacyBiomeTypeEntry);
    DEFINE_ENTRY_TO_JSON(LegacyBiomeTypeEntry);

    LegacyBiomeTypeRegistry::LegacyBiomeTypeRegistry(const Registry<LegacyBiomeTypeEntry> &registry) {
        biomeByLegacyBiome.reserve(registry.entries.size());
        legacyBiomeByBiome.reserve(registry.entries.size());
        for (const auto &[id, name] : registry.entries) {
            biomeByLegacyBiome[id] = name;
            legacyBiomeByBiome[name] = id;
        }
    }

    auto LegacyBiomeTypeRegistry::populateUpgradeMap(const BiomeRegistry &biomeRegistry, LegacyBiomeTypeUpgradeMap &upgradeMap) const -> result::Result<std::monostate, std::string> {
        for (const auto &[legacyId, biomeName] : biomeByLegacyBiome) {
            const auto biomeType = biomeRegistry.biomeProperties.biomeType(biomeName);
            if (biomeType == MISSING_BIOME_TYPE)
                return ERR(fmt::format("Unable to find legacy upgrade translation for biome type: {}", biomeName));

            upgradeMap[legacyId] = biomeType;
        }
        return {};
    }

}