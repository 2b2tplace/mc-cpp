#pragma once

#include <spanstream>
#include <fmt/format.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/stream.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/anvil/anvil_chunk.hpp>
#include <mc_cpp/registry/registries/dimensions.hpp>
#include <utility>

namespace mc::anvil {

    static constexpr auto REGION_SIDELENGTH_CHUNKS = 32;
    static constexpr auto REGION_SIZE_CHUNKS = REGION_SIDELENGTH_CHUNKS * REGION_SIDELENGTH_CHUNKS;

    static constexpr auto SECTOR_SIZE = 4096;

    enum class RegionReadResult {
        OK,
        FILE_HANDLE_CLOSED,
        EMPTY_REGION_FILE,
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

        explicit Region(fs::path parentDirectory, const DimensionType dimension, const Pos &pos);

        [[nodiscard]]
        static auto chunkIndex(const Pos &chunkPos) -> size_t ;

        [[nodiscard]]
        static auto bytesToInt(const std::span<uint8_t> &arr, size_t start, const size_t end) -> uint32_t ;

        [[nodiscard]]
        auto absoluteChunkPos(const Pos &chunkPos) const;

        [[nodiscard]]
        auto createChunk(const int32_t chunkX, const int32_t chunkZ, const int32_t minY, const MinecraftRegistry &registry) const -> NbtFile ;

        [[nodiscard]]
        auto createChunk(const Pos chunkPos, const int32_t minY, const MinecraftRegistry &registry) const -> NbtFile ;

        [[nodiscard]]
        auto readChunkNbtAt(const int32_t chunkX, const int32_t chunkZ, const bool allowOversizedChunk = true) const -> NbtFile ;

        [[nodiscard]]
        auto readChunkNbtAt(const Pos &chunkPos, const bool allowOversizedChunk = true) const -> NbtFile ;

        auto writeChunkNbtAt(const int32_t chunkX, const int32_t chunkZ, const NbtFile &chunkNBT,
                             const bool allowOversizedChunk = true,
                             const Compression compression = Compression::ZLIB) -> void ;

        auto writeChunkNbtAt(const Pos &chunkPos, const NbtFile &chunkNBT,
                             const bool allowOversizedChunk = true,
                             const Compression compression = Compression::ZLIB) -> void ;

        [[nodiscard]]
        auto chunkViewAt(const int32_t chunkX, const int32_t chunkZ) const -> const ChunkView& ;

        [[nodiscard]]
        auto chunkViewAt(const Pos &chunkPos) const -> const ChunkView& ;

        [[nodiscard]]
        auto chunkViewAt(const int32_t chunkX, const int32_t chunkZ) -> ChunkView& ;

        [[nodiscard]]
        auto chunkViewAt(const Pos &chunkPos) -> ChunkView& ;

        [[nodiscard]]
        auto hasChunkViewAt(const int32_t chunkX, const int32_t chunkZ) const -> bool ;

        [[nodiscard]]
        auto hasChunkViewAt(const Pos &chunkPos) const -> bool ;

        [[nodiscard]]
        auto externalChunkPath(const Pos &chunkPos) const -> fs::path ;

        [[nodiscard]]
        auto filePath() const -> fs::path ;

        [[nodiscard]]
        auto read() -> RegionReadResult ;

        [[nodiscard]]
        auto read(const fs::path &path) -> RegionReadResult ;

        auto write() const -> void ;

        auto write(const fs::path &path) const -> void ;

        [[nodiscard]]
        auto read(std::istream &in) -> RegionReadResult ;

        auto write(std::ostream &out) const -> void ;
    };

}
