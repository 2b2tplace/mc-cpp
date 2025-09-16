#pragma once

#include <absl/container/flat_hash_set.h>
#include <mc_cpp/registry/registries/biomes.hpp>
#include <mc_cpp/registry/registries/blockstates.hpp>

#include "../../../cmake-build-debug/_deps/result-src/include/result.hpp"

namespace mc {

    class MinecraftRegistry {
        BlockType waterBlock{};
        BlockType grassBlock{};
        absl::flat_hash_set<BlockState> foliage;

    public:
        BiomeRegistry biomes;
        BlockRegistry blocks;
  // TileEntityRegistry tileEntities;

        explicit MinecraftRegistry(const Registry<BiomeColorEntry> &foliageColors,
                                   const Registry<BiomeColorEntry> &grassColors,
                                   const Registry<BiomeColorEntry> &waterColors,
                                   const Registry<ColorTriangleEntry> &colorTriangles,
                                   const Registry<BiomePropertyEntry> &biomePropertyRegistry,
                                   const Registry<BlockStateEntry> &blockStateRegistry):
            biomes(BiomeRegistry { foliageColors, grassColors, waterColors, colorTriangles, biomePropertyRegistry }),
            blocks(BlockRegistry { blockStateRegistry }) {

            waterBlock = blocks.blockType("water");
            grassBlock = blocks.blockType("grass_block");

            for (const auto foliageBlockName : FOLIAGE_BLOCK_TYPES) {
                const auto[min, max] = blocks.blockType(foliageBlockName);

                for (BlockState b = min; b <= max; b++)
                    foliage.emplace(b);
            }
        }

        static auto load(const std::filesystem::path &parentDirectory) -> result::Result<MinecraftRegistry, std::string> {
            Registry<BiomeColorEntry>    foliageColorRegistry;
            Registry<BiomeColorEntry>    grassColorRegistry;
            Registry<BiomeColorEntry>    waterColorRegistry;
            Registry<ColorTriangleEntry> colorTriangleRegistry;
            Registry<BiomePropertyEntry> biomePropertyRegistry;
            Registry<BlockStateEntry>    blockStateRegistry;

            TRY(foliageColorRegistry .tryLoad(parentDirectory / "foliage_colors.json"));
            TRY(grassColorRegistry   .tryLoad(parentDirectory / "grass_colors.json"));
            TRY(waterColorRegistry   .tryLoad(parentDirectory / "water_colors.json"));
            TRY(colorTriangleRegistry.tryLoad(parentDirectory / "color_triangles.json"));
            TRY(biomePropertyRegistry.tryLoad(parentDirectory / "biome_properties.json"));
            TRY(blockStateRegistry   .tryLoad(parentDirectory / "blockstates.json"));

            return MinecraftRegistry {
                foliageColorRegistry,
                grassColorRegistry,
                waterColorRegistry,
                colorTriangleRegistry,
                biomePropertyRegistry,
                blockStateRegistry
            };
        }

        [[nodiscard]]
        bool isWaterBlock(const BlockState state) const {
            return state == waterBlock;
        }

        [[nodiscard]]
        bool isGrassBlock(const BlockState state) const {
            return state == grassBlock;
        }

        [[nodiscard]]
        bool isFoliageBlock(const BlockState state) const {
            return foliage.contains(state);
        }

        [[nodiscard]]
        auto blockStateProperties(const BlockState state) const -> const BlockStateProperties& {
            return blocks.blockStateProperties(state);
        }

        [[nodiscard]]
        auto blockState(const std::string_view name) const -> BlockState {
            return blocks.blockState(name);
        }

        [[nodiscard]]
        auto blockStateProperties(const std::string_view name) const -> const BlockStateProperties& {
            return blocks.blockStateProperties(name);
        }

        [[nodiscard]]
        auto blockType(const std::string_view name) const -> const BlockType& {
            return blocks.blockType(name);
        }

        [[nodiscard]]
        auto blockType(const BlockState state) const -> const BlockType& {
            return blocks.blockType(state);
        }

        [[nodiscard]]
        auto blockStateColor(const RGBA &biomeColor, const BlockState state) const -> RGBA {
            const auto textureBrightness = blocks.blockStateProperties(state).textureBrightness;
            if (isWaterBlock(state) || isGrassBlock(state) || isFoliageBlock(state))
                return multiplyColor(biomeColor, textureBrightness);

            return blocks.blockStateProperties(state).illuminatedColor;
        }

        [[nodiscard]]
        auto biomeBlockStateColor(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> RGBA {
            return blockStateColor(biomeColorMultiplier(biomeType, yLevel, state), state);
        }

        [[nodiscard]]
        auto biomeColorMultiplier(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> const RGBA& {
            return isWaterBlock(state) ? biomes.biomeWaterColor(biomeType) : biomeFoliageColor(biomeType, yLevel, state);
        }

        [[nodiscard]]
        auto biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> const RGBA& {
            if (const auto isGrass = isGrassBlock(state); isGrass || isFoliageBlock(state))
                return biomes.biomeFoliageColor(biomeType, yLevel, isGrass);

            return WHITE;
        }

        [[nodiscard]]
        auto biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const bool isGrass) const -> const RGBA& {
            return biomes.biomeFoliageColor(biomeType, yLevel, isGrass);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeProperties &properties) const -> const RGBA& {
            return biomes.biomeWaterColor(properties);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeType biomeType) const -> const RGBA& {
            return biomes.biomeWaterColor(biomeType);
        }

        [[nodiscard]]
        auto biomeWaterColor(const BiomeEnvironment biomeEnv) const -> const RGBA& {
            return biomes.biomeWaterColor(biomeEnv);
        }

        [[nodiscard]]
        auto biomeColorMultiplier(const float temperature, const float downfall, const int y, const bool isGrass) const -> RGBA {
            return biomes.biomeColors.biomeColorMultiplier(temperature, downfall, y, isGrass);
        }

        [[nodiscard]]
        auto biomeProperties(const uint8_t id) const -> const BiomeProperties& {
            return biomes.biomeProperties.biomeProperties(id);
        }

        [[nodiscard]]
        auto biomeProperties(const std::string_view biomeName) const -> const BiomeProperties& {
            return biomes.biomeProperties.biomeProperties(biomeName);
        }

        [[nodiscard]]
        auto biomeType(const std::string_view biomeName) const -> BiomeType {
            return biomes.biomeProperties.biomeType(biomeName);
        }

    };

}
