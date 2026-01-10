#include <mc_cpp/anvil/level_dat/position_rotation.hpp>

namespace mc {
    auto Location::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void {
        compound.put("dimension", getNamespacedDimension(dimension));
        compound.putNbt("pos", NbtIntArray{std::vector{x, y, z}});
    }

    auto Location::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void {
        std::string dimensionName;
        compound.read("dimension", dimensionName);
        dimension = getDimensionType(dimensionName);
        const auto &pos = compound.readNbt<NbtIntArray>("pos");

        if (pos.value.size() == 3) {
            x = pos.value[0];
            y = pos.value[1];
            z = pos.value[2];
        }
    }

    void LocationRotation::writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const {
        Location::writeCompound(registry, compound);
        compound.put("yaw", yaw);
        compound.put("pitch", pitch);
        compound.put("forced", forced);
    }

    void LocationRotation::readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) {
        Location::readCompound(registry, compound);
        compound.read("yaw", yaw);
        compound.read("pitch", pitch);
        compound.read("forced", forced);
    }

    auto Vec3d::createNbtList() const -> NbtList {
        NbtList vec{NbtType::DOUBLE};
        vec.add(NbtDouble{x});
        vec.add(NbtDouble{y});
        vec.add(NbtDouble{z});
        return vec;
    }

    auto Vec3d::readNbtList(const NbtList &list) -> void {
        if (list.length() == 3 && list.getHeldType() == NbtType::DOUBLE) {
            x = get<double>(*list.values[0]);
            y = get<double>(*list.values[1]);
            z = get<double>(*list.values[2]);
        }
    }

    auto Vec2d::createNbtList() const -> NbtList {
        NbtList vec{NbtType::FLOAT};
        vec.add(NbtFloat{yaw});
        vec.add(NbtFloat{pitch});
        return vec;
    }

    auto Vec2d::readNbtList(const NbtList &list) -> void {
        if (list.length() == 2 && list.getHeldType() == NbtType::DOUBLE) {
            yaw = get<double>(*list.values[0]);
            pitch = get<double>(*list.values[1]);
        }
    }
}
