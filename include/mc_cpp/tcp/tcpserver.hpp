#pragma once

#include <mc_cpp/tcp/tcpsocket.hpp>
#include <memory>
#include <thread>

#if defined(__linux__) || defined(__APPLE__)
#define USE_SO_REUSEPORT
#endif

namespace mc {

    template <uint16_t BUFFER_SIZE = AS_DEFAULT_BUFFER_SIZE>
    class TCPServer final: public BaseSocket {
    public:
        std::function<void(std::shared_ptr<TCPSocket<BUFFER_SIZE>>)> onNewConnection = [](const std::shared_ptr<TCPSocket<BUFFER_SIZE>> sock) {
            FDR_UNUSED(sock)
        };

        explicit TCPServer(FDR_ON_ERROR) : BaseSocket(onError) {
            constexpr char opt = 1;
            setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

#ifdef USE_SO_REUSEPORT
            setsockopt(this->sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));
#endif
        }

        void sockBind(const char* address, const uint16_t port, FDR_ON_ERROR) {
            switch (inet_pton(AF_INET, address, &this->address.sin_addr)) {
                case -1:
                    onError(errno, "Invalid address. Address type not supported.");
                return;
                case 0:
                    onError(errno, "AF_INET is not supported. Please send message to developer.");
                return;
                default:
                    break;
            }

            this->address.sin_family = AF_INET;
            this->address.sin_port = htons(port);

            if (bind(this->sock, reinterpret_cast<const sockaddr*>(&this->address), sizeof(this->address)) == -1)
                onError(errno, "Cannot bind the socket.");
        }

        void sockBind(const uint16_t port, FDR_ON_ERROR) { this->sockBind("0.0.0.0", port, onError); }

        void sockListen(FDR_ON_ERROR) {
            if (listen(this->sock, 20) == -1) {
                onError(errno, "Error: Server can't listen the socket.");
                return;
            }
            std::thread t(sockAccept, this, onError);
            t.detach();
        }

    private:
        static void sockAccept(TCPServer* server, FDR_ON_ERROR) {
            sockaddr_in newSocketInfo{};
            socklen_t newSocketInfoLength = sizeof(newSocketInfo);

            while (true) {
                const int newSocketFileDescriptor = accept(server->sock, reinterpret_cast<sockaddr*>(&newSocketInfo), &newSocketInfoLength);
                if (newSocketFileDescriptor == -1) {
                    if (errno == EBADF || errno == EINVAL) return;

                    onError(errno, "Error while accepting a new connection.");
                    return;
                }
                auto newSocket = std::make_shared<TCPSocket<BUFFER_SIZE>>(onError, newSocketFileDescriptor);
                newSocket->address = newSocketInfo;
                server->onNewConnection(newSocket);
                newSocket->sockListen();
            }
        }
    };

}