#pragma once

#include <mutex>

namespace mc {
    template<class T, class M=std::mutex,
        template<class...> class WL=std::unique_lock,
        template<class...> class RL=std::unique_lock>
    struct Mutex {
        auto read(const auto &readFunction) const {
            const auto lock = readLock();
            return readFunction(data);
        }

        template<typename Member>
        auto read(Member T::*member) const -> Member {
            const auto lock = readLock();
            return data.*member;
        }

        auto write(const auto &writeFunction) {
            const auto lock = writeLock();
            return writeFunction(data);
        }

        auto clone() const -> T {
            return read([this](const auto &) { return data; });
        }

        Mutex() = default;

        explicit Mutex(T in): data(std::move(in)) {
        }

    private:
        mutable M mtx;
        T data;

        auto readLock() const {
            return RL<M>(mtx);
        }

        auto writeLock() const {
            return WL<M>(mtx);
        }
    };
}
