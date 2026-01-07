#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>

namespace mc {

    struct Location : ToCompound {
        DimensionType dimension;
        int32_t x;
        int32_t y;
        int32_t z;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            compound.put("dimension", getNamespacedDimension(dimension));
            compound.putNbt("pos", NbtIntArray{std::vector{x, y, z}});
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
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
    };

    struct LocationRotation final : Location {

        float yaw{};
        float pitch{};
        bool forced{};

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override {
            Location::writeCompound(registry, compound);
            compound.put("yaw", yaw);
            compound.put("pitch", pitch);
            compound.put("forced", forced);
        }

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override {
            Location::readCompound(registry, compound);
            compound.read("yaw", yaw);
            compound.read("pitch", pitch);
            compound.read("forced", forced);
        }
    };

    struct Vec3d {
        double x{};
        double y{};
        double z{};

        [[nodiscard]]
        auto createNbtList() const -> NbtList {
            NbtList vec{NbtType::DOUBLE};
            vec.add(NbtDouble{x});
            vec.add(NbtDouble{y});
            vec.add(NbtDouble{z});
            return vec;
        }

        auto readNbtList(const NbtList &list) -> void {
            if (list.length() == 3 && list.getHeldType() == NbtType::DOUBLE) {
                x = get<double>(*list.values[0]);
                y = get<double>(*list.values[1]);
                z = get<double>(*list.values[2]);
            }
        }
    };

    struct Vec2d {
        float yaw{};
        float pitch{};

        [[nodiscard]]
        auto createNbtList() const -> NbtList {
            NbtList vec{NbtType::FLOAT};
            vec.add(NbtFloat{yaw});
            vec.add(NbtFloat{pitch});
            return vec;
        }

        auto readNbtList(const NbtList &list) -> void {
            if (list.length() == 2 && list.getHeldType() == NbtType::DOUBLE) {
                yaw = get<double>(*list.values[0]);
                pitch = get<double>(*list.values[1]);
            }
        }
    };

}
