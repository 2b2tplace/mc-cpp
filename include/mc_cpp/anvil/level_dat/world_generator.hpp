#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>

namespace mc {

    enum class WorldGeneratorType {
        DEBUG, FLAT, NOISE
    };

    struct DebugWorldGenerator final : ToCompound {
        static constexpr std::string_view identifier = "minecraft:debug";

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {}
    };

    struct FlatWorldGenerator final : ToCompound {
        static constexpr std::string_view identifier = "minecraft:flat";

        struct Layer final : ToCompound {
            int32_t height;
            BlockType block;

            Layer() = default;

            explicit Layer(const MinecraftRegistry &registry, const int32_t height, const std::string_view block);

            explicit Layer(const MinecraftRegistry &registry, const int32_t height);

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
        };

        std::vector<Layer> layers;
        BiomeType biome;
        bool lakes{};
        bool features{};
        std::vector<std::string> structureOverrides;

        explicit FlatWorldGenerator(const MinecraftRegistry &registry);

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

    struct NoiseWorldGenerator final : ToCompound {
        static constexpr std::string_view identifier = "minecraft:noise";

        struct BiomeSource final : ToCompound {
            result::Option<std::string> preset;
            std::string type;

            BiomeSource() = default;

            explicit BiomeSource(std::string preset, std::string type);

            explicit BiomeSource(std::string type);

            static auto overworld() -> BiomeSource ;

            static auto nether() -> BiomeSource ;

            static auto end() -> BiomeSource ;

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
        };

        BiomeSource biomeSource{};
        std::string settings;

        explicit NoiseWorldGenerator(const DimensionType dimension);

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

    struct WorldGenerator final : ToCompound {

        DimensionType dimension{};

        WorldGenerator() = default;

        explicit WorldGenerator(const MinecraftRegistry &registry,
                                const WorldGeneratorType type, const DimensionType dimension);

        [[nodiscard]]
        auto type() const -> WorldGeneratorType ;

        [[nodiscard]]
        auto asDebug() const -> const DebugWorldGenerator* ;

        [[nodiscard]]
        auto asDebug() -> DebugWorldGenerator* ;

        [[nodiscard]]
        auto asFlat() const -> const FlatWorldGenerator* ;

        [[nodiscard]]
        auto asFlat() -> FlatWorldGenerator* ;

        [[nodiscard]]
        auto asNoise() const -> const NoiseWorldGenerator* ;

        [[nodiscard]]
        auto asNoise() -> NoiseWorldGenerator* ;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;

    private:
        WorldGeneratorType _type;
        std::shared_ptr<ToCompound> worldGeneratorPtr;
    };

    struct WorldGeneratorSettings final : ToCompound {
        bool bonusChest{};
        int64_t seed{};
        bool generateFeatures{true};
        absl::flat_hash_map<std::string, WorldGenerator> dimensions;

        WorldGeneratorSettings() = default;

        explicit WorldGeneratorSettings(const MinecraftRegistry &registry, const WorldGeneratorType type);

        static WorldGeneratorSettings defaultWorld(const MinecraftRegistry &registry);

        static WorldGeneratorSettings emptyWorld(const MinecraftRegistry &registry);

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

}
