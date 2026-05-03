#include <mc_cpp/anvil/level_dat/world_generator.hpp>

namespace mc {
    auto DebugWorldGenerator::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        compound.put<std::string>("type", identifier.data());
    }

    FlatWorldGenerator::Layer::Layer(const MinecraftRegistry &registry, const int32_t height,
        const std::string_view block):
        height(height), block(registry.blockType(block)) {}

    FlatWorldGenerator::Layer::Layer(const MinecraftRegistry &registry, const int32_t height):
        Layer(registry, height, "air") {}

    auto FlatWorldGenerator::Layer::writeCompound(const MinecraftRegistry &registry,
        NbtCompound &compound) const -> void {
        auto blockName = registry.blockName(block);
        prependMinecraftNamespace(&blockName);
        compound.put("block", blockName);
        compound.put("height", height);
    }

    auto FlatWorldGenerator::Layer::
    readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        std::string blockName;
        compound.read("block", blockName);
        compound.read("height", height);

        stripMinecraftNamespace(&blockName);
        block = registry.blockType(blockName);
    }

    FlatWorldGenerator::FlatWorldGenerator(const MinecraftRegistry &registry): biome(registry.biomeType("plains")) {}

    auto FlatWorldGenerator::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        NbtCompound settings;

        auto biomeName = registry.biomeName(biome);
        prependMinecraftNamespace(&biomeName);
        settings.put("biome", biomeName);
        settings.put("features", features);
        settings.put("lakes", lakes);

        NbtList layersNBT{NbtType::COMPOUND};
        for (const auto &layer : layers)
            layersNBT.add(layer.createCompound(registry));

        settings.putNbt("layers", layersNBT);
        compound.putNbt("settings", settings);
        compound.put<std::string>("type", identifier.data());
    }

    auto FlatWorldGenerator::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        const auto &settings = compound.readNbt<NbtCompound>("settings");

        std::string biomeName;
        settings.read("biome", biomeName);
        settings.read("features", features);
        settings.read("lakes", lakes);

        prependMinecraftNamespace(&biomeName);
        biome = registry.biomeType(biomeName);

        const auto &layersNBT = settings.readNbt<NbtList>("layers");
        for (const auto &layerNBTPtr : layersNBT.values) {
            Layer layer;
            layer.readCompound(registry, getAsTag<const NbtCompound&>(*layerNBTPtr));
            layers.push_back(layer);
        }
    }

    NoiseWorldGenerator::BiomeSource::BiomeSource(std::string preset, std::string type):
        preset(std::move(preset)), type(std::move(type)) {}

    NoiseWorldGenerator::BiomeSource::BiomeSource(std::string type):
        type(std::move(type)) {}

    auto NoiseWorldGenerator::BiomeSource::overworld() -> BiomeSource {
        return BiomeSource{"minecraft:overworld", "minecraft:multi_noise"};
    }

    auto NoiseWorldGenerator::BiomeSource::nether() -> BiomeSource {
        return BiomeSource{"minecraft:nether", "minecraft:multi_noise"};
    }

    auto NoiseWorldGenerator::BiomeSource::end() -> BiomeSource {
        return BiomeSource{"minecraft:the_end"};
    }

    auto NoiseWorldGenerator::BiomeSource::writeCompound(const MinecraftRegistry &registry,
        NbtCompound &compound) const -> void {
        if (preset) compound.put("preset", *preset);
        compound.put("type", type);
    }

    auto NoiseWorldGenerator::BiomeSource::readCompound(const MinecraftRegistry &registry,
        const NbtCompound &compound) -> void {
        if (compound.contains<std::string>("preset"))
            compound.read("preset", *preset);

        compound.read("type", type);
    }

    NoiseWorldGenerator::NoiseWorldGenerator(const DimensionType dimension) {
        switch (dimension) {
            case OVERWORLD:
                biomeSource = BiomeSource::overworld();
                settings = "minecraft:overworld";
                break;
            case NETHER:
                biomeSource = BiomeSource::nether();
                settings = "minecraft:nether";
                break;
            case THE_END:
                biomeSource = BiomeSource::end();
                settings = "minecraft:end";
                break;
            default:
                std::unreachable();
        }
    }

    auto NoiseWorldGenerator::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        compound.putNbt("biome_source", biomeSource.createCompound(registry));
        compound.put("settings", settings);
        compound.put("type", std::string{identifier});
    }

    auto NoiseWorldGenerator::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        const auto &biomeSourceNBT = compound.readNbt<NbtCompound>("biome_source");
        biomeSource.readCompound(registry, biomeSourceNBT);

        compound.read("settings", settings);
    }

    WorldGenerator::WorldGenerator(const MinecraftRegistry &registry, const WorldGeneratorType type,
        const DimensionType dimension):
        dimension(dimension), type_(type) {
        switch (type) {
            case WorldGeneratorType::DEBUG:
                worldGeneratorPtr = std::make_unique<DebugWorldGenerator>();
                break;
            case WorldGeneratorType::FLAT:
                worldGeneratorPtr = std::make_unique<FlatWorldGenerator>(registry);
                break;
            case WorldGeneratorType::NOISE:
                worldGeneratorPtr = std::make_unique<NoiseWorldGenerator>(dimension);
                break;
        }
    }

    auto WorldGenerator::type() const -> WorldGeneratorType {
        return type_;
    }

    auto WorldGenerator::asDebug() const -> const DebugWorldGenerator * {
        return dynamic_cast<const DebugWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::asDebug() -> DebugWorldGenerator * {
        return dynamic_cast<DebugWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::asFlat() const -> const FlatWorldGenerator * {
        return dynamic_cast<const FlatWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::asFlat() -> FlatWorldGenerator * {
        return dynamic_cast<FlatWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::asNoise() const -> const NoiseWorldGenerator * {
        return dynamic_cast<const NoiseWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::asNoise() -> NoiseWorldGenerator * {
        return dynamic_cast<NoiseWorldGenerator*>(worldGeneratorPtr.get());
    }

    auto WorldGenerator::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        compound.putNbt("generator", worldGeneratorPtr->createCompound(registry));
        compound.put<std::string>("type", getNamespacedDimension(dimension));
    }

    auto WorldGenerator::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        std::string dimensionName;
        compound.read<std::string>("type", dimensionName);
        dimension = getDimensionType(dimensionName);

        const auto &generator = compound.readNbt<NbtCompound>("generator");
        const auto &generatorTypeStr = generator.read<std::string>("type");

        if (generatorTypeStr == DebugWorldGenerator::identifier)
            worldGeneratorPtr = std::make_unique<DebugWorldGenerator>();
        else if (generatorTypeStr == FlatWorldGenerator::identifier)
            worldGeneratorPtr = std::make_unique<FlatWorldGenerator>(registry);
        else if (generatorTypeStr == NoiseWorldGenerator::identifier)
            worldGeneratorPtr = std::make_unique<NoiseWorldGenerator>(dimension);
    }

    WorldGeneratorSettings::WorldGeneratorSettings(const MinecraftRegistry &registry, const WorldGeneratorType type) {
        for (size_t i = 0; i < DIMENSION_NAMES.size(); i++) {
            const auto dimension = static_cast<DimensionType>(i);
            dimensions.emplace(getNamespacedDimension(dimension), WorldGenerator{registry, type, dimension});
        }
    }

    WorldGeneratorSettings WorldGeneratorSettings::defaultWorld(const MinecraftRegistry &registry) {
        return WorldGeneratorSettings{registry, WorldGeneratorType::NOISE};
    }

    WorldGeneratorSettings WorldGeneratorSettings::emptyWorld(const MinecraftRegistry &registry) {
        return WorldGeneratorSettings{registry, WorldGeneratorType::FLAT};
    }

    auto WorldGeneratorSettings::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        compound.put("bonus_chest", bonusChest);
        compound.put("seed", seed);
        compound.put("generate_features", generateFeatures);

        NbtCompound dimensionsNBT;
        for (const auto &[dimension, generator] : dimensions)
            dimensionsNBT.putNbt(dimension, generator.createCompound(registry));

        compound.putNbt("dimensions", dimensionsNBT);
    }

    auto WorldGeneratorSettings::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        compound.read("bonus_chest", bonusChest);
        compound.read("seed", seed);
        compound.read("generate_features", generateFeatures);

        const auto &dimensionsNBT = compound.readNbt<NbtCompound>("dimensions");
        for (const auto &dimensionName : dimensionsNBT.getKeys()) {
            const auto &generatorNBT = dimensionsNBT.readNbt<NbtCompound>(dimensionName);

            WorldGenerator generator;
            generator.readCompound(registry, generatorNBT);
            dimensions.emplace(dimensionName, std::move(generator));
        }
    }
}
