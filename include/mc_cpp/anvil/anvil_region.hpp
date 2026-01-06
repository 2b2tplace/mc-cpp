#pragma once

#include <spanstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/anvil/anvil_chunk.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>

namespace mc::anvil {

    static constexpr auto REGION_SIDELENGTH_CHUNKS = 32;
    static constexpr auto REGION_SIZE_CHUNKS = REGION_SIDELENGTH_CHUNKS * REGION_SIDELENGTH_CHUNKS;

    enum class RegionReadResult {
        OK,
        FILE_HANDLE_CLOSED,
        CORRUPTED_HEADER,
        INVALID_CHUNK_OFFSET,
        CORRUPTED_CHUNK_EMPTY,
        CORRUPTED_CHUNK_INVALID_SIZE
    };

    struct ChunkView {
        uint32_t timestamp{};
        uint8_t size{};
        std::vector<uint8_t> data{};
    };

    struct Region {
        Pos pos;
        absl::flat_hash_map<size_t, ChunkView> chunkViews;
        fs::path parentDirectory;
        DimensionType dimension;

        explicit Region(const fs::path &parentDirectory, const DimensionType dimension, const Pos &pos):
            pos(pos),
            parentDirectory(parentDirectory),
            dimension(dimension) {
            chunkViews.reserve(REGION_SIZE_CHUNKS);
        }

        [[nodiscard]]
        static auto chunkIndex(const Pos &chunkPos) -> size_t {
            return 4 * (chunkPos.localZ(REGION_SIDELENGTH_CHUNKS) * REGION_SIDELENGTH_CHUNKS + chunkPos.localX(REGION_SIDELENGTH_CHUNKS));
        }

        [[nodiscard]]
        static auto bytesToInt(const std::span<uint8_t> &arr, size_t start, const size_t end) -> uint32_t {
            uint32_t res = 0;
            do {
                res |= arr[start] << (end - start) * 8;
            } while (start++ < end);

            return res;
        }

        [[nodiscard]]
        auto absoluteChunkPos(const Pos &chunkPos) const {
            return Pos {
                pos.x * REGION_SIDELENGTH_CHUNKS + chunkPos.localX(REGION_SIDELENGTH_CHUNKS),
                pos.z * REGION_SIDELENGTH_CHUNKS + chunkPos.localZ(REGION_SIDELENGTH_CHUNKS)
            };
        }

        [[nodiscard]]
        auto createChunk(const int32_t chunkX, const int32_t chunkZ, const int32_t minY, const MinecraftRegistry &registry) const -> NbtFile {
            return createChunk(Pos{chunkX, chunkZ}, minY, registry);
        }

        [[nodiscard]]
        auto createChunk(const Pos chunkPos, const int32_t minY, const MinecraftRegistry &registry) const -> NbtFile {
            auto emptyPostProcessing = NbtList{NbtType::LIST};
            for (size_t i = 0; i < 24; i++)
                emptyPostProcessing.add(NbtList{NbtType::SHORT});

            const auto [absX, absZ] = absoluteChunkPos(chunkPos);

            NbtFile chunkNBT;
            chunkNBT.put("DataVersion", registry.dataVersion());
            chunkNBT.putNbt("Heightmaps", NbtCompound{});
            chunkNBT.put("InhabitedTime", 0L);
            chunkNBT.put("LastUpdate", 0L);
            chunkNBT.putNbt("PostProcessing", emptyPostProcessing);
            chunkNBT.put("xPos", absX);
            chunkNBT.put("zPos", absZ);
            chunkNBT.put("yPos", minY);
            chunkNBT.put("Status", "full");
            chunkNBT.putNbt("block_ticks", NbtList{NbtType::COMPOUND});
            chunkNBT.putNbt("fluid_ticks", NbtList{NbtType::COMPOUND});
            chunkNBT.put("isLightOn", true);
            return chunkNBT;
        }

        [[nodiscard]]
        auto readChunkNbtAt(const int32_t chunkX, const int32_t chunkZ, const bool allowOversizedChunk = true) const -> NbtFile {
            return readChunkNbtAt(Pos{chunkX, chunkZ}, allowOversizedChunk);
        }

