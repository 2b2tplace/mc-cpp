#pragma once

#include <mc_cpp/nbt/to_compound.hpp>

namespace mc {

    struct DataPacks final : ToCompound {
        absl::flat_hash_map<std::string, bool> dataPacks;

        DataPacks() {
            dataPacks["vanilla"] = true;
        }

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            NbtList enabled{NbtType::STRING};
            NbtList disabled{NbtType::STRING};

            for (const auto &[id, isEnabled] : dataPacks) {
                if (isEnabled)
                    enabled.add(NbtString{id});
                else
                    disabled.add(NbtString{id});
            }
            compound.putNbt("Enabled", enabled);
            compound.putNbt("Disabled", disabled);
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            for (const auto &id : compound.readNbt<NbtList>("Enabled").values)
                dataPacks[get<std::string>(*id)] = true;

            for (const auto &id : compound.readNbt<NbtList>("Disabled").values)
                dataPacks[get<std::string>(*id)] = false;
        }
    };

}
