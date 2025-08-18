#pragma once

#include <mc_cpp/types.hpp>
#include <protocolCraft/Packets/Login/Serverbound/ServerboundKeyPacket.hpp>
#include <openssl/aes.h>
#include <openssl/types.h>

namespace mc {

    using mc::ByteBuf;

    class AESContext {
        EVP_CIPHER_CTX *encryptionCtx;
        EVP_CIPHER_CTX *decryptionCtx;
        uint32_t blocksize;

    public:
        ~AESContext();

        AESContext(const AESContext&) = delete;

        AESContext(AESContext&&) = delete;

        auto operator=(const AESContext&) -> AESContext& = delete;

        auto operator=(AESContext&&) -> AESContext& = delete;

        explicit AESContext(const ByteBuf &secret);

        [[nodiscard]]
        auto encrypt(const ByteBuf &in) const -> ByteBuf;

        [[nodiscard]]
        auto decrypt(const ByteBuf &in) const -> ByteBuf;

        [[nodiscard]]
        static auto initialize(EVP_PKEY *pubkey, ByteBuf &sharedSecret, const ByteBuf &verifyToken,
                               ProtocolCraft::ServerboundKeyPacket &packet) -> bool;
    };
}
