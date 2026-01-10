#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <mc_cpp/registry/registry.hpp>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>
#include <mc_cpp/game/block.hpp>
#include <mc_cpp/game/constants.hpp>

namespace mc {

    struct BlockStateEntry {
        NAMED_FIELD(uint16_t, id);
        NAMED_FIELD(std::string, name);
        NAMED_FIELD(int32_t, color);
        NAMED_FIELD(uint8_t, lightLevel);

        DECLARE_ENTRY_BACKEND;
    };

    DECLARE_ENTRY_FROM_JSON(BlockStateEntry);
    DECLARE_ENTRY_TO_JSON(BlockStateEntry);

    using BlockStatePropertyMap = absl::flat_hash_map<std::string, std::string>;

    class BlockRegistry {
    public:
        absl::flat_hash_map<BlockState, BlockStateRenderProperties> renderProperties;
        absl::flat_hash_map<std::string, BlockState> blockStateByName;
        absl::flat_hash_map<BlockState, std::string> blockNameByState;
        absl::flat_hash_map<BlockState, BlockStatePropertyMap> blockStatePropertyMaps;

        absl::flat_hash_map<std::string, size_t> blockTypeByName;
        absl::flat_hash_map<BlockState, size_t> blockTypeByState;

        std::vector<BlockType> blockTypes;
        size_t currentBlockTypeId{};

        explicit BlockRegistry(const Registry<BlockStateEntry> &registry);

        [[nodiscard]]
        auto blockStateRenderProperties(BlockState state) const -> const BlockStateRenderProperties&;

        [[nodiscard]]
        auto blockStatePropertyMap(BlockState state) const -> const BlockStatePropertyMap&;

        [[nodiscard]]
        auto blockState(std::string_view name) const -> BlockState;

        [[nodiscard]]
        auto blockStateRenderProperties(std::string_view name) const -> const BlockStateRenderProperties&;

        [[nodiscard]]
        auto blockName(const BlockType &type) const -> const std::string&;

        [[nodiscard]]
        auto blockName(BlockState state) const -> const std::string&;

        [[nodiscard]]
        auto blockType(std::string_view name) const -> const BlockType&;

        [[nodiscard]]
        auto blockType(BlockState state) const -> const BlockType&;

    private:
        auto extendBlockType(BlockState additionalState, const std::string &stateName) -> void;
    };
    
}
