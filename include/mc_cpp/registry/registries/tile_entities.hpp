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

    inline DEFINE_ENTRY_FROM_JSON(TileEntityEntry);
    inline DEFINE_ENTRY_TO_JSON(TileEntityEntry);

    struct TileEntityRegistry {
        absl::flat_hash_map<uint16_t, TileEntityEntry> tileEntitiesById;
        absl::flat_hash_map<std::string, uint16_t> tileEntityIdsByName;

        explicit TileEntityRegistry(const Registry<TileEntityEntry> &registry) {
            tileEntitiesById.reserve(registry.entries.size());
            tileEntityIdsByName.reserve(registry.entries.size());

            for (const auto &entry : registry.entries) {
                tileEntitiesById[entry.id] = entry;
                tileEntityIdsByName[entry.name] = entry.id;
            }
        }

        [[nodiscard]]
        auto tileEntity(const uint16_t tileEntityId) const -> const TileEntityEntry& {
            return tileEntitiesById.at(tileEntityId);
        }

        [[nodiscard]]
        auto tileEntity(const std::string_view tileEntityName) const -> const TileEntityEntry& {
            return tileEntity(tileEntityId(tileEntityName));
        }

        [[nodiscard]]
        auto tileEntityId(const std::string_view tileEntityName) const -> uint16_t {
            return tileEntityIdsByName.at(tileEntityName);
        }

        [[nodiscard]]
        auto tileEntityName(const uint16_t tileEntityId) const -> const std::string& {
            return tileEntity(tileEntityId).name;
        }
    };

}
