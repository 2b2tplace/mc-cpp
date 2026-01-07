#pragma once

#include <mc_cpp/anvil/level_dat/nbt_uuid.hpp>
#include <mc_cpp/nbt/to_compound.hpp>

namespace mc {

    struct DragonFight final : ToCompound {
        result::Option<NbtUUID> dragonUUID;
        bool dragonKilled{};
        NbtIntArray exitPortalLocation{};
        NbtIntArray gateways{};
        bool needsStateScanning{true};
        bool previouslyKilled{};

        [[nodiscard]]
        static auto defaultFight() -> DragonFight {
            DragonFight fight;
            fight.gateways.value.resize(20);
            for (size_t i = 0; i < fight.gateways.value.size(); i++)
                fight.gateways.value[i] = i;

            fight.exitPortalLocation.value = {0, 62, 0};
            return fight;
        }

        [[nodiscard]]
        static auto finishedFight() -> DragonFight {
            DragonFight fight;
            fight.dragonKilled = true;
            fight.needsStateScanning = false;
            fight.previouslyKilled = true;
            fight.exitPortalLocation.value = {0, 62, 0};
            return fight;
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            if (dragonUUID)
                compound.putNbt("Dragon", dragonUUID->uuid);

            compound.put("DragonKilled", dragonKilled);
            compound.putNbt("ExitPortalLocation", exitPortalLocation);
            compound.putNbt("Gateways", gateways);
            compound.put("NeedsStateScanning", needsStateScanning);
            compound.put("PreviouslyKilled", previouslyKilled);
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            if (compound.containsNbt<NbtIntArray>("Dragon"))
                compound.readNbt<NbtIntArray>("Dragon", dragonUUID->uuid);

            compound.read("DragonKilled", dragonKilled);
            compound.readNbt("ExitPortalLocation", exitPortalLocation);
            compound.readNbt("Gateways", gateways);
            compound.read("NeedsStateScanning", needsStateScanning);
            compound.read("PreviouslyKilled", previouslyKilled);
        }

    };

}
