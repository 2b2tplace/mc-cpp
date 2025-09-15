#pragma once

#include <mc_cpp/common/mutex.hpp>
#include <mc_cpp/logger.hpp>
#include <mc_cpp/protocol.hpp>
#include <mc_cpp/compression/zlib.hpp>
#include <mc_cpp/crypto/rsa.hpp>
#include <mc_cpp/crypto/aes.hpp>
#include <mc_cpp/tcp/tcpsocket.hpp>
#include <mc_cpp/tcp/tcptraffic.hpp>

namespace mc {

    template<typename Packet>
    using PacketHandler = std::function<bool(const Packet&)>;

    using BufferHandler = std::function<bool(ReadIter&, size_t&)>;
    using DefaultHandler = std::function<void(int32_t, const ByteBuf&, ReadIter&, size_t&)>;

    template<typename PacketType, typename State>
    class PacketListener {
        std::unordered_map<int32_t, BufferHandler> handlers;
        std::optional<DefaultHandler> defaultHandler;

    public:
        auto handlePacket(const ByteBuf &packetBuffer) const -> void {
            auto it = packetBuffer.cbegin();
            auto len = packetBuffer.size();

            const auto packetID = ProtocolCraft::ReadData<ProtocolCraft::VarInt>(it, len);
            handlePacket(packetID, packetBuffer, it, len);
        }

        auto handlePacket(const int packetID, const ByteBuf &packetBuffer, ReadIter &it, size_t &len) const -> void {
            if (handlers.contains(packetID) && !handlers.at(packetID)(it, len))
                return;

            if (defaultHandler)
                (*defaultHandler)(packetID, packetBuffer, it, len);
        }

        template<typename Packet>
        auto accept(const PacketHandler<Packet> &packetHandler) -> PacketListener& {
            const auto packetID = PacketType::template get<State, Packet>;
            const BufferHandler handler = [packetHandler](ReadIter &it, size_t &len) {
                Packet packet;
                packet.Read(it, len);
                return packetHandler(packet);
            };
            return accept(packetID, handler);
        }

        auto accept(const int packetID, const BufferHandler &bufferHandler) -> PacketListener& {
            handlers.insert({packetID, bufferHandler});
            return *this;
        }

        auto acceptDefault(const DefaultHandler &handler) -> PacketListener& {
            defaultHandler = handler;
            return *this;
        }
    };

    template<typename PacketType>
    class ConnectionListener {
        static constexpr auto defaultState = state::StateType::HANDSHAKE;

    public:
        state::StateType connectionState{defaultState};

        PacketListener<PacketType, state::Handshake> handshake;
        PacketListener<PacketType, state::Status> status;
        PacketListener<PacketType, state::Login> login;
        PacketListener<PacketType, state::Play> play;
        PacketListener<PacketType, state::Config> config;

        auto handlePacket(const ByteBuf &packetBuffer) const -> void {
            handlePacketWithArgs(packetBuffer);
        }

        auto handlePacket(const int packetID, ReadIter &it, size_t &len) const -> void {
            handlePacketWithArgs(packetID, it, len);
        }

        auto resetState() -> void {
            connectionState = defaultState;
        }

    private:
        template<typename... Args>
        auto handlePacketWithArgs(Args &&... args) const -> void {
            switch (connectionState) {
                case state::StateType::HANDSHAKE:
                    handshake.handlePacket(args...);
                    break;
                case state::StateType::STATUS:
                    status.handlePacket(args...);
                    break;
                case state::StateType::LOGIN:
                    login.handlePacket(args...);
                    break;
                case state::StateType::PLAY:
                    play.handlePacket(args...);
                    break;
                case state::StateType::CONFIG:
                    config.handlePacket(args...);
                    break;
            }
        }
    };

    template<typename Side, typename Listener = ConnectionListener<PacketType<Side>>>
    class TCPConnection {
    public:
        Logger &logger;
        Listener listener;
        std::shared_ptr<TCPSocket<>> socket;
        Mutex<ByteBuf> packetBuilder;
        int32_t compressionThreshold{-1};
        std::shared_ptr<AESContext> aesContext;
        std::shared_ptr<TCPTraffic> measureTraffic;

        explicit TCPConnection(Logger &logger, const std::shared_ptr<TCPSocket<>> &socket):
            logger(logger), socket(socket) {}

        auto sendPacket(const pc::Packet &packet) const -> void {
            ByteBuf packetBuffer;
            packet.Write(packetBuffer);
            sendPacketWithCompression(packetBuffer);
        }

