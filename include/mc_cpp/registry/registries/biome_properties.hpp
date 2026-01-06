#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>
#include <mc_cpp/game/biome.hpp>

namespace mc {

    struct BiomePropertyEntry {
        NAMED_FIELD(uint8_t, id);
        NAMED_FIELD(std::string, name);
        NAMED_FIELD(float, downfall);
        NAMED_FIELD(float, temperature);
        NAMED_FIELD_OPTIONAL(std::string, biomeType, "Default");

        DECLARE_ENTRY_BACKEND;
    };

    inline DEFINE_ENTRY_FROM_JSON(BiomePropertyEntry);
    inline DEFINE_ENTRY_TO_JSON(BiomePropertyEntry);

    struct BiomePropertyRegistry {
        absl::flat_hash_map<BiomeType, BiomeProperties> properties;
        absl::flat_hash_map<std::string, BiomeType> biomeIdByName;
        absl::flat_hash_map<BiomeType, std::string> biomeNameById;

        explicit BiomePropertyRegistry(const Registry<BiomePropertyEntry> &registry) {
            properties.reserve(registry.entries.size());
            biomeIdByName.reserve(registry.entries.size());
            biomeNameById.reserve(registry.entries.size());

            for (const auto&[id, name, downfall, temperature, biomeType] : registry.entries) {
                properties[id] = BiomeProperties{ getBiomeType(biomeType), temperature, downfall };
                biomeIdByName[name] = id;
                biomeNameById[id] = name;
            }
        }

        [[nodiscard]]
        auto biomeProperties(const uint8_t id) const -> const BiomeProperties& {
            return properties.at(id);
        }

        [[nodiscard]]
        auto biomeProperties(const std::string_view biomeName) const -> const BiomeProperties& {
            return biomeProperties(biomeType(biomeName));
        }

        [[nodiscard]]
        auto biomeType(const std::string_view biomeName) const -> BiomeType {
            return biomeIdByName.at(biomeName);
        }

        [[nodiscard]]
        auto biomeName(const BiomeType biomeType) const -> const std::string& {
            return biomeNameById.at(biomeType);
        }

    };
    
}