        [[nodiscard]]
        auto readChunkNbtAt(const Pos &chunkPos, const bool allowOversizedChunk = true) const -> NbtFile {
            NbtFile chunk;
            const auto &view = chunkViewAt(chunkPos);
            const uint32_t length = view.data[0] << 24 | view.data[1] << 16 | view.data[2] << 8 | view.data[3];
            const auto flags = view.data[4];

            if (allowOversizedChunk && (flags & 128) != 0) {
                const auto compression = static_cast<Compression>(flags & -129);
                const auto path = externalChunkPath(chunkPos);
                std::ifstream in(path, std::ios_base::binary);
                in.seekg(0, std::ios::end);
                const auto filesize = in.tellg();
                in.seekg(0, std::ios::beg);

                std::vector<uint8_t> bytes(filesize);
                in.read(reinterpret_cast<char*>(bytes.data()), filesize);
                in.close();

                std::span span(reinterpret_cast<const char*>(bytes.data()), filesize);
                std::ispanstream iss(span);

                chunk.readNBT(iss, compression);
                return chunk;
            }
            const auto compression = static_cast<Compression>(flags);
            if (length > view.data.size())
                return chunk;

            std::span span(reinterpret_cast<const char*>(view.data.data()) + 5, length);
            std::ispanstream iss(span);

            chunk.readNBT(iss, compression);
            return chunk;
        }

        auto writeChunkNbtAt(const int32_t chunkX, const int32_t chunkZ, const NbtFile &chunkNBT,
                             const bool allowOversizedChunk = true,
                             const Compression compression = Compression::ZLIB) -> void {
            writeChunkNbtAt(Pos{chunkX, chunkZ}, chunkNBT, allowOversizedChunk, compression);
        }

        auto writeChunkNbtAt(const Pos &chunkPos, const NbtFile &chunkNBT,
                             const bool allowOversizedChunk = true,
                             const Compression compression = Compression::ZLIB) -> void {
            std::ostringstream oss(std::ios::out | std::ios::binary);
            chunkNBT.writeNBT(oss, compression);

            const auto str = oss.str();
            auto length = str.size();
            auto &data = chunkViewAt(chunkPos);

            auto flags = static_cast<uint8_t>(compression);

            auto sectorCount = (length + 5 + SECTION_SIZE_BLOCKS - 1) / SECTION_SIZE_BLOCKS;
            if (allowOversizedChunk && sectorCount >= 256) {
                length = 1;
                sectorCount = 1;

                const auto path = externalChunkPath(chunkPos);
                std::ofstream out(path, std::ios_base::binary);
                out.write(str.data(), str.size());
                out.close();
                flags |= 0x80;
            }
            data.data.resize(length + 5);
            std::memcpy(data.data.data() + 5, str.data(), length);

            data.size = static_cast<uint8_t>(sectorCount);
            data.data[0] = static_cast<uint8_t>(length >> 24);
            data.data[1] = static_cast<uint8_t>(length >> 16);
            data.data[2] = static_cast<uint8_t>(length >> 8);
            data.data[3] = static_cast<uint8_t>(length);
            data.data[4] = flags;
        }

        [[nodiscard]]
        auto chunkViewAt(const int32_t chunkX, const int32_t chunkZ) const -> const ChunkView& {
            return chunkViewAt(Pos{chunkX, chunkZ});
        }

        [[nodiscard]]
        auto chunkViewAt(const Pos &chunkPos) const -> const ChunkView& {
            return chunkViews.at(chunkIndex(chunkPos));
        }

        [[nodiscard]]
        auto chunkViewAt(const int32_t chunkX, const int32_t chunkZ) -> ChunkView& {
            return chunkViewAt(Pos{chunkX, chunkZ});
        }

        [[nodiscard]]
        auto chunkViewAt(const Pos &chunkPos) -> ChunkView& {
            return chunkViews[chunkIndex(chunkPos)];
        }

        [[nodiscard]]
        auto hasChunkViewAt(const int32_t chunkX, const int32_t chunkZ) const -> bool {
            return hasChunkViewAt(Pos{chunkX, chunkZ});
        }

        [[nodiscard]]
        auto hasChunkViewAt(const Pos &chunkPos) const -> bool {
            return chunkViews.contains(chunkIndex(chunkPos));
        }

        [[nodiscard]]
        auto externalChunkPath(const Pos &chunkPos) const -> fs::path {
            const auto [absX, absZ] = absoluteChunkPos(chunkPos);
            return parentDirectory / getDimensionRegionDirectory(dimension) / fmt::format("c.{}.{}.mcc", absX, absZ);
        }

        [[nodiscard]]
        auto filePath() const -> fs::path {
            return parentDirectory / getDimensionRegionDirectory(dimension) / fmt::format("r.{}.{}.mca", pos.x, pos.z);
        }

