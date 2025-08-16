// ReSharper disable CppDeprecatedEntity
// fuck you openssl, it's not like it's my fault mojang still uses sha1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <mc_cpp/crypto/daft_sha1.hpp>
#include <openssl/types.h>
#include <openssl/bn.h>
#include <array>
#include <algorithm>

namespace mc {
    DaftSha1::DaftSha1() {
        SHA1_Init(&ctx);
    }

    auto DaftSha1::update(const mc::ByteBuf &in) -> void {
        SHA1_Update(&ctx, in.data(), in.size());
    }

    auto DaftSha1::update(const std::string &in) -> void {
        SHA1_Update(&ctx, in.data(), in.size());
    }

    // taken from https://gist.github.com/madmongo1/53c303c6fe8de64f93adc014c7671d51
    auto DaftSha1::digest() -> std::string {
        std::string result;

        auto buf = std::array<std::uint8_t, 20>{};
        SHA1_Final(buf.data(), &ctx);

        BIGNUM *bn = BN_bin2bn(buf.data(), buf.size(), nullptr);
        SHA1_Init(&ctx);

        if (BN_is_bit_set(bn, 159)) {
            result += '-';

            auto tmp = mc::ByteBuf(BN_num_bytes(bn));
            BN_bn2bin(bn, tmp.data());
            std::ranges::transform(tmp, tmp.begin(), [](const auto b) {
                return ~b;
            });
            BN_bin2bn(tmp.data(), static_cast<int>(tmp.size()), bn);
            BN_add_word(bn, 1);
        }
        const auto hex = BN_bn2hex(bn);

        std::string_view view = hex;
        while (!view.empty() && view[0] == '0')
            view = view.substr(1);

        result.append(view.begin(), view.end());
        OPENSSL_free(hex);
        BN_free(bn);

        std::ranges::transform(result, result.begin(), [](const auto c) {
            return std::tolower(c);
        });

        return result;
    }
}

#pragma GCC diagnostic pop
