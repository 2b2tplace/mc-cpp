#pragma once

#include <openssl/rsa.h>
#include <filesystem>
#include <vector>

namespace mc {
    [[nodiscard]]
    auto generateRSAKeyPair(size_t bits = 1024) -> EVP_PKEY *;

    [[nodiscard]]
    auto loadRSAPrivateKey(const std::filesystem::path &filepath) -> EVP_PKEY *;

    [[nodiscard]]
    auto decryptRSA(EVP_PKEY *privkey, const std::vector<uint8_t> &ciphertext, std::vector<uint8_t> &buf) -> bool;

    [[nodiscard]]
    auto encryptRSA(EVP_PKEY *pubkey, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &buf) -> bool;

    [[nodiscard]]
    auto dumpPublicKey(const EVP_PKEY *pkey, std::vector<uint8_t> &buf) -> bool;
}
