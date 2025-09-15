#pragma once

#include <atomic>

namespace mc {

    class TCPTraffic {
        std::shared_ptr<TCPTraffic> parent;
        std::atomic<size_t> totalBytesSend;
        std::atomic<size_t> totalBytesRecv;

    public:
        TCPTraffic() = default;

        TCPTraffic(const TCPTraffic &other) {
            parent = other.parent;
            totalBytesSend = other.totalBytesSend.load();
            totalBytesRecv = other.totalBytesRecv.load();
        }

        auto setParent(const TCPTraffic &parentNew) -> void {
            parent = std::make_shared<TCPTraffic>(parentNew);
        }

        auto incrementRecv(const size_t amount) -> void {
            if (parent) parent->incrementRecv(amount);
            totalBytesRecv += amount;
        }

        auto incrementSend(const size_t amount) -> void {
            if (parent) parent->incrementSend(amount);
            totalBytesSend += amount;
        }
    };

}
