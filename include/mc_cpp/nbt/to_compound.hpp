#pragma once

#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace mc {

    struct ToCompound {
        virtual ~ToCompound() = default;

        [[nodiscard]]
        virtual auto createCompound(const MinecraftRegistry &registry) const -> NbtCompound = 0;
    };

}
