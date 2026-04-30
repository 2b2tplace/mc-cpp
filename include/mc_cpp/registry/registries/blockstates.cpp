#include <mc_cpp/registry/registries/blockstates.hpp>

namespace mc {
    DEFINE_ENTRY_FROM_JSON(BlockStateEntry);
    DEFINE_ENTRY_TO_JSON(BlockStateEntry);

    BlockRegistry::BlockRegistry(const Registry<BlockStateEntry> &registry) {
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
            blockStateNameByState[id] = name;
            extendBlockType(id, name);
        }
    }

    auto BlockRegistry::blockStateRenderProperties(const BlockState state) const -> const BlockStateRenderProperties& {
        if (!renderProperties.contains(state))
            return MISSING_BLOCK_RENDER_PROPERTIES;

        return renderProperties.at(state);
    }

    auto BlockRegistry::blockStatePropertyMap(const BlockState state) const -> const BlockStatePropertyMap& {
        if (!blockStatePropertyMaps.contains(state))
            return MISSING_BLOCK_PROPERTY_MAP;

        return blockStatePropertyMaps.at(state);
    }

    auto BlockRegistry::blockState(const std::string_view name) const -> BlockState {
        if (!blockStateByName.contains(name))
            return MISSING_BLOCK_STATE;

        return blockStateByName.at(name);
    }

    auto BlockRegistry::blockStateRenderProperties(const std::string_view name) const -> const BlockStateRenderProperties& {
        return blockStateRenderProperties(blockState(name));
    }

    auto BlockRegistry::blockName(const BlockType &type) const -> const std::string& {
        if (!blockNameByState.contains(type.min))
            return MISSING_BLOCK_NAME;

        return blockNameByState.at(type.min);
    }

    auto BlockRegistry::blockName(const BlockState state) const -> const std::string& {
        if (!blockNameByState.contains(state))
            return MISSING_BLOCK_NAME;

        return blockNameByState.at(state);
    }

    auto BlockRegistry::blockStateName(const BlockState state) const -> const std::string& {
        if (!blockStateNameByState.contains(state))
            return MISSING_BLOCK_NAME;

        return blockStateNameByState.at(state);
    }

    auto BlockRegistry::blockType(const std::string_view name) const -> const BlockType& {
        if (!blockTypeByName.contains(name))
            return MISSING_BLOCK_TYPE;

        return blockTypes.at(blockTypeByName.at(name));
    }

    auto BlockRegistry::blockType(const BlockState state) const -> const BlockType& {
        if (!blockTypeByState.contains(state))
            return MISSING_BLOCK_TYPE;

        return blockTypes.at(blockTypeByState.at(state));
    }

    auto BlockRegistry::extendBlockType(const BlockState additionalState, const std::string &stateName) -> void {
        const auto [blockName, properties] = parseBlockStateProperties(stateName);
        blockStatePropertyMaps[additionalState] = properties;
        blockNameByState[additionalState] = blockName;
        blockTypeByState[additionalState] = currentBlockTypeId;

        if (!blockTypeByName.contains(blockName)) {
            blockTypeByName[blockName] = currentBlockTypeId;
            blockTypes.emplace_back(additionalState, additionalState);
            currentBlockTypeId++;
            return;
        }
        auto&[min, max] = blockTypes[blockTypeByName.at(blockName)];

        if (additionalState < min) min = additionalState;
        if (additionalState > max) max = additionalState;
    }
}