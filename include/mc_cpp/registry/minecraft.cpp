#include <mc_cpp/registry/minecraft.hpp>

namespace mc {
    auto stripMinecraftNamespace(std::string_view *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter != std::string::npos && namespacedId->starts_with("minecraft:"))
            *namespacedId = namespacedId->substr(delimiter + 1);
    }

    auto stripMinecraftNamespace(std::string *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter != std::string::npos && namespacedId->starts_with("minecraft:"))
            *namespacedId = namespacedId->substr(delimiter + 1);
    }

    auto prependMinecraftNamespace(std::string *namespacedId) -> void {
        if (const auto delimiter = namespacedId->find(':');
            delimiter == std::string::npos)
            *namespacedId = "minecraft:" + *namespacedId;
    }

    MinecraftRegistry::MinecraftRegistry(const SupportedMinecraftVersion version,
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
        for (auto &tileEntity : tileEntities.tileEntitiesById | std::views::values) {
            tileEntity.blockStates.reserve(tileEntity.blockTypes.size());
            for (const auto &block : tileEntity.blockTypes) {
                const auto [min, max] = blockType(block);
                if (min == MISSING_BLOCK_STATE) continue;

                for (BlockState state = min; state <= max; state++) {
                    tileEntity.blockStates.emplace(state);
                }
            }
        }
    }

    auto MinecraftRegistry::minecraftVersion() const -> SupportedMinecraftVersion {
        return version;
    }

    auto MinecraftRegistry::namedVersion() const -> std::string_view {
        return NAMED_VERSIONS[version];
    }

    auto MinecraftRegistry::dataVersion() const -> int32_t {
        return DATA_VERSIONS[version];
    }

    auto MinecraftRegistry::protocolVersion() const -> int32_t {
        return PROTOCOL_VERSIONS[version];
    }

    auto MinecraftRegistry::load(const std::filesystem::path &parentDirectory, const SupportedMinecraftVersion version) -> result::Result<std::shared_ptr<MinecraftRegistry>, std::string> {
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

    bool MinecraftRegistry::isWaterBlock(const BlockState state) const {
        return state == waterBlock;
    }

    bool MinecraftRegistry::isGrassBlock(const BlockState state) const {
        return state == grassBlock;
    }

    bool MinecraftRegistry::isFoliageBlock(const BlockState state) const {
        return foliage.contains(state);
    }

    auto MinecraftRegistry::blockStateRenderProperties(const BlockState state) const -> const BlockStateRenderProperties& {
        return blocks.blockStateRenderProperties(state);
    }

    auto MinecraftRegistry::blockState(const std::string_view name) const -> BlockState {
        return blocks.blockState(name);
    }

    auto MinecraftRegistry::blockStatePropertyMap(const BlockState state) const -> const BlockStatePropertyMap& {
        return blocks.blockStatePropertyMap(state);
    }

    auto MinecraftRegistry::blockStateRenderProperties(const std::string_view name) const -> const BlockStateRenderProperties& {
        return blocks.blockStateRenderProperties(name);
    }

    auto MinecraftRegistry::blockName(const BlockType &type) const -> const std::string& {
        return blocks.blockName(type);
    }

    auto MinecraftRegistry::blockName(const BlockState state) const -> const std::string& {
        return blocks.blockName(state);
    }

    auto MinecraftRegistry::blockType(const std::string_view name) const -> const BlockType& {
        return blocks.blockType(name);
    }

    auto MinecraftRegistry::blockType(const BlockState state) const -> const BlockType& {
        return blocks.blockType(state);
    }

    auto MinecraftRegistry::blockStateColor(const RGBA &biomeColor, const BlockState state) const -> RGBA {
        const auto textureBrightness = blocks.blockStateRenderProperties(state).textureBrightness;
        if (isWaterBlock(state) || isGrassBlock(state) || isFoliageBlock(state))
            return multiplyColor(biomeColor, textureBrightness);

        return blocks.blockStateRenderProperties(state).illuminatedColor;
    }

    auto MinecraftRegistry::biomeBlockStateColor(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> RGBA {
        return blockStateColor(biomeColorMultiplier(biomeType, yLevel, state), state);
    }

    auto MinecraftRegistry::biomeColorMultiplier(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> const RGBA& {
        return isWaterBlock(state) ? biomes.biomeWaterColor(biomeType) : biomeFoliageColor(biomeType, yLevel, state);
    }

    auto MinecraftRegistry::biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const BlockState state) const -> const RGBA& {
        if (const auto isGrass = isGrassBlock(state); isGrass || isFoliageBlock(state))
            return biomes.biomeFoliageColor(biomeType, yLevel, isGrass);

        return WHITE;
    }

    auto MinecraftRegistry::biomeFoliageColor(const BiomeType biomeType, const int16_t yLevel, const bool isGrass) const -> const RGBA& {
        return biomes.biomeFoliageColor(biomeType, yLevel, isGrass);
    }

    auto MinecraftRegistry::biomeWaterColor(const BiomeProperties &properties) const -> const RGBA& {
        return biomes.biomeWaterColor(properties);
    }

    auto MinecraftRegistry::biomeWaterColor(const BiomeType biomeType) const -> const RGBA& {
        return biomes.biomeWaterColor(biomeType);
    }

    auto MinecraftRegistry::biomeWaterColor(const BiomeEnvironment biomeEnv) const -> const RGBA& {
        return biomes.biomeWaterColor(biomeEnv);
    }

    auto MinecraftRegistry::biomeColorMultiplier(const float temperature, const float downfall, const int y, const bool isGrass) const -> RGBA {
        return biomes.biomeColors.biomeColorMultiplier(temperature, downfall, y, isGrass);
    }

    auto MinecraftRegistry::biomeProperties(const uint8_t id) const -> const BiomeProperties& {
        return biomes.biomeProperties.biomeProperties(id);
    }

    auto MinecraftRegistry::biomeProperties(const std::string_view biomeName) const -> const BiomeProperties& {
        return biomes.biomeProperties.biomeProperties(biomeName);
    }

    auto MinecraftRegistry::biomeType(const std::string_view biomeName) const -> BiomeType {
        return biomes.biomeProperties.biomeType(biomeName);
    }

    auto MinecraftRegistry::biomeName(const BiomeType biomeType) const -> const std::string& {
        return biomes.biomeProperties.biomeName(biomeType);
    }

    auto MinecraftRegistry::tileEntity(const uint16_t tileEntityId) const -> const TileEntityEntry& {
        return tileEntities.tileEntity(tileEntityId);
    }

    auto MinecraftRegistry::tileEntity(const std::string_view tileEntityName) const -> const TileEntityEntry& {
        return tileEntities.tileEntity(tileEntityName);
    }

    auto MinecraftRegistry::tileEntityId(const std::string_view tileEntityName) const -> uint16_t {
        return tileEntities.tileEntityId(tileEntityName);
    }

    auto MinecraftRegistry::tileEntityName(const uint16_t tileEntityId) const -> const std::string& {
        return tileEntities.tileEntityName(tileEntityId);
    }

    auto loadRegistries(const std::filesystem::path &parentDirectory,
                               const std::vector<SupportedMinecraftVersion> &mcVersions) -> result::Result<std::monostate, std::string> {
        for (const auto mcVersion : mcVersions)
            REGISTRIES.emplace(mcVersion, TRY(MinecraftRegistry::load(parentDirectory, mcVersion)));

        return {};
    }

    auto getRegistry(const SupportedMinecraftVersion version) -> const MinecraftRegistry& {
        return *REGISTRIES.at(version);
    }

    auto getRegistry(const int32_t dataVersion) -> std::shared_ptr<MinecraftRegistry> {
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