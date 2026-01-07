#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>

namespace mc {

    enum class WorldGeneratorType {
        DEBUG, FLAT, NOISE
    };

    struct DebugWorldGenerator final : public ToCompound {
        static constexpr std::string_view identifier = "minecraft:debug";

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.put<std::string>("type", identifier.data());
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {

        }
    };

    struct FlatWorldGenerator final : public ToCompound {
        static constexpr std::string_view identifier = "minecraft:flat";

        struct Layer final : public ToCompound {
            int32_t height;
            BlockType block;

            Layer() = default;

            explicit Layer(const MinecraftRegistry &registry, const int32_t height, const std::string_view block):
                height(height), block(registry.blockType(block)) {}

            explicit Layer(const MinecraftRegistry &registry, const int32_t height):
                Layer(registry, height, "air") {}

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
                auto blockName = registry.blockName(block);
                prependMinecraftNamespace(&blockName);
                compound.put("block", blockName);
                compound.put("height", height);
            }

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
                std::string blockName;
                compound.read("block", blockName);
                compound.read("height", height);

                stripMinecraftNamespace(&blockName);
                block = registry.blockType(blockName);
            }
        };

        std::vector<Layer> layers;
        BiomeType biome;
        bool lakes{};
        bool features{};
        std::vector<std::string> structureOverrides;

        explicit FlatWorldGenerator(const MinecraftRegistry &registry): biome(registry.biomeType("plains")) {}

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
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

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
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
    };

    struct NoiseWorldGenerator final : public ToCompound {
        static constexpr std::string_view identifier = "minecraft:noise";

        struct BiomeSource final : public ToCompound {
            result::Option<std::string> preset;
            std::string type;

            BiomeSource() = default;

            explicit BiomeSource(std::string preset, std::string type):
                preset(std::move(preset)), type(std::move(type)) {}

            explicit BiomeSource(std::string type):
                type(std::move(type)) {}

            static auto overworld() -> BiomeSource {
                return BiomeSource{"minecraft:overworld", "minecraft:multi_noise"};
            }

            static auto nether() -> BiomeSource {
                return BiomeSource{"minecraft:nether", "minecraft:multi_noise"};
            }

            static auto end() -> BiomeSource {
                return BiomeSource{"minecraft:the_end"};
            }

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
                if (preset) compound.put("preset", *preset);
                compound.put("type", type);
            }

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
                if (compound.contains<std::string>("preset"))
                    compound.read("preset", *preset);

                compound.read("type", type);
            }
        };

        BiomeSource biomeSource{};
        std::string settings;

        explicit NoiseWorldGenerator(const DimensionType dimension) {
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

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.putNbt("biome_source", biomeSource.createCompound(registry));
            compound.put("settings", settings);
            compound.put("type", std::string{identifier});
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            const auto &biomeSourceNBT = compound.readNbt<NbtCompound>("biome_source");
            biomeSource.readCompound(registry, biomeSourceNBT);

            compound.read("settings", settings);
        }
    };

    struct WorldGenerator final : public ToCompound {

        DimensionType dimension{};

        WorldGenerator() = default;

        explicit WorldGenerator(const MinecraftRegistry &registry,
                                const WorldGeneratorType type, const DimensionType dimension):
            dimension(dimension), _type(type) {
            switch (type) {
                case WorldGeneratorType::DEBUG:
                    worldGeneratorPtr = std::make_shared<DebugWorldGenerator>();
                    break;
                case WorldGeneratorType::FLAT:
                    worldGeneratorPtr = std::make_shared<FlatWorldGenerator>(registry);
                    break;
                case WorldGeneratorType::NOISE:
                    worldGeneratorPtr = std::make_shared<NoiseWorldGenerator>(dimension);
                    break;
            }
        }

        [[nodiscard]]
        auto type() const -> WorldGeneratorType {
            return _type;
        }

        [[nodiscard]]
        auto asDebug() const -> const DebugWorldGenerator* {
            return dynamic_cast<const DebugWorldGenerator*>(worldGeneratorPtr.get());
        }

        [[nodiscard]]
        auto asDebug() -> DebugWorldGenerator* {
            return dynamic_cast<DebugWorldGenerator*>(worldGeneratorPtr.get());
        }

        [[nodiscard]]
        auto asFlat() const -> const FlatWorldGenerator* {
            return dynamic_cast<const FlatWorldGenerator*>(worldGeneratorPtr.get());
        }

        [[nodiscard]]
        auto asFlat() -> FlatWorldGenerator* {
            return dynamic_cast<FlatWorldGenerator*>(worldGeneratorPtr.get());
        }

        [[nodiscard]]
        auto asNoise() const -> const NoiseWorldGenerator* {
            return dynamic_cast<const NoiseWorldGenerator*>(worldGeneratorPtr.get());
        }

        [[nodiscard]]
        auto asNoise() -> NoiseWorldGenerator* {
            return dynamic_cast<NoiseWorldGenerator*>(worldGeneratorPtr.get());
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.putNbt("generator", worldGeneratorPtr->createCompound(registry));
            compound.put<std::string>("type", getNamespacedDimension(dimension));
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            std::string dimensionName;
            compound.read<std::string>("type", dimensionName);
            dimension = getDimensionType(dimensionName);

            const auto &generator = compound.readNbt<NbtCompound>("generator");
            const auto &generatorTypeStr = generator.read<std::string>("type");

            if (generatorTypeStr == DebugWorldGenerator::identifier)
                worldGeneratorPtr = std::make_shared<DebugWorldGenerator>();
            else if (generatorTypeStr == FlatWorldGenerator::identifier)
                worldGeneratorPtr = std::make_shared<FlatWorldGenerator>(registry);
            else if (generatorTypeStr == NoiseWorldGenerator::identifier)
                worldGeneratorPtr = std::make_shared<NoiseWorldGenerator>(dimension);
        }

    private:
        WorldGeneratorType _type;
        std::shared_ptr<ToCompound> worldGeneratorPtr;
    };

    struct WorldGeneratorSettings final : public ToCompound {
        bool bonusChest{};
        int64_t seed{};
        bool generateFeatures{true};
        absl::flat_hash_map<std::string, WorldGenerator> dimensions;

        WorldGeneratorSettings() = default;

        explicit WorldGeneratorSettings(const MinecraftRegistry &registry, const WorldGeneratorType type) {
            for (size_t i = 0; i < DIMENSION_NAMES.size(); i++) {
                const auto dimension = static_cast<DimensionType>(i);
                dimensions.emplace(getNamespacedDimension(dimension), WorldGenerator{registry, type, dimension});
            }
        }

        static WorldGeneratorSettings defaultWorld(const MinecraftRegistry &registry) {
            return WorldGeneratorSettings{registry, WorldGeneratorType::NOISE};
        }

        static WorldGeneratorSettings emptyWorld(const MinecraftRegistry &registry) {
            return WorldGeneratorSettings{registry, WorldGeneratorType::FLAT};
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.put("bonus_chest", bonusChest);
            compound.put("seed", seed);
            compound.put("generate_features", generateFeatures);

            NbtCompound dimensionsNBT;
            for (const auto &[dimension, generator] : dimensions)
                dimensionsNBT.putNbt(dimension, generator.createCompound(registry));

            compound.putNbt("dimensions", dimensionsNBT);
            std::cout << stringifyNbt(dimensionsNBT) << '\n';
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            compound.read("bonus_chest", bonusChest);
            compound.read("seed", seed);
            compound.read("generate_features", generateFeatures);

            const auto &dimensionsNBT = compound.readNbt<NbtCompound>("dimensions");
            for (const auto &dimensionName : dimensionsNBT.getKeys()) {
                const auto &generatorNBT = dimensionsNBT.readNbt<NbtCompound>(dimensionName);

                WorldGenerator generator;
                generator.readCompound(registry, generatorNBT);
                dimensions.insert({dimensionName, generator});
            }
        }
    };

}
