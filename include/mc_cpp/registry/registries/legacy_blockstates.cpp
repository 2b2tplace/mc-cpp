#include <mc_cpp/registry/registries/legacy_blockstates.hpp>
#include <fmt/format.h>

namespace mc {
    DEFINE_ENTRY_FROM_JSON(LegacyBlockStateEntry);
    DEFINE_ENTRY_TO_JSON(LegacyBlockStateEntry);

    LegacyBlockStateRegistry::LegacyBlockStateRegistry(const Registry<LegacyBlockStateEntry> &registry) {
        stateByLegacyBlock.reserve(registry.entries.size());
        legacyBlockByState.reserve(registry.entries.size());
        for (const auto &[id, data, blockstate] : registry.entries) {
            const auto legacyBlock = toLegacy(id, data);
            stateByLegacyBlock[legacyBlock] = blockstate;
            legacyBlockByState[blockstate] = legacyBlock;
        }
    }

    auto LegacyBlockStateRegistry::populateUpgradeMap(const BlockRegistry &blockRegistry, LegacyBlockUpgradeMap &upgradeMap) const -> result::Result<std::monostate, std::string> {
        for (const auto &[legacyBlock, blockstate] : stateByLegacyBlock) {
            const auto [blockName, properties] = parseBlockStateProperties(blockstate);
            const auto [min, max] = blockRegistry.blockType(blockName);
            if (min == MISSING_BLOCK_STATE || max == MISSING_BLOCK_STATE)
                return ERR(fmt::format("Unable to find legacy upgrade translation for block type: {}", blockName));

            BlockState foundState = MISSING_BLOCK_STATE;
            for (BlockState state = min; state <= max; state++) {
                if (properties == blockRegistry.blockStatePropertyMap(state)) {
                    foundState = state;
                    break;
                }
            }
            if (foundState == MISSING_BLOCK_STATE)
                return ERR(fmt::format("Unable to find legacy upgrade translation for blockstate: {}", blockstate));

            upgradeMap[legacyBlock] = foundState;
        }
        return {};
    }

}
