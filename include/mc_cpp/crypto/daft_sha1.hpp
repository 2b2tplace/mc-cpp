#pragma once

#include <mc_cpp/types.hpp>
#include <openssl/sha.h>
#include <string>

namespace mc {
    class DaftSha1 {
        SHA_CTX ctx{};

    public:
        DaftSha1();

        DaftSha1(const DaftSha1&) = delete;

        DaftSha1(DaftSha1&&) = delete;

        auto operator=(const DaftSha1&) -> DaftSha1& = delete;

        auto operator=(DaftSha1&&) -> DaftSha1& = delete;

        auto update(const ByteBuf &in) -> void;

        auto update(const std::string &in) -> void;

        [[nodiscard]]
        auto digest() -> std::string;
    };
}