        auto sendPacketWithCompression(const ByteBuf &packetBuffer) const -> void {
            if (compressionThreshold == -1) {
                sendPacket(packetBuffer);
                return;
            }
            if (packetBuffer.size() < compressionThreshold) {
                ByteBuf uncompressedPacket;
                pc::WriteData<pc::VarInt>(0, uncompressedPacket);
                uncompressedPacket.insert(uncompressedPacket.end(), packetBuffer.begin(), packetBuffer.end());
                sendPacket(uncompressedPacket);
                return;
            }
            ByteBuf compressedPacket;
            pc::WriteData<pc::VarInt>(static_cast<int>(packetBuffer.size()), compressedPacket);

            auto compressedData = compress(packetBuffer);
            compressedPacket.insert(compressedPacket.end(), compressedData.begin(), compressedData.end());
            sendPacket(compressedPacket);
        }

        auto append(const char *rawMessage, const size_t length) -> void {
            packetBuilder.write([&](auto &buffer) {
                buffer.reserve(length);
                if (!aesContext) {
                    for (size_t i = 0; i < length; i++)
                        buffer.push_back(rawMessage[i]);
                    return;
                }
                ByteBuf decrypted(length);
                std::memcpy(decrypted.data(), rawMessage, length);

                decrypted = aesContext->decrypt(decrypted);

                for (const auto i: decrypted)
                    buffer.push_back(i);
            });
        }

        auto handlePacket(ByteBuf &packet) -> void {
            logger.log<DEBUG>("Handling {} packet with encryption enabled = {}", Side::asString, aesContext != nullptr);
            if (compressionThreshold == -1) {
                listener.handlePacket(packet);
                return;
            }
            auto iter = packet.cbegin();
            auto length = packet.size();

            if (pc::ReadData<pc::VarInt>(iter, length) == 0) {
                packet.erase(packet.cbegin());
                listener.handlePacket(packet);
                return;
            }
            listener.handlePacket(decompress(packet, packet.size() - length));
        }

        auto handleMessage(const std::function<void(const std::runtime_error &)> &errorHandler) -> void {
            packetBuilder.write([&](auto &buffer) {
                while (!buffer.empty()) {
                    auto readIter = buffer.cbegin();
                    auto maxLength = buffer.size();
                    int packetLength;
                    try {
                        packetLength = pc::ReadData<pc::VarInt>(readIter, maxLength);
                    } catch (const std::runtime_error &) {
                        break;
                    }
                    const auto bytesRead = std::distance(buffer.cbegin(), readIter);
                    if (packetLength <= 0 || buffer.size() < bytesRead + packetLength)
                        break;

                    auto dataPacket = std::vector(buffer.begin() + bytesRead,
                                                  buffer.begin() + bytesRead + packetLength);

                    if (packetLength != dataPacket.size()) {
                        logger.log<ERROR>("packetLength was not equal to dataPacket.size(): {} / {}", packetLength,
                                          dataPacket.size());
                        break;
                    }
                    if (dataPacket.empty()) {
                        logger.log<ERROR>("dataPacket.size() was 0: {} / {}", packetLength, dataPacket.size());
                        break;
                    }
                    if (measureTraffic)
                        measureTraffic->incrementRecv(dataPacket.size());

                    try {
                        handlePacket(dataPacket);
                    } catch (const std::runtime_error &err) {
                        logger.log<ERROR>("Failed to handle packet: {}", err.what());
                        errorHandler(err);
                        break;
                    }
                    buffer.erase(buffer.begin(), buffer.begin() + bytesRead + packetLength);
                }
            });
        }

    private:
        auto sendPacket(const ByteBuf &packetWithID) const -> void {
            const auto packetLength = static_cast<int>(packetWithID.size());

            ByteBuf packetBuffer;
            pc::WriteData<pc::VarInt>(packetLength, packetBuffer);
            pc::WriteByteArray(packetWithID, packetBuffer);

            logger.log<DEBUG>("Sending packet to {}bound with encryption enabled = {}", Side::source,
                              aesContext != nullptr);

            // AES encryption does not change the packet size, just calculate here
            if (measureTraffic)
                measureTraffic->incrementSend(packetBuffer.size());

            if (!aesContext) {
                socket->sockSend(packetBuffer);
                return;
            }
            const auto encrypted = aesContext->encrypt(packetBuffer);
            logger.log<DEBUG>("Sending encrypted packet with length {}", encrypted.size());
            socket->sockSend(encrypted);
        }
    };
}
