#pragma once

#include <vector>
#include <cstdint>

namespace mc {
    using ByteBuf = std::vector<uint8_t>;
    using ReadIter = ByteBuf::const_iterator;
}
