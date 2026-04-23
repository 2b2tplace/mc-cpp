#pragma once

#include <mc_cpp/registry/registries/blockstates.hpp>

namespace mc {

    struct LegacyBlockStateEntry {
        NAMED_FIELD(uint8_t, id);
        NAMED_FIELD(uint8_t, data);
        NAMED_FIELD(std::string, blockstate);

        DECLARE_ENTRY_BACKEND;
    };

    DECLARE_ENTRY_FROM_JSON(LegacyBlockStateEntry);
    DECLARE_ENTRY_TO_JSON(LegacyBlockStateEntry);

    using LegacyBlockState = uint16_t;

    inline auto toLegacy(const uint8_t id, const uint8_t data) -> LegacyBlockState {
        return static_cast<uint16_t>(data) << 8 | static_cast<uint16_t>(id);
    }

    using LegacyBlockUpgradeMap = absl::flat_hash_map<LegacyBlockState, BlockState>;

    class LegacyBlockStateRegistry {
    public:
        absl::flat_hash_map<std::string, LegacyBlockState> legacyBlockByState;
        absl::flat_hash_map<LegacyBlockState, std::string> stateByLegacyBlock;

        explicit LegacyBlockStateRegistry(const Registry<LegacyBlockStateEntry> &registry);

        [[nodiscard]]
        auto populateUpgradeMap(const BlockRegistry &blockRegistry, LegacyBlockUpgradeMap &upgradeMap) const -> result::Result<std::monostate, std::string>;
    };

}
