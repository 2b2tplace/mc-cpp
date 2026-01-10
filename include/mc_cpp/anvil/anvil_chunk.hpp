#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <cassert>
#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace mc::anvil {

    struct Pos {
        int32_t x{};
        int32_t z{};

        [[nodiscard]]
        auto localX(const int32_t parentSidelength) const -> int32_t ;

        [[nodiscard]]
        auto localZ(const int32_t parentSidelength) const -> int32_t ;
    };

    static constexpr auto SECTION_SIDELENGTH_BLOCKS = 16;
    static constexpr size_t SECTION_SIZE_BLOCKS = SECTION_SIDELENGTH_BLOCKS * SECTION_SIDELENGTH_BLOCKS * SECTION_SIDELENGTH_BLOCKS;

    static constexpr auto SECTION_SIDELENGTH_BIOMES = 4;
    static constexpr size_t SECTION_SIZE_BIOMES = SECTION_SIDELENGTH_BIOMES * SECTION_SIDELENGTH_BIOMES * SECTION_SIDELENGTH_BIOMES;

    static constexpr size_t SECTION_SIZE_LIGHT = 2048;

    struct Blocks{};
    struct Biomes{};

    template<typename SectionDataType>
    struct SectionDataInfo {
        static_assert(std::is_same_v<SectionDataType, Blocks> || std::is_same_v<SectionDataType, Biomes>);
    };

    template<>
    struct SectionDataInfo<Blocks> {
        static constexpr uint8_t paletteMinBits = 4;
        static constexpr uint8_t paletteMaxBits = 8;
        static constexpr uint8_t sidelength = SECTION_SIDELENGTH_BLOCKS;
        static constexpr size_t sectionSize = SECTION_SIZE_BLOCKS;
    };

    template<>
    struct SectionDataInfo<Biomes> {
        static constexpr uint8_t paletteMinBits = 0;
        static constexpr uint8_t paletteMaxBits = 3;
        static constexpr uint8_t sidelength = SECTION_SIDELENGTH_BIOMES;
        static constexpr size_t sectionSize = SECTION_SIZE_BIOMES;
    };

    template<typename SectionDataType>
    using UnpackedData = std::array<uint16_t, SectionDataInfo<SectionDataType>::sectionSize>;
    using PackedData = std::vector<int64_t>;

    using PackedLightData = std::array<int8_t, SECTION_SIZE_LIGHT>;
    using Palette = std::vector<uint16_t>;
    using PaletteIndices = std::array<uint16_t, UINT16_MAX + 1>;

    struct ChunkSection {
        UnpackedData<Blocks> unpackedBlockData{};
        UnpackedData<Biomes> unpackedBiomeData{};
        PackedLightData packedBlockLightData{};
        PackedLightData packedSkyLightData{};
        int8_t yLevel;

        explicit ChunkSection(const MinecraftRegistry &registry, const int8_t yLevel);
    };

    static void prepareTileEntity(NbtCompound &tileEntityNBT, const std::string_view tileEntityName,
                                  const Pos &localizeBlockPos, const int32_t yLevel);

    struct Chunk {
        std::vector<ChunkSection> sections;
        NbtList tileEntities;

        auto writeSections(NbtCompound &chunkNBT, const MinecraftRegistry &registry) const -> void ;

        [[nodiscard]]
        static auto readSections(const NbtCompound &chunkNBT, const MinecraftRegistry &registry) -> result::Option<Chunk> ;
    };

    template<typename SectionDataType>
    [[nodiscard]]
    static uint64_t getBitsPerIndex(const size_t length) {
        return std::max<uint64_t>(std::bit_width(std::max(length, 1UL) - 1), SectionDataInfo<SectionDataType>::paletteMinBits);
    }

    template<typename SectionDataType>
    void unpackPalettedDataPre1_16(const PackedData &data, const Palette &palette, UnpackedData<SectionDataType> &unpacked) {
        const auto bitsPerValue = getBitsPerIndex<SectionDataType>(palette.size());
        const auto individualValueMask = (1 << bitsPerValue) - 1;
        const auto blocksPerLong = 64 / bitsPerValue;

        for (size_t i = 0; i < SectionDataInfo<SectionDataType>::sectionSize; i++) {
            const auto startLong = i * bitsPerValue / 64;
            const auto startOffset = i * bitsPerValue % 64;
            const auto endLong = ((i + 1) * bitsPerValue - 1) / 64;

            assert(startLong < data.size());
            uint16_t value;
            if (startLong == endLong) {
                value = static_cast<uint16_t>(data[startLong] >> startOffset);
            } else {
                const auto endOffset = 64 - startOffset;
                assert(endLong < data.size());
                value = static_cast<uint16_t>(data[startLong] >> startOffset | data[endLong] << endOffset);
            }
            value &= individualValueMask;
            unpacked[i] = value;
        }
    }

    template<typename SectionDataType>
    void unpackPalettedData(const PackedData &data, const Palette &palette, UnpackedData<SectionDataType> &unpacked) {
        if (palette.size() == 1 && data.empty()) {
            unpacked.fill(palette[0]);
            return;
        }
        const auto bitsPerValue = getBitsPerIndex<SectionDataType>(palette.size());
        const auto individualValueMask = (1 << bitsPerValue) - 1;
        const auto blocksPerLong = 64 / bitsPerValue;

        for (size_t i = 0; i < SectionDataInfo<SectionDataType>::sectionSize; i++) {
            const auto longIndex = i / blocksPerLong;
            const auto indexInLong = i % blocksPerLong;
            const auto startOffset = indexInLong * bitsPerValue;

            assert(longIndex < data.size());
            unpacked[i] = palette[static_cast<uint16_t>(data[longIndex] >> startOffset) & individualValueMask];
        }
    }

    template<typename SectionDataType>
    void buildPalette(Palette &palette, PaletteIndices &indices, const UnpackedData<SectionDataType> &unpacked) {
        std::bitset<UINT16_MAX + 1> unique;
        for (const auto atom : unpacked) {
            if (unique.test(atom)) continue;
            unique.set(atom);

            indices[atom] = static_cast<uint16_t>(palette.size());
            palette.push_back(atom);
        }
    }

    template<typename SectionDataType>
    auto packPalettedData(PackedData &data, Palette &palette, const UnpackedData<SectionDataType> &unpacked) -> void {
        PaletteIndices indices;
        buildPalette<SectionDataType>(palette, indices, unpacked);
        if (palette.size() == 1) return;

        const auto bitsPerValue = getBitsPerIndex<SectionDataType>(palette.size());
        const auto individualValueMask = (1 << bitsPerValue) - 1;
        const auto blocksPerLong = 64 / bitsPerValue;
        const auto packedSize = static_cast<size_t>(std::ceil(static_cast<double>(SectionDataInfo<SectionDataType>::sectionSize) / blocksPerLong));

        data.resize(packedSize);

        for (size_t i = 0; i < SectionDataInfo<SectionDataType>::sectionSize; i++) {
            const auto longIndex = i / blocksPerLong;
            const auto indexInLong = i % blocksPerLong;
            const auto startOffset = indexInLong * bitsPerValue;

            assert(i < unpacked.size());
            assert(unpacked[i] < indices.size());
            const auto packedValue = static_cast<int64_t>(indices[unpacked[i]] & individualValueMask);

            assert(longIndex < data.size());
            data[longIndex] &= ~(static_cast<int64_t>(individualValueMask) << startOffset);
            data[longIndex] |= static_cast<int64_t>(packedValue << startOffset);
        }
    }
}
