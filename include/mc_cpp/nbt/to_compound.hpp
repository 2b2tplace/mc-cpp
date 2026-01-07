#pragma once

#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace mc {

    struct ToCompound {
        virtual ~ToCompound() = default;

        [[nodiscard]]
        virtual auto createCompound(const MinecraftRegistry &registry) const -> NbtCompound {
            NbtCompound compound;
            writeCompound(registry, compound);
            return compound;
        }

        virtual auto writeCompound(const MinecraftRegistry &registry, NbtCompound &compound) const -> void = 0;

        virtual auto readCompound(const MinecraftRegistry &registry, const NbtCompound &compound) -> void = 0;
    };

}
