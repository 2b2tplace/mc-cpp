#include <mc_cpp/registry/registries/biome_properties.hpp>

namespace mc {
    DEFINE_ENTRY_FROM_JSON(BiomePropertyEntry);
    DEFINE_ENTRY_TO_JSON(BiomePropertyEntry);

    BiomePropertyRegistry::BiomePropertyRegistry(const Registry<BiomePropertyEntry> &registry) {
        properties.reserve(registry.entries.size());
        biomeIdByName.reserve(registry.entries.size());
        biomeNameById.reserve(registry.entries.size());

        for (const auto&[id, name, downfall, temperature, biomeType] : registry.entries) {
            properties[id] = BiomeProperties{ getBiomeType(biomeType), temperature, downfall };
            biomeIdByName[name] = id;
            biomeNameById[id] = name;
        }
    }

    auto BiomePropertyRegistry::biomeProperties(const uint8_t id) const -> const BiomeProperties& {
        return properties.at(id);
    }

    auto BiomePropertyRegistry::biomeProperties(const std::string_view biomeName) const -> const BiomeProperties& {
        return biomeProperties(biomeType(biomeName));
    }

    auto BiomePropertyRegistry::biomeType(const std::string_view biomeName) const -> BiomeType {
        return biomeIdByName.at(biomeName);
    }

    auto BiomePropertyRegistry::biomeName(const BiomeType biomeType) const -> const std::string& {
        return biomeNameById.at(biomeType);
    }
}