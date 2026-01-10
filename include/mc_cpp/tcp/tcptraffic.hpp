#pragma once

#include <atomic>
#include <memory>

namespace mc {

    class TCPTraffic {
        std::shared_ptr<TCPTraffic> parent;
        std::atomic<size_t> totalBytesSend;
        std::atomic<size_t> totalBytesRecv;

    public:
        TCPTraffic() = default;

        TCPTraffic(const TCPTraffic &other);

        auto setParent(const std::shared_ptr<TCPTraffic> &parentNew) -> void;

        auto incrementRecv(size_t amount) -> void;

        [[nodiscard]]
        auto recv() -> size_t;

        auto incrementSend(size_t amount) -> void;

        [[nodiscard]]
        auto send() -> size_t;
    };

}
