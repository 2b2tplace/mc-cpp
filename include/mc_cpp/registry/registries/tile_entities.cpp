#include <mc_cpp/registry/registries/tile_entities.hpp>

namespace mc {
    DEFINE_ENTRY_FROM_JSON(TileEntityEntry);
    DEFINE_ENTRY_TO_JSON(TileEntityEntry);

    TileEntityRegistry::TileEntityRegistry(const Registry<TileEntityEntry> &registry) {
        tileEntitiesById.reserve(registry.entries.size());
        tileEntityIdsByName.reserve(registry.entries.size());

        for (const auto &entry : registry.entries) {
            tileEntitiesById[entry.id] = entry;
            tileEntityIdsByName[entry.name] = entry.id;
        }
    }

    auto TileEntityRegistry::tileEntity(const uint16_t tileEntityId) const -> const TileEntityEntry& {
        return tileEntitiesById.at(tileEntityId);
    }

    auto TileEntityRegistry::tileEntity(const std::string_view tileEntityName) const -> const TileEntityEntry& {
        return tileEntity(tileEntityId(tileEntityName));
    }

    auto TileEntityRegistry::tileEntityId(const std::string_view tileEntityName) const -> uint16_t {
        return tileEntityIdsByName.at(tileEntityName);
    }

    auto TileEntityRegistry::tileEntityName(const uint16_t tileEntityId) const -> const std::string& {
        return tileEntity(tileEntityId).name;
    }
}