#pragma once

#include <absl/container/flat_hash_set.h>
#include <mc_cpp/registry/registries/biomes.hpp>
#include <mc_cpp/registry/registries/blockstates.hpp>
#include <mc_cpp/registry/registries/tile_entities.hpp>
#include <result.hpp>

namespace mc {

    enum SupportedMinecraftVersion {
        RELEASE_1_19_4,
        RELEASE_1_20_4,
        RELEASE_1_21_4,
        _SUPPORTED_MINECRAFT_VERSION_COUNT
    };

    static constexpr std::array<uint16_t, _SUPPORTED_MINECRAFT_VERSION_COUNT> DATA_VERSIONS = {
        3337,
        3700,
        4189
    };

    static constexpr std::array<uint16_t, _SUPPORTED_MINECRAFT_VERSION_COUNT> PROTOCOL_VERSIONS = {
        762,
        765,
        769
    };

    static constexpr std::array<std::string_view, _SUPPORTED_MINECRAFT_VERSION_COUNT> NAMED_VERSIONS = {
        "1.19.4",
        "1.20.4",
        "1.21.4"
    };

    inline auto stripMinecraftNamespace(std::string_view *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter != std::string::npos && namespacedId->starts_with("minecraft:"))
            *namespacedId = namespacedId->substr(delimiter + 1);
    }

    inline auto stripMinecraftNamespace(std::string *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter != std::string::npos && namespacedId->starts_with("minecraft:"))
            *namespacedId = namespacedId->substr(delimiter + 1);
    }

    inline auto prependMinecraftNamespace(std::string *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter == std::string::npos)
            *namespacedId = "minecraft:" + *namespacedId;
    }

    class MinecraftRegistry {
        BlockType waterBlock{};
        BlockType grassBlock{};
        absl::flat_hash_set<BlockState> foliage;
        SupportedMinecraftVersion version;

    public:
        BiomeRegistry biomes;
        BlockRegistry blocks;
        TileEntityRegistry tileEntities;

        explicit MinecraftRegistry(const SupportedMinecraftVersion version,
                                   const Registry<BiomeColorEntry> &foliageColors,
                                   const Registry<BiomeColorEntry> &grassColors,
                                   const Registry<BiomeColorEntry> &waterColors,
                                   const Registry<ColorTriangleEntry> &colorTriangles,
                                   const Registry<BiomePropertyEntry> &biomePropertyRegistry,
                                   const Registry<BlockStateEntry> &blockStateRegistry,
                                   const Registry<TileEntityEntry> &tileEntityRegistry):
            version(version),
            biomes(BiomeRegistry { foliageColors, grassColors, waterColors, colorTriangles, biomePropertyRegistry }),
            blocks(BlockRegistry { blockStateRegistry }),
            tileEntities(TileEntityRegistry { tileEntityRegistry }) {

            waterBlock = blocks.blockType("water");
            grassBlock = blocks.blockType("grass_block");

            for (const auto foliageBlockName : FOLIAGE_BLOCK_TYPES) {
                const auto[min, max] = blocks.blockType(foliageBlockName);

                for (BlockState b = min; b <= max; b++)
                    foliage.emplace(b);
            }
        }

        [[nodiscard]]
        auto minecraftVersion() const -> SupportedMinecraftVersion {
            return version;
        }

        [[nodiscard]]
        auto namedVersion() const -> std::string_view {
            return NAMED_VERSIONS[version];
        }

        [[nodiscard]]
        auto dataVersion() const -> int32_t {
            return DATA_VERSIONS[version];
        }

        [[nodiscard]]
        auto protocolVersion() const -> int32_t {
            return PROTOCOL_VERSIONS[version];
        }

        static auto load(const std::filesystem::path &parentDirectory, const SupportedMinecraftVersion version) -> result::Result<std::shared_ptr<MinecraftRegistry>, std::string> {
            Registry<BiomeColorEntry>    foliageColorRegistry;
            Registry<BiomeColorEntry>    grassColorRegistry;
            Registry<BiomeColorEntry>    waterColorRegistry;
            Registry<ColorTriangleEntry> colorTriangleRegistry;
            Registry<BiomePropertyEntry> biomePropertyRegistry;
            Registry<BlockStateEntry>    blockStateRegistry;
            Registry<TileEntityEntry>    tileEntityRegistry;

            const auto directory = parentDirectory / std::to_string(PROTOCOL_VERSIONS[version]);

            TRY(foliageColorRegistry .tryLoad(directory / "foliage_colors.json"));
            TRY(grassColorRegistry   .tryLoad(directory / "grass_colors.json"));
            TRY(waterColorRegistry   .tryLoad(directory / "water_colors.json"));
            TRY(colorTriangleRegistry.tryLoad(directory / "color_triangles.json"));
            TRY(biomePropertyRegistry.tryLoad(directory / "biome_properties.json"));
            TRY(blockStateRegistry   .tryLoad(directory / "blockstates.json"));
            TRY(tileEntityRegistry   .tryLoad(directory / "tile_entities.json"));

            return std::make_shared<MinecraftRegistry>(
                version,
                foliageColorRegistry,
                grassColorRegistry,
                waterColorRegistry,
                colorTriangleRegistry,
                biomePropertyRegistry,
                blockStateRegistry,
                tileEntityRegistry
            );
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
        auto blockStateRenderProperties(const BlockState state) const -> const BlockStateRenderProperties& {
            return blocks.blockStateRenderProperties(state);
        }

        [[nodiscard]]
        auto blockState(const std::string_view name) const -> BlockState {
            return blocks.blockState(name);
        }

        [[nodiscard]]
        auto blockStatePropertyMap(const BlockState state) const -> const BlockStatePropertyMap& {
            return blocks.blockStatePropertyMap(state);
        }

        [[nodiscard]]
        auto blockStateRenderProperties(const std::string_view name) const -> const BlockStateRenderProperties& {
            return blocks.blockStateRenderProperties(name);
        }

        [[nodiscard]]
        auto blockName(const BlockType &type) const -> const std::string& {
            return blocks.blockName(type);
        }

        [[nodiscard]]
        auto blockName(const BlockState state) const -> const std::string& {
            return blocks.blockName(state);
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
            const auto textureBrightness = blocks.blockStateRenderProperties(state).textureBrightness;
            if (isWaterBlock(state) || isGrassBlock(state) || isFoliageBlock(state))
                return multiplyColor(biomeColor, textureBrightness);

            return blocks.blockStateRenderProperties(state).illuminatedColor;
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

        [[nodiscard]]
        auto biomeName(const BiomeType biomeType) const -> const std::string& {
            return biomes.biomeProperties.biomeName(biomeType);
        }

        [[nodiscard]]
        auto tileEntity(const uint16_t tileEntityId) const -> const TileEntityEntry& {
            return tileEntities.tileEntity(tileEntityId);
        }

        [[nodiscard]]
        auto tileEntity(const std::string_view tileEntityName) const -> const TileEntityEntry& {
            return tileEntities.tileEntity(tileEntityName);
        }

        [[nodiscard]]
        auto tileEntityId(const std::string_view tileEntityName) const -> uint16_t {
            return tileEntities.tileEntityId(tileEntityName);
        }

        [[nodiscard]]
        auto tileEntityName(const uint16_t tileEntityId) const -> const std::string& {
            return tileEntities.tileEntityName(tileEntityId);
        }
    };

    using RegistryHolder = absl::flat_hash_map<SupportedMinecraftVersion, std::shared_ptr<MinecraftRegistry>>;

    inline auto REGISTRIES = RegistryHolder{};

    [[nodiscard]]
    inline auto loadRegistries(const std::filesystem::path &parentDirectory,
                               const std::vector<SupportedMinecraftVersion> &mcVersions) -> result::Result<std::monostate, std::string> {
        for (const auto mcVersion : mcVersions)
            REGISTRIES.emplace(mcVersion, TRY(MinecraftRegistry::load(parentDirectory, mcVersion)));

        return {};
    }

    [[nodiscard]]
    inline auto getRegistry(const SupportedMinecraftVersion version) -> const MinecraftRegistry& {
        return *REGISTRIES.at(version);
    }

    [[nodiscard]]
    inline auto getRegistry(const int32_t dataVersion) -> std::shared_ptr<MinecraftRegistry> {
        for (size_t i = 0; i < DATA_VERSIONS.size(); i++) {
            const auto version = static_cast<SupportedMinecraftVersion>(i);
            if (DATA_VERSIONS[i] == dataVersion && REGISTRIES.contains(version)) {
                const auto registry = REGISTRIES.at(version);
                if (!registry) continue;

                return registry;
            }
        }
        return nullptr;
    }

}
