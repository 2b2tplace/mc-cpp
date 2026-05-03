#pragma once

#include <absl/container/flat_hash_set.h>
#include <mc_cpp/registry/registries/biomes.hpp>
#include <mc_cpp/registry/registries/blockstates.hpp>
#include <mc_cpp/registry/registries/tile_entities.hpp>
#include <mc_cpp/registry/registries/legacy_biomes.hpp>
#include <mc_cpp/registry/registries/legacy_blockstates.hpp>
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

    auto stripMinecraftNamespace(std::string_view *namespacedId) -> void;

    auto stripMinecraftNamespace(std::string *namespacedId) -> void;

    auto prependMinecraftNamespace(std::string *namespacedId) -> void;

    class MinecraftRegistry {
        BlockType waterBlock{};
        BlockType grassBlock{};
        absl::flat_hash_set<BlockState> foliage;
        SupportedMinecraftVersion version;

    public:
        BiomeRegistry biomes;
        BlockRegistry blocks;
        TileEntityRegistry tileEntities;
        LegacyBlockUpgradeMap legacyBlockUpgradeMap;
        LegacyBiomeTypeUpgradeMap legacyBiomeTypeUpgradeMap;

        explicit MinecraftRegistry(SupportedMinecraftVersion version,
                                   const Registry<BiomeColorEntry> &foliageColors,
                                   const Registry<BiomeColorEntry> &grassColors,
                                   const Registry<BiomeColorEntry> &waterColors,
                                   const Registry<ColorTriangleEntry> &colorTriangles,
                                   const Registry<BiomePropertyEntry> &biomePropertyRegistry,
                                   const Registry<BlockStateEntry> &blockStateRegistry,
                                   const Registry<TileEntityEntry> &tileEntityRegistry);

        [[nodiscard]]
        auto minecraftVersion() const -> SupportedMinecraftVersion;

        [[nodiscard]]
        auto namedVersion() const -> std::string_view;

        [[nodiscard]]
        auto dataVersion() const -> int32_t;

        [[nodiscard]]
        auto protocolVersion() const -> int32_t;

        static auto load(const std::filesystem::path &parentDirectory, SupportedMinecraftVersion version) -> result::Result<std::unique_ptr<MinecraftRegistry>, std::string>;

        [[nodiscard]]
        bool isWaterBlock(BlockState state) const;

        [[nodiscard]]
        bool isGrassBlock(BlockState state) const;

        [[nodiscard]]
        bool isFoliageBlock(BlockState state) const;

        [[nodiscard]]
        auto blockStateRenderProperties(BlockState state) const -> const BlockStateRenderProperties&;

        [[nodiscard]]
        auto blockState(std::string_view name) const -> BlockState;

        [[nodiscard]]
        auto blockStateForLegacy(uint8_t id, uint8_t data) const -> BlockState;

        [[nodiscard]]
        auto blockStatePropertyMap(BlockState state) const -> const BlockStatePropertyMap&;

        [[nodiscard]]
        auto blockStateRenderProperties(std::string_view name) const -> const BlockStateRenderProperties&;

        [[nodiscard]]
        auto blockName(const BlockType &type) const -> const std::string&;

        [[nodiscard]]
        auto blockName(BlockState state) const -> const std::string&;

        [[nodiscard]]
        auto blockStateName(BlockState state) const -> const std::string&;

        [[nodiscard]]
        auto blockType(std::string_view name) const -> const BlockType&;

        [[nodiscard]]
        auto blockType(BlockState state) const -> const BlockType&;

        [[nodiscard]]
        auto blockStateColor(const RGBA &biomeColor, BlockState state) const -> RGBA;

        [[nodiscard]]
        auto biomeBlockStateColor(BiomeType biomeType, int16_t yLevel, BlockState state) const -> RGBA;

        [[nodiscard]]
        auto biomeColorMultiplier(BiomeType biomeType, int16_t yLevel, BlockState state) const -> const RGBA&;

        [[nodiscard]]
        auto biomeFoliageColor(BiomeType biomeType, int16_t yLevel, BlockState state) const -> const RGBA&;

        [[nodiscard]]
        auto biomeFoliageColor(BiomeType biomeType, int16_t yLevel, bool isGrass) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(const BiomeProperties &properties) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(BiomeType biomeType) const -> const RGBA&;

        [[nodiscard]]
        auto biomeWaterColor(BiomeEnvironment biomeEnv) const -> const RGBA&;

        [[nodiscard]]
        auto biomeColorMultiplier(float temperature, float downfall, int y, bool isGrass) const -> RGBA;

        [[nodiscard]]
        auto biomeProperties(uint8_t id) const -> const BiomeProperties&;

        [[nodiscard]]
        auto biomeProperties(std::string_view biomeName) const -> const BiomeProperties&;

        [[nodiscard]]
        auto biomeType(std::string_view biomeName) const -> BiomeType;

        [[nodiscard]]
        auto biomeTypeForLegacy(LegacyBiomeType legacyBiomeType) const -> BiomeType;

        [[nodiscard]]
        auto biomeName(BiomeType biomeType) const -> const std::string&;

        [[nodiscard]]
        auto tileEntity(uint16_t tileEntityId) const -> const TileEntityEntry&;

        [[nodiscard]]
        auto tileEntity(std::string_view tileEntityName) const -> const TileEntityEntry&;

        [[nodiscard]]
        auto tileEntityId(std::string_view tileEntityName) const -> uint16_t;

        [[nodiscard]]
        auto tileEntityName(uint16_t tileEntityId) const -> const std::string&;
    };

    using RegistryHolder = absl::flat_hash_map<SupportedMinecraftVersion, std::unique_ptr<MinecraftRegistry>>;

    inline auto REGISTRIES = RegistryHolder{};

    [[nodiscard]]
    auto loadRegistries(const std::filesystem::path &parentDirectory,
                        const std::vector<SupportedMinecraftVersion> &mcVersions,
                        bool loadLegacyRegistries = false) -> result::Result<std::monostate, std::string>;

    [[nodiscard]]
    auto getRegistry(SupportedMinecraftVersion version) -> const MinecraftRegistry&;

    [[nodiscard]]
    auto getRegistry(int32_t dataVersion) -> const std::unique_ptr<MinecraftRegistry>&;

}
