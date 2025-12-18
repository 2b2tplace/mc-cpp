#pragma once

#include <bit>
#include <iosfwd>
#include <istream>
#include <ostream>

namespace mc {

    constexpr auto isBigEndian() -> bool {
        return std::endian::native == std::endian::big;
    }

    template <typename T>
    constexpr T bigEndian(T x) {
        if constexpr (isBigEndian()) return x;

        return std::byteswap(x);
    }

    template <typename T>
    [[nodiscard]]
    auto readBE(std::istream& stream) -> T requires std::is_trivially_copyable_v<T> {
        T value{};
        stream.read(reinterpret_cast<char*>(&value), sizeof(value));

        if constexpr (std::is_integral_v<T>) {
            return bigEndian(value);
        } else if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            static_assert(sizeof(T) == sizeof(I));

            I tmp = std::bit_cast<I>(value);
            tmp = bigEndian(tmp);
            return std::bit_cast<T>(tmp);
        }
        return value;
    }

    template <typename T>
    void writeBE(std::ostream& stream, T value) requires std::is_trivially_copyable_v<T> {
        if constexpr (std::is_integral_v<T>) {
            value = bigEndian(value);
            stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
        } else if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            static_assert(sizeof(T) == sizeof(I));

            I tmp = std::bit_cast<I>(value);
            tmp = bigEndian(tmp);
            stream.write(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
        }
    }

    [[nodiscard]]
    inline std::string readBEString(std::istream& stream) {
        const auto length = readBE<int16_t>(stream);
        if (length < 0)
            throw std::runtime_error("readBE<string>: negative length");

        std::string value(static_cast<size_t>(length), '\0');
        stream.read(value.data(), length);

        return value;
    }

    inline void writeBEString(std::ostream& stream, const std::string& value) {
        if (value.size() > static_cast<size_t>(INT16_MAX))
            throw std::runtime_error("writeBE<string>: string too long");

        writeBE<int16_t>(stream, static_cast<int16_t>(value.size()));
        stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    }

}
