#include <mc_cpp/anvil/level_dat/datapacks.hpp>

namespace mc {
    DataPacks::DataPacks() {
        dataPacks["vanilla"] = true;
    }

    auto DataPacks::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
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

    auto DataPacks::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        for (const auto &id : compound.readNbt<NbtList>("Enabled").values)
            dataPacks[get<std::string>(*id)] = true;

        for (const auto &id : compound.readNbt<NbtList>("Disabled").values)
            dataPacks[get<std::string>(*id)] = false;
    }
}
