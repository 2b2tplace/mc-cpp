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

    inline DEFINE_ENTRY_FROM_JSON(BlockStateEntry);
    inline DEFINE_ENTRY_TO_JSON(BlockStateEntry);

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

        explicit BlockRegistry(const Registry<BlockStateEntry> &registry) {
            renderProperties.reserve(registry.entries.size());
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
                renderProperties[id] = BlockStateRenderProperties { id, name, rgba, colorBrightness(rgba), lightLevel, illuminatedColor };
                blockStateByName[name] = id;
                extendBlockType(id, name);
            }
        }

        [[nodiscard]]
        auto blockStateRenderProperties(const BlockState state) const -> const BlockStateRenderProperties& {
            if (!renderProperties.contains(state))
                return MISSING_BLOCK_RENDER_PROPERTIES;

            return renderProperties.at(state);
        }

        [[nodiscard]]
        auto blockStatePropertyMap(const BlockState state) const -> const BlockStatePropertyMap& {
            if (!blockStatePropertyMaps.contains(state))
                return MISSING_BLOCK_PROPERTY_MAP;

            return blockStatePropertyMaps.at(state);
        }

        [[nodiscard]]
        auto blockState(const std::string_view name) const -> BlockState {
            if (!blockStateByName.contains(name))
                return MISSING_BLOCK_STATE;

            return blockStateByName.at(name);
        }

        [[nodiscard]]
        auto blockStateRenderProperties(const std::string_view name) const -> const BlockStateRenderProperties& {
            return blockStateRenderProperties(blockState(name));
        }

        [[nodiscard]]
        auto blockName(const BlockType &type) const -> const std::string& {
            if (!blockNameByState.contains(type.min))
                return MISSING_BLOCK_NAME;

            return blockNameByState.at(type.min);
        }

        [[nodiscard]]
        auto blockName(const BlockState state) const -> const std::string& {
            if (!blockNameByState.contains(state))
                return MISSING_BLOCK_NAME;

            return blockNameByState.at(state);
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
            const auto open = stateName.find('[');
            const auto close = stateName.find(']');
            const auto blockName = open == std::string::npos ? stateName : stateName.substr(0, open);
            const auto blockPropertiesStr = close == std::string_view::npos || close < open ? "" : stateName.substr(open + 1, close - open - 1);

            absl::flat_hash_map<std::string, std::string> properties;
            if (!blockPropertiesStr.empty() && blockPropertiesStr != "-") {
                std::istringstream ss(blockPropertiesStr);
                for (std::string token; std::getline(ss, token, ',');) {
                    if (token.empty()) continue;

                    const auto index = token.find('=');
                    if (index == std::string::npos) continue;

                    const auto key = token.substr(0, index);
                    const auto value = token.substr(index + 1);
                    properties[key] = value;
                }
            }
            blockStatePropertyMaps[additionalState] = properties;
            blockNameByState[additionalState] = blockName;

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
