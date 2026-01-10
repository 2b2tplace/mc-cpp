#pragma once

#include <mc_cpp/nbt/to_compound.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>

namespace mc {

    struct Location : ToCompound {
        DimensionType dimension;
        int32_t x;
        int32_t y;
        int32_t z;

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

    struct LocationRotation final : Location {

        float yaw{};
        float pitch{};
        bool forced{};

        auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void override;

        auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void override;
    };

    struct Vec3d {
        double x{};
        double y{};
        double z{};

        [[nodiscard]]
        auto createNbtList() const -> NbtList;

        auto readNbtList(const NbtList &list) -> void;
    };

    struct Vec2d {
        float yaw{};
        float pitch{};

        [[nodiscard]]
        auto createNbtList() const -> NbtList;

        auto readNbtList(const NbtList &list) -> void;
    };

}
