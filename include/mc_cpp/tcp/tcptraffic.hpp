#pragma once

#include <atomic>

namespace mc {

    struct TCPTraffic {
        std::atomic<size_t> totalBytesSend;
        std::atomic<size_t> totalBytesRecv;
    };

}
