#pragma once

#include <mc_cpp/nbt/to_compound.hpp>

namespace mc {

    struct DataPacks final : ToCompound {
        absl::flat_hash_map<std::string, bool> dataPacks;

        DataPacks();

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

}
