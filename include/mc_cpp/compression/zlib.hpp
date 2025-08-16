#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace mc {
    [[nodiscard]]
    auto compress(const std::vector<uint8_t> &uncompressed) -> std::vector<uint8_t>;

    [[nodiscard]]
    auto decompress(const std::vector<uint8_t> &compressed, size_t start) -> std::vector<uint8_t>;
}
