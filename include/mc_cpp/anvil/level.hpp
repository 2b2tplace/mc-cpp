#pragma once

#include <utility>
#include <mc_cpp/anvil/level_dat/custom_boss_events.hpp>
#include <mc_cpp/anvil/level_dat/datapacks.hpp>
#include <mc_cpp/anvil/level_dat/dragon_fight.hpp>
#include <mc_cpp/anvil/level_dat/gamerules.hpp>
#include <mc_cpp/anvil/level_dat/world_generator.hpp>
#include <mc_cpp/anvil/level_dat/player_data.hpp>

namespace mc {

    namespace fs = std::filesystem;

    enum Difficulty {
        PEACEFUL, EASY, NORMAL, HARD
    };

    struct Level final : public ToCompound {
        bool allowCommands{true};
        double borderCenterX{};
        double borderCenterZ{};
        double borderDamagePerBlock{0.2};
        double borderSize{60000000.0};
        double borderSafeZone{5.0};
        double borderSizeLerpTarget{60000000.0};
        int64_t borderSizeLerpTime{0};
        double borderWarningBlocks{5};
        double borderWarningTime{15};
        int32_t clearWeatherTime{};
        CustomBossEvents customBossEvents{};
        DataPacks dataPacks{};
        int32_t dataVersion{};
        int64_t dayTime{};
        Difficulty difficulty{HARD};
        bool difficultyLocked{};
        DragonFight dragonFight = DragonFight::defaultFight();
        GameRules gameRules{};
        WorldGeneratorSettings worldGeneratorSettings{};
        GameType gameType{SURVIVAL};
        bool hardcore{};
        bool initialized{true};
        time_t lastPlayed{};
        std::string levelName;
        bool mapFeatures{true};
        PlayerData player{};
        bool raining{};
        int32_t rainTime{};
        int64_t randomSeed{};
        int32_t spawnX{};
        int32_t spawnY{};
        int32_t spawnZ{};
        bool thundering{};
        int32_t thunderTime{};
        int64_t time{};
        bool wasModded{};

        explicit Level(std::string levelName, const GameType gameType, const MinecraftRegistry &registry,
                       const std::function<auto(const MinecraftRegistry&) -> WorldGeneratorSettings> &generator = WorldGeneratorSettings::defaultWorld):
            gameType(gameType), levelName(std::move(levelName)), player(registry, gameType) {
            dataVersion = registry.dataVersion();
            worldGeneratorSettings = generator(registry);
        }

        static auto createStaticEmptyLevel(const std::string &levelName, const MinecraftRegistry &registry) -> Level {
            Level level{levelName, SPECTATOR, registry, WorldGeneratorSettings::emptyWorld};
            level.dragonFight = DragonFight::finishedFight();
            level.gameRules = GameRules::staticGameRules();
            level.worldGeneratorSettings.generateFeatures = false;
            return level;
        }

        Level() = default;

        auto readCompound(const NbtCompound &compound) -> void {
            const auto &rootTag = compound.readNbt<NbtCompound>("Data");
            rootTag.read("DataVersion", dataVersion);
            const auto registry = getRegistry(dataVersion);
            if (!registry)
                throw std::runtime_error("Unsupported data version: " + std::to_string(dataVersion));

            readCompound(*registry, compound);
        }