        [[nodiscard]]
        auto read() -> RegionReadResult {
            return read(filePath());
        }

        [[nodiscard]]
        auto read(const fs::path &path) -> RegionReadResult {
            std::ifstream in(path, std::ios_base::binary);
            const auto result = read(in);
            in.close();

            return result;
        }

        auto write() const -> void {
            write(filePath());
        }

        auto write(const fs::path &path) const -> void {
            std::ofstream out(path, std::ios_base::binary);
            write(out);
            out.close();
        }

        [[nodiscard]]
        auto read(std::istream &in) -> RegionReadResult {
            if (!in)
                return RegionReadResult::FILE_HANDLE_CLOSED;

            chunkViews.reserve(REGION_SIZE_CHUNKS);
            in.seekg(0, std::ios::end);
            const auto filesize = in.tellg();
            in.seekg(0, std::ios::beg);

            std::vector<uint8_t> bytes(filesize);
            in.read(reinterpret_cast<char*>(bytes.data()), filesize);

            const std::span offsets(bytes.data(), SECTION_SIZE_BLOCKS);
            const std::span timestamps(bytes.data() + SECTION_SIZE_BLOCKS, SECTION_SIZE_BLOCKS);
            const std::span chunkDataArray(bytes.data() + SECTION_SIZE_BLOCKS * 2, bytes.size() - SECTION_SIZE_BLOCKS * 2);

            for (size_t i = 0; i < SECTION_SIZE_BLOCKS; i += 4) {
                const uint32_t timestamp = bytesToInt(timestamps, i, i + 3);
                const uint32_t offset = bytesToInt(offsets, i, i + 2);

                const auto size = offsets[i + 3];
                if (size == 0) continue;

                const size_t chunkDataStart = (offset - 2) * SECTION_SIZE_BLOCKS;
                const size_t chunkDataEnd   = (offset + size - 2) * SECTION_SIZE_BLOCKS;

                if (chunkDataStart < 0 || static_cast<size_t>(chunkDataStart) >= chunkDataArray.size()) continue;
                if (chunkDataEnd < 0 || static_cast<size_t>(chunkDataEnd) > chunkDataArray.size() || chunkDataEnd < chunkDataStart) continue;

                chunkViews[i] = ChunkView {
                    timestamp, size,
                    std::vector(chunkDataArray.data() + chunkDataStart, chunkDataArray.data() + chunkDataEnd)
                };
            }
            return RegionReadResult::OK;
        }

        auto write(std::ostream &out) const -> void {
            std::vector<uint8_t> offsets(SECTION_SIZE_BLOCKS);
            std::vector<uint8_t> timestamps(SECTION_SIZE_BLOCKS);
            absl::flat_hash_map<size_t, const std::vector<uint8_t>*> chunkDataList;
            size_t maxPos{};

            size_t offset{2};
            for (const auto &[i, view] : chunkViews) {
                offsets[i] = static_cast<uint8_t>(offset >> 16);
                offsets[i + 1] = static_cast<uint8_t>(offset >> 8);
                offsets[i + 2] = static_cast<uint8_t>(offset);
                offsets[i + 3] = view.size;

                timestamps[i] = static_cast<uint8_t>(view.timestamp >> 24);
                timestamps[i + 1] = static_cast<uint8_t>(view.timestamp >> 16);
                timestamps[i + 2] = static_cast<uint8_t>(view.timestamp >> 8);
                timestamps[i + 3] = static_cast<uint8_t>(view.timestamp);

                chunkDataList[offset] = &view.data;

                if (const auto bytePos = (view.size + offset - 2) * SECTION_SIZE_BLOCKS;
                    bytePos > maxPos) maxPos = bytePos;

                offset += view.size;
            }
            const auto totalBytes = offsets.size() + timestamps.size() + maxPos;
            std::vector<uint8_t> res(totalBytes);

            std::memcpy(res.data(), offsets.data(), offsets.size());
            std::memcpy(res.data() + SECTION_SIZE_BLOCKS, timestamps.data(), timestamps.size());

            for (const auto& [i, data] : chunkDataList) {
                const auto j = static_cast<size_t>(i) * SECTION_SIZE_BLOCKS;
                if (j + data->size() > res.size()) continue;

                std::memcpy(res.data() + j, data->data(), data->size());
            }
            out.write(reinterpret_cast<const char*>(res.data()), static_cast<std::streamsize>(res.size()));
        }
    };

}
