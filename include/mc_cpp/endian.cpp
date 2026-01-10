#include <mc_cpp/endian.hpp>

namespace mc {

    auto readBEString(std::istream& stream) -> std::string {
        const auto length = readBE<int16_t>(stream);
        if (length < 0)
            throw std::runtime_error("readBE<string>: negative length");

        std::string value(static_cast<size_t>(length), '\0');
        stream.read(value.data(), length);

        return value;
    }

    auto writeBEString(std::ostream& stream, const std::string& value) -> void {
        if (value.size() > static_cast<size_t>(INT16_MAX))
            throw std::runtime_error("writeBE<string>: string too long");

        writeBE<int16_t>(stream, static_cast<int16_t>(value.size()));
        stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    }

}