#pragma once

#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <cerrno>
#include <functional>
#include <string>

#define FDR_UNUSED(expr) \
    { (void)(expr); }
#define FDR_ON_ERROR \
    const std::function<void(int, const std::string&)> &onError = [](const int errorCode, const std::string& errorMessage) { \
        FDR_UNUSED(errorCode); \
        FDR_UNUSED(errorMessage); \
    }

#ifndef AS_DEFAULT_BUFFER_SIZE
#define AS_DEFAULT_BUFFER_SIZE 0x1000
#endif

namespace mc {

    class BaseSocket {
    public:
        sockaddr_in address{};

        auto sockClose() const -> void {
#if defined(_WIN32)
            shutdown(this->sock, SD_BOTH);
            closesocket(this->sock);
            WSACleanup();
#else
            shutdown(this->sock, SHUT_RDWR);
            close(this->sock);
#endif
        }

        [[nodiscard]]
        auto remoteAddress() const -> std::string {
            return ipToString(this->address);
        }

        [[nodiscard]]
        auto remotePort() const -> int {
            return ntohs(this->address.sin_port);
        }

        [[nodiscard]]
        auto fileDescriptor() const -> int {
            return this->sock;
        }

    protected:
        int sock = 0;

        [[nodiscard]]
        static auto ipToString(const sockaddr_in &addr) -> std::string {
            char ip[16];
#if defined(__linux__) || defined(__APPLE__)
            inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
#elif defined(_WIN32)
            inet_ntop(AF_INET, &addr.sin_addr, ip, 16);
#endif

            return {ip};
        }

        explicit BaseSocket(FDR_ON_ERROR, const int socketId = -1) {
#if defined(_WIN32)
            WSADATA wsaData;
            int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (iResult != 0) {
                onError(iResult, "WSAStartup failed.");
                return;
            }
#endif

            if (socketId != -1) {
                this->sock = socketId;
                return;
            }
            this->sock = socket(AF_INET, SOCK_STREAM, 0);

            if (this->sock == -1)
                onError(errno, "Socket creating error.");
        }

        virtual ~BaseSocket() = default;
    };

}