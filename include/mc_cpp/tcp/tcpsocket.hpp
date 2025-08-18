#pragma once

#include <mc_cpp/tcp/basesocket.hpp>
#include <cstring>
#include <functional>
#include <string>
#include <thread>

namespace mc {

    template <uint16_t BUFFER_SIZE = AS_DEFAULT_BUFFER_SIZE>
    class TCPSocket final: public BaseSocket {
    public:
        std::function<void(std::string)> onMessageReceived;
        std::function<void(const char*, ssize_t)> onRawMessageReceived;
        std::function<void(int)> onSocketClosed;

        explicit TCPSocket(FDR_ON_ERROR, const int socketId = -1) : BaseSocket(onError, socketId) {}

        auto sockSend(const char *bytes, const size_t length) const -> void {
            send(this->sock, bytes, length, 0);
        }

        auto sockSend(const std::vector<uint8_t> &bytes) const -> void {
            send(this->sock, bytes.data(), bytes.size(), 0);
        }

        auto sockSend(const std::string &message) const -> void {
            sockSend(message.c_str(), message.length());
        }

        auto sockConnect(uint32_t ipv4, const uint16_t port, const std::function<void()> &onConnected = [] {
                         }, FDR_ON_ERROR) -> void {
            this->address.sin_family = AF_INET;
            this->address.sin_port = htons(port);
            this->address.sin_addr.s_addr = ipv4;

            this->setTimeout(5);

            if (connect(this->sock, reinterpret_cast<const sockaddr*>(&this->address), sizeof(sockaddr_in)) == -1) {
                onError(errno, "Connection failed to the host.");
                this->setTimeout(0);
                return;
            }
            this->setTimeout(0);

            onConnected();
            this->sockListen();
        }

        auto sockConnect(const char *host, const uint16_t port, const std::function<void()> &onConnected = [] {
                         }, FDR_ON_ERROR) -> void {
            addrinfo hints{}, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            if (const int status = getaddrinfo(host, nullptr, &hints, &res); status != 0) {
                onError(errno, "Invalid address." + std::string(gai_strerror(status)));
                return;
            }
            for (const addrinfo *it = res; it != nullptr; it = it->ai_next) {
                if (it->ai_family == AF_INET) {
                    memcpy(static_cast<void*>(&this->address), it->ai_addr, sizeof(sockaddr_in));
                    break;
                }
            }
            freeaddrinfo(res);
            this->sockConnect(this->address.sin_addr.s_addr, port, onConnected, onError);
        }

        auto sockConnect(const std::string &host, const uint16_t port, const std::function<void()> &onConnected = [] {
                         }, FDR_ON_ERROR) -> void {
            this->sockConnect(host.c_str(), port, onConnected, onError);
        }

        auto sockListen() -> void {
            std::thread t(sockReceive, this);
            t.detach();
        }

    private:
        static auto sockReceive(const TCPSocket *socket) -> void {
            char tempBuffer[BUFFER_SIZE + 1];
            ssize_t messageLength;

            while ((messageLength = recv(socket->sock, tempBuffer, BUFFER_SIZE, 0)) > 0) {
                tempBuffer[messageLength] = '\0';
                if (socket->onMessageReceived)
                    socket->onMessageReceived(std::string(tempBuffer, messageLength));

                if (socket->onRawMessageReceived)
                    socket->onRawMessageReceived(tempBuffer, messageLength);
            }
            socket->sockClose();
            if (socket->onSocketClosed)
                socket->onSocketClosed(errno);
        }

        auto setTimeout(const int seconds) const -> void {
            timeval tv{};
            tv.tv_sec = seconds;
            tv.tv_usec = 0;

            setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(tv));
            setsockopt(this->sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&tv), sizeof(tv));
        }
    };

}