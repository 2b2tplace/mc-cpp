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

            explicit Event(std::string name, int32_t currentHealth, int32_t maxHealth);

            explicit Event(std::string name, int32_t maxHealth);

            auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

            auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
        };

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;

        absl::flat_hash_map<std::string, Event> bossEvents;
    };

}
