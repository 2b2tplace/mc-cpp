#include <mc_cpp/anvil/level_dat/nbt_uuid.hpp>

namespace mc {
    NbtUUID::NbtUUID(NbtIntArray array): uuid(std::move(array)) {}

    NbtUUID::NbtUUID(const int32_t u0, const int32_t u1, const int32_t u2, const int32_t u3) {
        uuid.value.resize(4);
        uuid.value[0] = u0;
        uuid.value[1] = u1;
        uuid.value[2] = u2;
        uuid.value[3] = u3;
    }
}
