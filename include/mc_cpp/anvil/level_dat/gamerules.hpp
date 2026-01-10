#pragma once

#include <mc_cpp/nbt/to_compound.hpp>

namespace mc {

    struct GameRules final : ToCompound {
        bool doFireTick{true};
        bool mobGriefing{true};
        bool keepInventory{};
        bool doMobSpawning{true};
        bool doMobLoot{true};
        bool projectilesCanBreakBlocks{true};
        bool doTileDrops{true};
        bool doEntityDrops{true};
        bool commandBlockOutput{true};
        bool naturalRegeneration{true};
        bool doDaylightCycle{true};
        bool logAdminCommands{true};
        bool showDeathMessages{true};
        int32_t randomTickSpeed{3};
        bool sendCommandFeedback{true};
        bool reducedDebugInfo{};
        bool spectatorsGenerateChunks{true};
        int32_t spawnRadius{10};
        bool disablePlayerMovementCheck{};
        bool disableElytraMovementCheck{};
        int32_t maxEntityCramming{24};
        bool doWeatherCycle{true};
        bool doLimitedCrafting{};
        int32_t maxCommandChainLength{65536};
        int32_t maxCommandForkCount{65536};
        int32_t commandModificationBlockLimit{32768};
        bool announceAdvancements{true};
        bool disableRaids{};
        bool doInsomnia{true};
        bool doImmediateRespawn{};
        int32_t playersNetherPortalDefaultDelay{80};
        int32_t playersNetherPortalCreativeDelay{0};
        bool drowningDamage{true};
        bool fallDamage{true};
        bool fireDamage{true};
        bool freezeDamage{true};
        bool doPatrolSpawning{true};
        bool doTraderSpawning{true};
        bool doWardenSpawning{true};
        bool forgiveDeadPlayers{true};
        bool universalAnger{};
        int32_t playersSleepingPercentage{100};
        bool blockExplosionDropDecay{true};
        bool mobExplosionDropDecay{true};
        bool tntExplosionDropDecay{};
        int32_t snowAccumulationHeight{1};
        bool waterSourceConversion{true};
        bool lavaSourceConversion{};
        bool globalSoundEvents{true};
        bool doVinesSpread{true};
        bool enderPearlsVanishOnDeath{true};
        int32_t minecartMaxSpeed{8};
        int32_t spawnChunkRadius{2};

        static auto staticGameRules() -> GameRules;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };
    
}
