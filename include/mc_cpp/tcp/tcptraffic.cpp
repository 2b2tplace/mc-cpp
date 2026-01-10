#include <mc_cpp/tcp/tcptraffic.hpp>

namespace mc {
    TCPTraffic::TCPTraffic(const TCPTraffic &other):
        parent(other.parent),
        totalBytesSend(other.totalBytesSend.load()),
        totalBytesRecv(other.totalBytesRecv.load()) {}

    auto TCPTraffic::setParent(const std::shared_ptr<TCPTraffic> &parentNew) -> void {
        parent = parentNew;
    }

    auto TCPTraffic::incrementRecv(const size_t amount) -> void {
        if (parent) parent->incrementRecv(amount);
        totalBytesRecv += amount;
    }

    auto TCPTraffic::recv() -> size_t {
        return totalBytesRecv;
    }

    auto TCPTraffic::incrementSend(const size_t amount) -> void {
        if (parent) parent->incrementSend(amount);
        totalBytesSend += amount;
    }

    auto TCPTraffic::send() -> size_t {
        return totalBytesSend;
    }

}