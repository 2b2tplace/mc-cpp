#include <mc_cpp/anvil/level_dat/dragon_fight.hpp>

namespace mc {
    auto DragonFight::defaultFight() -> DragonFight {
        DragonFight fight;
        fight.gateways.value.resize(20);
        for (size_t i = 0; i < fight.gateways.value.size(); i++)
            fight.gateways.value[i] = i;

        fight.exitPortalLocation.value = {0, 62, 0};
        return fight;
    }

    auto DragonFight::finishedFight() -> DragonFight {
        DragonFight fight;
        fight.dragonKilled = true;
        fight.needsStateScanning = false;
        fight.previouslyKilled = true;
        fight.exitPortalLocation.value = {0, 62, 0};
        return fight;
    }

    auto DragonFight::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        if (dragonUUID)
            compound.putNbt("Dragon", dragonUUID->uuid);

        compound.put("DragonKilled", dragonKilled);
        compound.putNbt("ExitPortalLocation", exitPortalLocation);
        compound.putNbt("Gateways", gateways);
        compound.put("NeedsStateScanning", needsStateScanning);
        compound.put("PreviouslyKilled", previouslyKilled);
    }

    auto DragonFight::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        if (compound.containsNbt<NbtIntArray>("Dragon"))
            compound.readNbt<NbtIntArray>("Dragon", dragonUUID->uuid);

        compound.read("DragonKilled", dragonKilled);
        compound.readNbt("ExitPortalLocation", exitPortalLocation);
        compound.readNbt("Gateways", gateways);
        compound.read("NeedsStateScanning", needsStateScanning);
        compound.read("PreviouslyKilled", previouslyKilled);
    }
}
