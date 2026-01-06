#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <cassert>
#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace mc::anvil {
    static constexpr size_t SECTION_SIDELENGTH_BLOCKS = 16;
    static constexpr size_t SECTION_SIZE_BLOCKS = SECTION_SIDELENGTH_BLOCKS * SECTION_SIDELENGTH_BLOCKS * SECTION_SIDELENGTH_BLOCKS;

    static constexpr size_t SECTION_SIDELENGTH_BIOMES = 4;
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

    struct ChunkSection {
        UnpackedData<Blocks> unpackedBlockData{};
        UnpackedData<Biomes> unpackedBiomeData{};
        PackedLightData packedBlockLightData{};
        PackedLightData packedSkyLightData{};
        int8_t yLevel;

        explicit ChunkSection(const MinecraftRegistry &registry, const int8_t yLevel): yLevel(yLevel) {
            unpackedBlockData.fill(registry.blockState("air"));
            unpackedBiomeData.fill(registry.biomeType("plains"));
            packedBlockLightData.fill(-1);
            packedSkyLightData.fill(-1);
        }
    };

    struct Chunk {
        std::vector<ChunkSection> sections;

        auto writeSections_1_18(NbtCompound &chunkNBT, const MinecraftRegistry &registry) const -> void {
            NbtList sectionsNBT;

            for (const auto &section : sections) {
                NbtCompound sectionNBT;
                sectionNBT.put("Y", section.yLevel);

                NbtCompound blockStatesNBT;
                NbtList blockStatePaletteNBT;

                Palette blockStatePalette;
                PackedData blockStateData;
                packPalettedData<Blocks>(blockStateData, blockStatePalette, section.unpackedBlockData);

                for (const auto stateId : blockStatePalette) {
                    NbtCompound compound;
                    std::string fullName = registry.blockName(stateId);
                    prependMinecraftNamespace(&fullName);
                    compound.put("Name", fullName);

                    if (const auto &map = registry.blockStatePropertyMap(stateId); !map.empty()) {
                        NbtCompound properties;
                        for (const auto &[key, value] : map) {
                            properties.put(key, value);
                        }
                        compound.putNbt("Properties", properties);
                    }
                    blockStatePaletteNBT.add(compound);
                }
                blockStatesNBT.putNbt("palette", blockStatePaletteNBT);
                if (!blockStateData.empty())
                    blockStatesNBT.putNbt("data", NbtLongArray{blockStateData});

                sectionNBT.putNbt("block_states", blockStatesNBT);

                NbtCompound biomesNBT;
                NbtList biomePaletteNBT;

                Palette biomeIdPalette;
                PackedData biomeData;
                packPalettedData<Biomes>(biomeData, biomeIdPalette, section.unpackedBiomeData);

                for (const auto biomeId : biomeIdPalette) {
                    auto biomeName = registry.biomeName(biomeId);
                    prependMinecraftNamespace(&biomeName);
                    biomePaletteNBT.add(NbtString{biomeName});
                }
                biomesNBT.putNbt("palette", biomePaletteNBT);
                if (!biomeData.empty())
                    biomesNBT.putNbt("data", NbtLongArray{biomeData});

                sectionNBT.putNbt("biomes", biomesNBT);

                const auto hasBlockLight = std::ranges::any_of(section.packedBlockLightData, [](const int8_t level) { return level != 0; });
                const auto hasSkyLight = std::ranges::any_of(section.packedSkyLightData,  [](const int8_t level) { return level != 0; });

                if (hasBlockLight)
                    sectionNBT.put("BlockLight", std::vector(section.packedBlockLightData.begin(), section.packedBlockLightData.end()));

                if (hasSkyLight)
                    sectionNBT.put("SkyLight", std::vector(section.packedSkyLightData.begin(), section.packedSkyLightData.end()));

                sectionsNBT.add(sectionNBT);
            }
            chunkNBT.putNbt<NbtList>("sections", sectionsNBT);
        }

        [[nodiscard]]
        static auto readSections_1_18(const NbtCompound &chunkNBT,
                                      const MinecraftRegistry &registry) -> result::Option<Chunk> {
            const auto &sections = chunkNBT.readNbt<NbtList>("sections");
            Chunk chunk;
            chunk.sections.reserve(sections.length());

            for (int i = 0; i < sections.length(); i++) {
                const auto section = sections.readNbt<NbtCompound>(i);
                if (!section.contains("block_states") || !section.contains("Y")) return result::None;

                chunk.sections.emplace_back(registry, section.read<int8_t>("Y"));
                auto &chunkSection = chunk.sections[i];
                const auto &blockStates = section.readNbt<NbtCompound>("block_states");
                const auto &palette = blockStates.readNbt<NbtList>("palette");
                Palette blockStatePalette;
                blockStatePalette.reserve(palette.length());

                for (size_t j = 0; j < palette.length(); j++) {
                    const auto &compound = palette.readNbt<NbtCompound>(j);
                    const auto &fullName = compound.read<std::string>("Name");
                    auto name = std::string_view(fullName);
                    stripMinecraftNamespace(&name);

                    BlockStatePropertyMap map;
                    if (compound.contains("Properties", NbtType::COMPOUND)) {
                        const auto &properties = compound.readNbt<NbtCompound>("Properties");
                        map.reserve(properties.entries.size());
                        for (const auto &[key, value] : properties.entries) {
                            if (value->getType() != NbtType::STRING) continue;

                            map[key] = mc::get<std::string>(*value);
                        }
                    }
                    const auto [min, max] = registry.blockType(name);
                    if (min == MISSING_BLOCK_STATE || max == MISSING_BLOCK_STATE) continue;

                    BlockState foundState = MISSING_BLOCK_STATE;
                    for (BlockState state = min; state <= max; state++) {
                        if (map == registry.blockStatePropertyMap(state)) {
                            foundState = state;
                            break;
                        }
                    }
                    blockStatePalette.push_back(foundState);
                }
                if (blockStates.contains<NbtLongArray>("data")) {
                    const auto &data = blockStates.readNbt<NbtLongArray>("data");
                    mc::anvil::unpackPalettedData<Blocks>(data.value, blockStatePalette, chunkSection.unpackedBlockData);
                } else if (!blockStatePalette.empty()) {
                    chunkSection.unpackedBlockData.fill(blockStatePalette[0]);
                }

                if (section.contains<NbtCompound>("biomes")) {
                    const auto &biomes = section.readNbt<NbtCompound>("biomes");
                    const auto &biomePalette = biomes.readNbt<NbtList>("palette");
                    Palette biomeIdPalette;
                    biomeIdPalette.reserve(biomePalette.length());

                    for (size_t j = 0; j < biomePalette.length(); j++) {
                        const auto &fullName = biomePalette.read<std::string>(j);
                        auto name = std::string_view(fullName);
                        stripMinecraftNamespace(&name);

                        biomeIdPalette.push_back(registry.biomeType(name));
                    }
                    if (biomes.contains<NbtLongArray>("data")) {
                        const auto &data = biomes.readNbt<NbtLongArray>("data");
                        mc::anvil::unpackPalettedData<Biomes>(data.value, biomeIdPalette, chunkSection.unpackedBiomeData);
                    } else if (!biomeIdPalette.empty()) {
                        chunkSection.unpackedBiomeData.fill(biomeIdPalette[0]);
                    }
                }
                if (section.contains<NbtByteArray>("BlockLight")) {
                    const auto &blockLight = section.readNbt<NbtByteArray>("BlockLight");
                    std::ranges::copy_n(
                        blockLight.value.begin(),
                        static_cast<int64_t>(std::min(blockLight.value.size(), chunkSection.packedBlockLightData.size())),
                        chunkSection.packedBlockLightData.begin()
                    );
                }
                if (section.contains<NbtByteArray>("SkyLight")) {
                    const auto &skyLight = section.readNbt<NbtByteArray>("SkyLight");
                    std::ranges::copy_n(
                        skyLight.value.begin(),
                        static_cast<int64_t>(std::min(skyLight.value.size(), chunkSection.packedSkyLightData.size())),
                        chunkSection.packedSkyLightData.begin()
                    );
                }
            }
            return chunk;
        }
    };
}
