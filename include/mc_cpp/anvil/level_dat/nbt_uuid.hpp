#pragma once

#include <mc_cpp/nbt/nbt.hpp>

namespace mc {

    struct NbtUUID {
        NbtIntArray uuid{};

        explicit NbtUUID(NbtIntArray array);

        explicit NbtUUID(const int32_t u0, const int32_t u1, const int32_t u2, const int32_t u3);
    };

}
