#pragma once

#include <mc_cpp/anvil/level_dat/nbt_uuid.hpp>
#include <mc_cpp/nbt/to_compound.hpp>

namespace mc {

    struct CustomBossEvents final : ToCompound {

        enum Overlay {
            PROGRESS, NOTCHED_6, NOTCHED_10, NOTCHED_12, NOTCHED_20
        };

        static constexpr std::array OVERLAY_NAMES = {
            "progress",
            "notched_6",
            "notched_10",
            "notched_12",
            "notched_20",
        };

        struct Event final : ToCompound {
            std::vector<NbtUUID> playerUUIDs{};
            std::string color{"white"};
            bool createWorldFog{};
            bool darkenScreen{};
            int32_t maxHealth;
            int32_t currentHealth;
            std::string name;
            Overlay overlay{PROGRESS};
            bool playBossMusic{};
            bool visible{true};

            Event() = default;

            explicit Event(std::string name, const int32_t currentHealth, const int32_t maxHealth):
                maxHealth(maxHealth), currentHealth(currentHealth), name(std::move(name)) {}

            explicit Event(std::string name, const int32_t maxHealth):
                maxHealth(maxHealth), currentHealth(maxHealth), name(std::move(name)) {}

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
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

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
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
        };

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            for (const auto &[id, event] : bossEvents)
                compound.putNbt(id, event.createCompound(registry));
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            for (const auto &key : compound.getKeys()) {
                Event event;
                event.readCompound(registry, compound.readNbt<NbtCompound>(key));
                bossEvents[key] = event;
            }
        }

        absl::flat_hash_map<std::string, Event> bossEvents;
    };

}
