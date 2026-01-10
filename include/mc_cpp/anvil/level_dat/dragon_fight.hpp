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
        static auto defaultFight() -> DragonFight ;

        [[nodiscard]]
        static auto finishedFight() -> DragonFight ;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

}
