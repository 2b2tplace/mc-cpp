#pragma once

#include <mc_cpp/nbt/nbt.hpp>

namespace mc {

    struct NbtUUID {
        NbtIntArray uuid{};

        explicit NbtUUID(NbtIntArray array);

        explicit NbtUUID(int32_t u0, int32_t u1, int32_t u2, int32_t u3);
    };

}
