#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/anvil/level_dat/nbt_uuid.hpp>
#include <mc_cpp/anvil/level_dat/position_rotation.hpp>

namespace mc {

    enum GameType {
        SURVIVAL, CREATIVE, ADVENTURE, SPECTATOR
    };

    struct PlayerData final : ToCompound {

        struct Abilities final : ToCompound {
            bool flying{};
            float flySpeed{0.05f};
            bool instaBuild{};
            bool invulnerable{};
            bool mayBuild{true};
            bool mayFly{};
            float walkSpeed{0.1};

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
        };

        struct RecipeBook final : ToCompound {
            NbtList recipes{NbtType::STRING};
            NbtList toBeDisplayed{NbtType::STRING};
            bool isBlastingFurnaceFilteringCraftable;
            bool isBlastingFurnaceGuiOpen{};
            bool isFilteringCraftable{};
            bool isFurnaceFilteringCraftable{};
            bool isFurnaceGuiOpen{};
            bool isGuiOpen{};
            bool isSmokerFilteringCraftable{};
            bool isSmokerGuiOpen{};

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
        };

        int16_t air{300};
        NbtList data{NbtType::COMPOUND};
        double fallDistance{};
        int16_t fire{-20};
        bool hasVisualFire{};
        bool invulnerable{}; // why does this tag exist here AND in abilities? why mojang???
        Vec3d motion{};
        bool noGravity{};
        bool onGround{true};
        NbtList passengers{NbtType::COMPOUND};
        int32_t portalCooldown{};
        Vec3d position{};
        Vec2d rotation{};
        NbtList tags{NbtType::COMPOUND};
        result::Option<int32_t> ticksFrozen{};
        NbtUUID uuid{0, 0, 0, 0};

        float absorptionAmount{};
        result::Option<NbtList> activeEffects{}; // TODO implement potion effects properly (custom type + registries)
        NbtList attributes{NbtType::COMPOUND}; // TODO implement attributes properly (custom type + registries)
        NbtCompound brain{};
        int16_t deathTime{};
        bool fallFlying{};
        float health{20.0f};
        int32_t hurtByTimestamp{};
        int16_t hurtTime{};

        Abilities abilities{};
        int32_t dataVersion{};
        DimensionType dimension{};
        NbtList enderItems{NbtType::COMPOUND};
        float foodExhaustionLevel{};
        int32_t foodLevel{20};
        float foodSaturationLevel{5.0f};
        int32_t foodTickTimer{};
        NbtList inventory{NbtType::COMPOUND};
        result::Option<Location> lastDeathLocation{};
        GameType playerGameType;
        result::Option<GameType> previousPlayerGameType{};
        RecipeBook recipeBook{};
        result::Option<NbtCompound> rootVehicle{};
        int32_t score{};
        bool seenCredits{};
        int32_t selectedItemSlot{};
        result::Option<NbtCompound> shoulderEntityLeft{};
        result::Option<NbtCompound> shoulderEntityRight{};
        int16_t sleepTimer{};
        result::Option<LocationRotation> respawn{};
        int32_t xpLevel{};
        float xpPercentage{};
        int32_t xpSeed{};
        int32_t xpTotal{};

        PlayerData() = default;

        explicit PlayerData(const MinecraftRegistry &registry, GameType gameType);

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

}
