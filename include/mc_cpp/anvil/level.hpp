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

    struct Level final : ToCompound {
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

        explicit Level(std::string levelName, GameType gameType, const MinecraftRegistry &registry,
                       const std::function<auto(const MinecraftRegistry&) -> WorldGeneratorSettings> &generator = WorldGeneratorSettings::defaultWorld);

        static auto createStaticEmptyLevel(const std::string &levelName, const MinecraftRegistry &registry) -> Level;

        Level() = default;

        auto readCompound(const NbtCompound &compound) -> void;

        auto writeCompound(NbtCompound &compound) const -> void;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        [[nodiscard]]
        static auto read(const fs::path &path) -> Level;

        auto write(const fs::path &path) const -> void;
    };

}
