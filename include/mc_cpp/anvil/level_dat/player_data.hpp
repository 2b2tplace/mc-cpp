#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/anvil/level_dat/nbt_uuid.hpp>
#include <mc_cpp/anvil/level_dat/position_rotation.hpp>

namespace mc {

    enum GameType {
        SURVIVAL, CREATIVE, ADVENTURE, SPECTATOR
    };

    struct PlayerData final : public ToCompound {

        struct Abilities final : public ToCompound {
            bool flying{};
            float flySpeed{0.05f};
            bool instaBuild{};
            bool invulnerable{};
            bool mayBuild{true};
            bool mayFly{};
            float walkSpeed{0.1};

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
                compound.put("flying", flying);
                compound.put("flySpeed", flySpeed);
                compound.put("instabuild", instaBuild); // intended, not a typo (why mojang)
                compound.put("invulnerable", invulnerable);
                compound.put("mayBuild", mayBuild);
                compound.put("mayfly", mayFly); // intended, not a typo (why mojang)
                compound.put("walkSpeed", walkSpeed);
            }

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
                compound.read("flying", flying);
                compound.read("flySpeed", flySpeed);
                compound.read("instabuild", instaBuild); // intended, not a typo (why mojang)
                compound.read("invulnerable", invulnerable);
                compound.read("mayBuild", mayBuild);
                compound.read("mayfly", mayFly); // intended, not a typo (why mojang)
                compound.read("walkSpeed", walkSpeed);
            }
        };

        struct RecipeBook final : public ToCompound {
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

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
                compound.putNbt("recipes", recipes);
                compound.putNbt("toBeDisplayed", toBeDisplayed);
                compound.put("isBlastingFurnaceFilteringCraftable", isBlastingFurnaceFilteringCraftable);
                compound.put("isBlastingFurnaceGuiOpen", isBlastingFurnaceGuiOpen);
                compound.put("isFilteringCraftable", isFilteringCraftable);
                compound.put("isFurnaceFilteringCraftable", isFurnaceFilteringCraftable);
                compound.put("isFurnaceGuiOpen", isFurnaceGuiOpen);
                compound.put("isGuiOpen", isGuiOpen);
                compound.put("isSmokerFilteringCraftable", isSmokerFilteringCraftable);
                compound.put("isSmokerGuiOpen", isSmokerGuiOpen);
            }

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
                compound.readNbt("recipes", recipes);
                compound.readNbt("toBeDisplayed", toBeDisplayed);
                compound.read("isBlastingFurnaceFilteringCraftable", isBlastingFurnaceFilteringCraftable);
                compound.read("isBlastingFurnaceGuiOpen", isBlastingFurnaceGuiOpen);
                compound.read("isFilteringCraftable", isFilteringCraftable);
                compound.read("isFurnaceFilteringCraftable", isFurnaceFilteringCraftable);
                compound.read("isFurnaceGuiOpen", isFurnaceGuiOpen);
                compound.read("isGuiOpen", isGuiOpen);
                compound.read("isSmokerFilteringCraftable", isSmokerFilteringCraftable);
                compound.read("isSmokerGuiOpen", isSmokerGuiOpen);
            }
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

        explicit PlayerData(const MinecraftRegistry &registry, const GameType gameType):
            playerGameType(gameType) {
            dataVersion = registry.dataVersion();

            NbtCompound entityInteractionRange;
            entityInteractionRange.put("base", 3.0);
            entityInteractionRange.put("id", "minecraft:entity_interaction_range");

            NbtCompound movementSpeed;
            movementSpeed.put("base", 0.1);
            movementSpeed.put("id", "minecraft:movement_speed");

            NbtCompound blockInteractionRange;
            blockInteractionRange.put("base", 4.5);
            blockInteractionRange.put("id", "minecraft:block_interaction_range");

            attributes.add(entityInteractionRange);
            attributes.add(movementSpeed);
            attributes.add(blockInteractionRange);

            brain.putNbt("memories", NbtCompound{});
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.put("Air", air);
            compound.putNbt("data", data);
            compound.put("fall_distance", fallDistance);
            compound.put("Fire", fire);
            compound.put("HasVisualFire", hasVisualFire);
            compound.put("Invulnerable", invulnerable);
            compound.putNbt("Motion", motion.createNbtList());
            compound.put("NoGravity", noGravity);
            compound.put("OnGround", onGround);
            compound.putNbt("Passengers", passengers);
            compound.put("PortalCooldown", portalCooldown);
            compound.putNbt("Pos", position.createNbtList());
            compound.putNbt("Rotation", rotation.createNbtList());
            compound.putNbt("Tags", tags);

            if (ticksFrozen)
                compound.put("TicksFrozen", *ticksFrozen);

            compound.putNbt("UUID", uuid.uuid);
            compound.put("AbsorptionAmount", absorptionAmount);

            if (activeEffects)
                compound.putNbt("active_effects", *activeEffects);

            compound.putNbt("attributes", attributes);
            compound.putNbt("Brain", brain);
            compound.put("DeathTime", deathTime);
            compound.put("FallFlying", fallFlying);
            compound.put("Health", health);
            compound.put("HurtByTimestamp", hurtByTimestamp);
            compound.put("HurtTime", hurtTime);

            compound.putNbt("abilities", abilities.createCompound(registry));
            compound.put("DataVersion", dataVersion);
            compound.put("Dimension", getNamespacedDimension(dimension));
            compound.putNbt("EnderItems", enderItems);
            compound.put("foodExhaustionLevel", foodExhaustionLevel);
            compound.put("foodLevel", foodLevel);
            compound.put("foodSaturationLevel", foodSaturationLevel);
            compound.put("foodTickTimer", foodTickTimer);
            compound.putNbt("Inventory", inventory);

            if (lastDeathLocation)
                compound.putNbt("LastDeathLocation", lastDeathLocation->createCompound(registry));

            compound.put<int32_t>("playerGameType", playerGameType);

            if (previousPlayerGameType)
                compound.put<int32_t>("previousPlayerGameType", *previousPlayerGameType);

            compound.putNbt("recipeBook", recipeBook.createCompound(registry));

            if (rootVehicle)
                compound.putNbt("RootVehicle", *rootVehicle);

            compound.put("Score", score);
            compound.put("seenCredits", seenCredits);
            compound.put("SelectedItemSlot", selectedItemSlot);

            if (shoulderEntityLeft)
                compound.putNbt("ShoulderEntityLeft", *shoulderEntityLeft);

            if (shoulderEntityRight)
                compound.putNbt("ShoulderEntityRight", *shoulderEntityRight);

            compound.put("SleepTimer", sleepTimer);

            if (respawn)
                compound.putNbt("respawn", respawn->createCompound(registry));

            compound.put("XpLevel", xpLevel);
            compound.put("XpP", xpPercentage);
            compound.put("XpSeed", xpSeed);
            compound.put("XpTotal", xpTotal);
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            compound.read("Air", air);
            compound.readNbt("data", data);
            compound.read("fall_distance", fallDistance);
            compound.read("Fire", fire);
            compound.read("HasVisualFire", hasVisualFire);
            compound.read("Invulnerable", invulnerable);

            motion.readNbtList(compound.readNbt<NbtList>("Motion"));
            compound.read("NoGravity", noGravity);
            compound.read("OnGround", onGround);
            compound.readNbt("Passengers", passengers);
            compound.read("PortalCooldown", portalCooldown);

            position.readNbtList(compound.readNbt<NbtList>("Pos"));
            rotation.readNbtList(compound.readNbt<NbtList>("Rotation"));

            compound.readNbt("Tags", tags);

            if (compound.contains<int32_t>("TicksFrozen"))
                compound.read("TicksFrozen", *ticksFrozen);

            compound.readNbt("UUID", uuid.uuid);

            compound.read("AbsorptionAmount", absorptionAmount);

            if (compound.containsNbt<NbtList>("active_effects"))
                compound.readNbt("active_effects", *activeEffects);

            compound.readNbt("attributes", attributes);
            compound.readNbt("Brain", brain);
            compound.read("DeathTime", deathTime);
            compound.read("FallFlying", fallFlying);
            compound.read("Health", health);
            compound.read("HurtByTimestamp", hurtByTimestamp);
            compound.read("HurtTime", hurtTime);

            abilities.readCompound(registry, compound.readNbt<NbtCompound>("abilities"));
            compound.read("DataVersion", dataVersion);

            std::string dimensionName;
            compound.read("Dimension", dimensionName);
            dimension = getDimensionType(dimensionName);

            compound.readNbt("EnderItems", enderItems);
            compound.read("foodExhaustionLevel", foodExhaustionLevel);
            compound.read("foodLevel", foodLevel);
            compound.read("foodSaturationLevel", foodSaturationLevel);
            compound.read("foodTickTimer", foodTickTimer);
            compound.readNbt("Inventory", inventory);
            if (compound.containsNbt<NbtCompound>("LastDeathLocation"))
                lastDeathLocation->readCompound(registry, compound.readNbt<NbtCompound>("LastDeathLocation"));

            playerGameType = static_cast<GameType>(compound.read<int32_t>("playerGameType"));
            if (compound.contains<int32_t>("previousPlayerGameType"))
                *previousPlayerGameType = static_cast<GameType>(compound.read<int32_t>("previousPlayerGameType"));

            recipeBook.readCompound(registry, compound.readNbt<NbtCompound>("recipeBook"));
            if (compound.containsNbt<NbtCompound>("RootVehicle"))
                compound.readNbt("RootVehicle", *rootVehicle);

            compound.read("Score", score);
            compound.read("seenCredits", seenCredits);
            compound.read("SelectedItemSlot", selectedItemSlot);

            if (compound.containsNbt<NbtCompound>("ShoulderEntityLeft"))
                compound.readNbt("ShoulderEntityLeft", *shoulderEntityLeft);

            if (compound.containsNbt<NbtCompound>("ShoulderEntityRight"))
                compound.readNbt("ShoulderEntityRight", *shoulderEntityRight);

            compound.read("SleepTimer", sleepTimer);

            if (compound.containsNbt<NbtCompound>("respawn"))
                respawn->readCompound(registry, compound.readNbt<NbtCompound>("respawn"));

            compound.read("XpLevel", xpLevel);
            compound.read("XpP", xpPercentage);
            compound.read("XpSeed", xpSeed);
            compound.read("XpTotal", xpTotal);
        }

    };

}
