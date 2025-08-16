#include <mc_cpp/compression/zlib.hpp>
#include <zlib.h>
#include <string>
#include <stdexcept>

namespace mc {
    auto compress(const std::vector<uint8_t> &uncompressed) -> std::vector<uint8_t> {
        const auto uncompressedSize = uncompressed.size();
        auto compressedSize = compressBound(uncompressedSize);

        auto compressedData = std::vector<uint8_t>(compressedSize);
        if (compress2(compressedData.data(), &compressedSize, uncompressed.data(), uncompressedSize,
                      Z_DEFAULT_COMPRESSION) != Z_OK)
            throw std::runtime_error("Error compressing packet");

        compressedData.resize(compressedSize);
        return compressedData;
    }

    auto decompress(const std::vector<uint8_t> &compressed, const size_t start) -> std::vector<uint8_t> {
        const auto decompressedSize = compressed.size() - start;

        std::vector<uint8_t> decompressedData;
        decompressedData.reserve(decompressedSize);

        std::vector<uint8_t> buffer(UINT16_MAX + 1);
        z_stream strm{};
        strm.next_in = const_cast<uint8_t *>(compressed.data() + start);
        strm.avail_in = decompressedSize;
        strm.next_out = buffer.data();
        strm.avail_out = static_cast<uint>(buffer.size());
        auto res = inflateInit(&strm);

        if (res != Z_OK)
            throw std::runtime_error("inflateInit failed: " + std::string(strm.msg));

        for (;;) {
            res = inflate(&strm, Z_NO_FLUSH);
            switch (res) {
                case Z_OK: {
                    decompressedData.insert(decompressedData.end(), buffer.begin(), buffer.end() - strm.avail_out);
                    strm.next_out = buffer.data();
                    strm.avail_out = static_cast<unsigned int>(buffer.size());
                    if (strm.avail_in == 0) {
                        inflateEnd(&strm);
                        return decompressedData;
                    }
                    break;
                }
                case Z_STREAM_END: {
                    decompressedData.insert(decompressedData.end(), buffer.begin(), buffer.end() - strm.avail_out);
                    inflateEnd(&strm);
                    return decompressedData;
                }
                default: {
                    inflateEnd(&strm);
                    throw std::runtime_error("Inflate decompression failed: " + std::string(strm.msg));
                }
            }
        }
    }
}