        auto writeCompound(NbtCompound &compound) const -> void {
            const auto registry = getRegistry(dataVersion);
            if (!registry)
                throw std::runtime_error("Unsupported data version: " + std::to_string(dataVersion));

            writeCompound(*registry, compound);
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            const auto &rootTag = compound.readNbt<NbtCompound>("Data");
            rootTag.read("DataVersion", dataVersion);
            assert(dataVersion == registry.dataVersion());

            rootTag.read("allowCommands", allowCommands);
            rootTag.read("BorderCenterX", borderCenterX);
            rootTag.read("BorderCenterZ", borderCenterZ);
            rootTag.read("BorderDamagePerBlock", borderDamagePerBlock);
            rootTag.read("BorderSize", borderSize);
            rootTag.read("BorderSafeZone", borderSafeZone);
            rootTag.read("BorderSizeLerpTarget", borderSizeLerpTarget);
            rootTag.read("BorderSizeLerpTime", borderSizeLerpTime);
            rootTag.read("BorderWarningBlocks", borderWarningBlocks);
            rootTag.read("BorderWarningTime", borderWarningTime);
            rootTag.read("clearWeatherTime", clearWeatherTime);
            customBossEvents.readCompound(registry, rootTag.readNbt<NbtCompound>("CustomBossEvents"));
            dataPacks.readCompound(registry, rootTag.readNbt<NbtCompound>("DataPacks"));
            rootTag.read("DayTime", dayTime);
            difficulty = static_cast<Difficulty>(rootTag.read<int8_t>("Difficulty"));
            rootTag.read("DifficultyLocked", difficultyLocked);
            dragonFight.readCompound(registry, rootTag.readNbt<NbtCompound>("DragonFight"));
            gameRules.readCompound(registry, rootTag.readNbt<NbtCompound>("GameRules"));
            worldGeneratorSettings.readCompound(registry, rootTag.readNbt<NbtCompound>("WorldGenSettings"));
            gameType = static_cast<GameType>(rootTag.read<int32_t>("GameType"));
            rootTag.read("hardcore", hardcore);
            rootTag.read("initialized", initialized);
            rootTag.read("LastPlayed", lastPlayed);
            rootTag.read("LevelName", levelName);
            rootTag.read("MapFeatures", mapFeatures);
            player.readCompound(registry, rootTag.readNbt<NbtCompound>("Player"));
            rootTag.read("raining", raining);
            rootTag.read("rainTime", rainTime);
            rootTag.read("RandomSeed", randomSeed);
            rootTag.read("SpawnX", spawnX);
            rootTag.read("SpawnY", spawnY);
            rootTag.read("SpawnZ", spawnZ);
            rootTag.read("thundering", thundering);
            rootTag.read("thunderTime", thunderTime);
            rootTag.read("Time", time);
            rootTag.read("WasModded", wasModded);
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            assert(dataVersion == registry.dataVersion());
            NbtCompound rootTag;
            rootTag.put("allowCommands", allowCommands);
            rootTag.put("BorderCenterX", borderCenterX);
            rootTag.put("BorderCenterZ", borderCenterZ);
            rootTag.put("BorderDamagePerBlock", borderDamagePerBlock);
            rootTag.put("BorderSize", borderSize);
            rootTag.put("BorderSafeZone", borderSafeZone);
            rootTag.put("BorderSizeLerpTarget", borderSizeLerpTarget);
            rootTag.put("BorderSizeLerpTime", borderSizeLerpTime);
            rootTag.put("BorderWarningBlocks", borderWarningBlocks);
            rootTag.put("BorderWarningTime", borderWarningTime);
            rootTag.put("clearWeatherTime", clearWeatherTime);
            rootTag.putNbt("CustomBossEvents", customBossEvents.createCompound(registry));
            rootTag.putNbt("DataPacks", dataPacks.createCompound(registry));
            rootTag.put("DataVersion", dataVersion);
            rootTag.put("DayTime", dayTime);
            rootTag.put<int8_t>("Difficulty", difficulty);
            rootTag.put("DifficultyLocked", difficultyLocked);
            rootTag.putNbt("DragonFight", dragonFight.createCompound(registry));
            rootTag.putNbt("GameRules", gameRules.createCompound(registry));
            rootTag.putNbt("WorldGenSettings", worldGeneratorSettings.createCompound(registry));
            rootTag.put<int32_t>("GameType", gameType);
            rootTag.put("hardcore", hardcore);
            rootTag.put("initialized", initialized);
            rootTag.put("LastPlayed", lastPlayed);
            rootTag.put("LevelName", levelName);
            rootTag.put("MapFeatures", mapFeatures);
            rootTag.putNbt("Player", player.createCompound(registry));
            rootTag.put("raining", raining);
            rootTag.put("rainTime", rainTime);
            rootTag.put("RandomSeed", randomSeed);
            rootTag.put("SpawnX", spawnX);
            rootTag.put("SpawnY", spawnY);
            rootTag.put("SpawnZ", spawnZ);
            rootTag.put("thundering", thundering);
            rootTag.put("thunderTime", thunderTime);
            rootTag.put("Time", time);
            rootTag.put("version", 19133); // anvil file format

            NbtCompound version;
            version.put("Id", dataVersion);
            version.put("Name", std::string{registry.namedVersion()});
            version.put("Series", "main");
            version.put("Snapshot", false);

            rootTag.putNbt("Version", version);
            rootTag.put("WasModded", wasModded);

            compound.putNbt("Data", rootTag);
        }

        [[nodiscard]]
        static auto read(const fs::path &path) -> Level {
            NbtFile nbtFile{};
            std::ifstream in{path, std::ios::binary};
            nbtFile.readCompressed(in);
            in.close();

            Level level;
            level.readCompound(nbtFile);
            return level;
        }

        auto write(const fs::path &path) const -> void {
            NbtFile nbtFile{};
            writeCompound(nbtFile);
            std::ofstream out{path, std::ios::binary};
            nbtFile.writeNBT(out);
            out.close();
        }

    };

}
