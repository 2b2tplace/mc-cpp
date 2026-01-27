#pragma once

#include <string>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>
#include <mc_cpp/registry/registry.hpp>
#include <absl/container/flat_hash_map.h>

namespace mc {
    struct TileEntityEntry {
        NAMED_FIELD(uint16_t, id);
        NAMED_FIELD(std::string, name);

        DECLARE_ENTRY_BACKEND;
    };

    static constexpr uint16_t MISSING_TILE_ENTITY_TYPE = UINT16_MAX;
    static const std::string MISSING_TILE_ENTITY_NAME = ":unknown";
    static const auto MISSING_TILE_ENTITY = TileEntityEntry {.id = MISSING_TILE_ENTITY_TYPE, .name = MISSING_TILE_ENTITY_NAME};

    DECLARE_ENTRY_FROM_JSON(TileEntityEntry);
    DECLARE_ENTRY_TO_JSON(TileEntityEntry);

    struct TileEntityRegistry {
        absl::flat_hash_map<uint16_t, TileEntityEntry> tileEntitiesById;
        absl::flat_hash_map<std::string, uint16_t> tileEntityIdsByName;

        explicit TileEntityRegistry(const Registry<TileEntityEntry> &registry);

        [[nodiscard]]
        auto tileEntity(uint16_t tileEntityId) const -> const TileEntityEntry&;

        [[nodiscard]]
        auto tileEntity(std::string_view tileEntityName) const -> const TileEntityEntry&;

        [[nodiscard]]
        auto tileEntityId(std::string_view tileEntityName) const -> uint16_t;

        [[nodiscard]]
        auto tileEntityName(uint16_t tileEntityId) const -> const std::string&;
    };
}
