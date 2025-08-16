#include <mc_cpp/crypto/rsa.hpp>
#include <openssl/pem.h>
#include <iostream>

namespace mc {
    auto generateRSAKeyPair(const size_t bits) -> EVP_PKEY * {
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx)
            return nullptr;

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return nullptr;
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, static_cast<int>(bits)) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return nullptr;
        }
        EVP_PKEY *pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return nullptr;
        }
        EVP_PKEY_CTX_free(ctx);
        return pkey;
    }

    auto loadRSAPrivateKey(const std::filesystem::path &filepath) -> EVP_PKEY * {
        auto *fp = fopen(filepath.c_str(), "rb");
        if (!fp)
            return nullptr;

        auto *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
        fclose(fp);

        if (!pkey)
            return nullptr;

        return pkey;
    }

    auto decryptRSA(EVP_PKEY *privkey, const std::vector<uint8_t> &ciphertext, std::vector<uint8_t> &buf) -> bool {
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privkey, nullptr);
        if (!ctx) return false;

        if (EVP_PKEY_decrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        size_t outlen = 0;
        if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, ciphertext.data(), ciphertext.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        buf.resize(outlen, 0);
        if (EVP_PKEY_decrypt(ctx, buf.data(), &outlen, ciphertext.data(), ciphertext.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        buf.resize(outlen);
        EVP_PKEY_CTX_free(ctx);
        return true;
    }

    auto encryptRSA(EVP_PKEY *pubkey, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &buf) -> bool {
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pubkey, nullptr);
        if (!ctx) return false;

        if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        size_t outlen = 0;
        if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, plaintext.data(), plaintext.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        buf.resize(outlen, 0);
        if (EVP_PKEY_encrypt(ctx, &buf[0], &outlen, plaintext.data(), plaintext.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return "";
        }
        buf.resize(outlen);
        EVP_PKEY_CTX_free(ctx);
        return true;
    }

    auto dumpPublicKey(const EVP_PKEY *pkey, std::vector<uint8_t> &buf) -> bool {
        const auto len = i2d_PUBKEY(pkey, nullptr);
        if (len <= 0) return false;

        buf.resize(len);
        auto *p = buf.data();

        return i2d_PUBKEY(pkey, &p) == len;
    }
}
