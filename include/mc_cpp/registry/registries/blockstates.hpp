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

    DEFINE_ENTRY_FROM_JSON(BlockStateEntry);
    DEFINE_ENTRY_TO_JSON(BlockStateEntry);

    class BlockRegistry {
        absl::flat_hash_map<BlockState, BlockStateProperties> properties;
        absl::flat_hash_map<std::string, BlockState> blockStateByName;

        absl::flat_hash_map<std::string, size_t> blockTypeByName;
        absl::flat_hash_map<BlockState, size_t> blockTypeByState;

        std::vector<BlockType> blockTypes;
        size_t currentBlockTypeId{};

    public:
        explicit BlockRegistry(const Registry<BlockStateEntry> &registry) {
            properties.reserve(registry.entries.size());
            blockStateByName.reserve(registry.entries.size());
            for (const auto&[id, name, color, lightLevel] : registry.entries) {
                const auto rgba = unpackARGB(color);
                auto illuminatedColor = rgba;

                if (lightLevel > 0) {
                    const auto levelInv = static_cast<float>(lightLevel) - MAX_LIGHT_LEVEL;
                    const auto brightnessMultiplier = sqrtf(MAX_LIGHT_LEVEL_SQ - levelInv * levelInv) / LUMINANCE_DIV;

                    auto hsv = toHSVA(rgba);
                    hsv[2] = std::clamp(hsv[2] * brightnessMultiplier, 0.0f, 1.0f);
                    illuminatedColor = toRGBA(hsv);
                }
                properties[id] = BlockStateProperties { id, name, rgba, colorBrightness(rgba), lightLevel, illuminatedColor };
                blockStateByName[name] = id;
                extendBlockType(id, name);
            }
        }

        [[nodiscard]]
        auto blockStateProperties(const BlockState state) const -> const BlockStateProperties& {
            if (!properties.contains(state))
                return MISSING_BLOCK_PROPERTIES;

            return properties.at(state);
        }

        [[nodiscard]]
        auto blockState(const std::string_view name) const -> BlockState {
            if (!blockStateByName.contains(name))
                return MISSING_BLOCK_STATE;

            return blockStateByName.at(name);
        }

        [[nodiscard]]
        auto blockStateProperties(const std::string_view name) const -> const BlockStateProperties& {
            return blockStateProperties(blockState(name));
        }

        [[nodiscard]]
        auto blockType(const std::string_view name) const -> const BlockType& {
            if (!blockTypeByName.contains(name))
                return MISSING_BLOCK_TYPE;

            return blockTypes.at(blockTypeByName.at(name));
        }

        [[nodiscard]]
        auto blockType(const BlockState state) const -> const BlockType& {
            if (!blockTypeByState.contains(state))
                return MISSING_BLOCK_TYPE;

            return blockTypes.at(blockTypeByState.at(state));
        }

    private:
        auto extendBlockType(const BlockState additionalState, const std::string &stateName) -> void {
            const auto pos = stateName.find('[');
            const auto blockName = pos != std::string::npos ? stateName.substr(0, pos) : stateName;

            if (!blockTypeByName.contains(blockName)) {
                blockTypeByName[blockName] = currentBlockTypeId;
                blockTypeByState[additionalState] = currentBlockTypeId;
                blockTypes.emplace_back(additionalState, additionalState);
                currentBlockTypeId++;
                return;
            }
            auto&[min, max] = blockTypes[blockTypeByName.at(blockName)];

            if (additionalState < min) min = additionalState;
            if (additionalState > max) max = additionalState;
        }
    };
    
}
