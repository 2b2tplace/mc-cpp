#include <mc_cpp/anvil/level_dat/custom_boss_events.hpp>

namespace mc {
    CustomBossEvents::Event::Event(std::string name, const int32_t currentHealth, const int32_t maxHealth):
        maxHealth(maxHealth), currentHealth(currentHealth), name(std::move(name)) {}

    CustomBossEvents::Event::Event(std::string name, const int32_t maxHealth):
        maxHealth(maxHealth), currentHealth(maxHealth), name(std::move(name)) {}

    auto CustomBossEvents::Event::
    writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        NbtList playerUUIDsNBT{NbtType::INT_ARRAY};
        for (const auto &uuid : playerUUIDs)
            playerUUIDsNBT.add(uuid.uuid);

        compound.putNbt("Players", playerUUIDsNBT);
        compound.put("Color", color);
        compound.put("CreateWorldFog", createWorldFog);
        compound.put("DarkenScreen", darkenScreen);
        compound.put("Max", maxHealth);
        compound.put("Value", currentHealth);
        compound.put("Name", name);
        compound.put("Overlay", OVERLAY_NAMES[overlay]);
        compound.put("PlayBossMusic", playBossMusic);
        compound.put("Visible", visible);
    }

    auto CustomBossEvents::Event::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        const auto &playerUUIDsNBT = compound.readNbt<NbtList>("Players");
        for (const auto &uuidNBT :  playerUUIDsNBT.values)
            playerUUIDs.emplace_back(getAsTag<NbtIntArray>(*uuidNBT));

        compound.read("Color", color);
        compound.read("CreateWorldFog", createWorldFog);
        compound.read("DarkenScreen", darkenScreen);
        compound.read("Max", maxHealth);
        compound.read("Value", currentHealth);
        compound.read("Name", name);

        std::string overlayName;
        compound.read("Overlay", overlayName);
        for (size_t i = 0; i < OVERLAY_NAMES.size(); i++) {
            if (OVERLAY_NAMES[i] == overlayName)
                overlay = static_cast<Overlay>(i);
        }
        compound.read("PlayBossMusic", playBossMusic);
        compound.read("Visible", visible);
    }

    auto CustomBossEvents::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        for (const auto &[id, event] : bossEvents)
            compound.putNbt(id, event.createCompound(registry));
    }

    auto CustomBossEvents::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        for (const auto &key : compound.getKeys()) {
            Event event;
            event.readCompound(registry, compound.readNbt<NbtCompound>(key));
            bossEvents[key] = event;
        }
    }
}
