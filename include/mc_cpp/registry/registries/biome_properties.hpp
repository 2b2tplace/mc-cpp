#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>
#include <mc_cpp/game/biome.hpp>
#include <mc_cpp/registry/registry.hpp>

namespace mc {
    struct BiomePropertyEntry {
        NAMED_FIELD(uint8_t, id);
        NAMED_FIELD(std::string, name);
        NAMED_FIELD(float, downfall);
        NAMED_FIELD(float, temperature);
        NAMED_FIELD_OPTIONAL(std::string, biomeType, "Default");

        DECLARE_ENTRY_BACKEND;
    };

    DECLARE_ENTRY_FROM_JSON(BiomePropertyEntry);
    DECLARE_ENTRY_TO_JSON(BiomePropertyEntry);

    struct BiomePropertyRegistry {
        absl::flat_hash_map<BiomeType, BiomeProperties> properties;
        absl::flat_hash_map<std::string, BiomeType> biomeIdByName;
        absl::flat_hash_map<BiomeType, std::string> biomeNameById;

        explicit BiomePropertyRegistry(const Registry<BiomePropertyEntry> &registry);

        [[nodiscard]]
        auto biomeProperties(uint8_t id) const -> const BiomeProperties&;

        [[nodiscard]]
        auto biomeProperties(std::string_view biomeName) const -> const BiomeProperties&;

        [[nodiscard]]
        auto biomeType(std::string_view biomeName) const -> BiomeType;

        [[nodiscard]]
        auto biomeName(BiomeType biomeType) const -> const std::string&;

    };
}