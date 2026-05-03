#include <mc_cpp/endian.hpp>

namespace mc {

    auto readBEString(std::istream& stream) -> std::string {
        const auto length = readBE<uint16_t>(stream);
        std::string value(length, '\0');
        stream.read(value.data(), length);

        return value;
    }

    auto writeBEString(std::ostream& stream, const std::string& value) -> void {
        if (value.length() > static_cast<size_t>(UINT16_MAX))
            throw std::runtime_error("writeBE<string>: string too long");

        writeBE<uint16_t>(stream, static_cast<uint16_t>(value.length()));
        stream.write(value.data(), static_cast<std::streamsize>(value.length()));
    }

}