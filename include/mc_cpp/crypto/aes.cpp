#include <mc_cpp/crypto/aes.hpp>
#include <mc_cpp/crypto/rsa.hpp>
#include <openssl/rand.h>
#include <openssl/x509.h>

namespace mc {
    AESContext::~AESContext() {
        EVP_CIPHER_CTX_free(encryptionCtx);
        EVP_CIPHER_CTX_free(decryptionCtx);
    }

    AESContext::AESContext(const ByteBuf &secret) {
        const auto *cipher = EVP_aes_128_cfb8();
        encryptionCtx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(encryptionCtx, cipher, nullptr, secret.data(), secret.data());

        decryptionCtx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(decryptionCtx, cipher, nullptr, secret.data(), secret.data());

        blocksize = EVP_CIPHER_block_size(cipher);
    }

    auto AESContext::encrypt(const ByteBuf &in) const -> ByteBuf {
        ByteBuf out;
        auto size = 0;

        out.resize(in.size() + blocksize);
        EVP_EncryptUpdate(encryptionCtx, out.data(), &size, in.data(), static_cast<int>(in.size()));
        out.resize(size);

        return out;
    }

    auto AESContext::decrypt(const ByteBuf &in) const -> ByteBuf {
        ByteBuf out;
        auto size = 0;

        out.resize(in.size() + blocksize);
        EVP_DecryptUpdate(decryptionCtx, out.data(), &size, in.data(), static_cast<int>(in.size()));
        out.resize(size);

        return out;
    }

    auto AESContext::initialize(EVP_PKEY *pubkey, ByteBuf &sharedSecret, const ByteBuf &verifyToken,
                                ProtocolCraft::ServerboundKeyPacket &packet) -> bool {
        sharedSecret.resize(AES_BLOCK_SIZE);
        if (!RAND_bytes(sharedSecret.data(), static_cast<int>(sharedSecret.size())))
            return false;

        ByteBuf encryptedSharedSecret;
        if (!encryptRSA(pubkey, sharedSecret, encryptedSharedSecret))
            return false;

        ByteBuf encryptedVerifyToken;
        if (!encryptRSA(pubkey, verifyToken, encryptedVerifyToken))
            return false;

        packet.SetKeyBytes(encryptedSharedSecret);
        packet.SetEncryptedChallenge(encryptedVerifyToken);
        return true;
    }
}
