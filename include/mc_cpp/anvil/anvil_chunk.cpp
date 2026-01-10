#include <mc_cpp/anvil/anvil_chunk.hpp>

namespace mc {
    auto anvil::Pos::localX(const int32_t parentSidelength) const -> int32_t {
        return x % parentSidelength < 0 ? x % parentSidelength + parentSidelength : x % parentSidelength;
    }

    auto anvil::Pos::localZ(const int32_t parentSidelength) const -> int32_t {
        return z % parentSidelength < 0 ? z % parentSidelength + parentSidelength : z % parentSidelength;
    }

    anvil::ChunkSection::ChunkSection(const MinecraftRegistry &registry, const int8_t yLevel): yLevel(yLevel) {
        unpackedBlockData.fill(registry.blockState("air"));
        unpackedBiomeData.fill(registry.biomeType("plains"));
        packedBlockLightData.fill(-1);
        packedSkyLightData.fill(-1);
    }

    void anvil::prepareTileEntity(NbtCompound &tileEntityNBT, const std::string_view tileEntityName,
        const Pos &localizeBlockPos, const int32_t yLevel) {
        tileEntityNBT.put("x", localizeBlockPos.localX(SECTION_SIZE_BLOCKS));
        tileEntityNBT.put("z", localizeBlockPos.localZ(SECTION_SIZE_BLOCKS));
        tileEntityNBT.put("y", yLevel);
        tileEntityNBT.put("keepPacked", false);

        auto tileEntityNameNamespaced = std::string(tileEntityName);
        prependMinecraftNamespace(&tileEntityNameNamespaced);
        tileEntityNBT.put("id", tileEntityNameNamespaced);
    }

    auto anvil::Chunk::writeSections(NbtCompound &chunkNBT, const MinecraftRegistry &registry) const -> void {
        chunkNBT.putNbt("block_entities", tileEntities);
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

    auto anvil::Chunk::readSections(const NbtCompound &chunkNBT,
        const MinecraftRegistry &registry) -> result::Option<Chunk> {
        const auto &sections = chunkNBT.readNbt<NbtList>("sections");
        Chunk chunk;
        chunk.tileEntities = chunkNBT.readNbt<NbtList>("block_entities");
        chunk.sections.reserve(sections.length());

        for (int i = 0; i < sections.length(); i++) {
            const auto section = sections.readNbt<NbtCompound>(i);
            if (!section.contains("Y")) return result::None;

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
                if (compound.containsNbt<NbtCompound>("Properties")) {
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
            if (blockStates.containsNbt<NbtLongArray>("data")) {
                const auto &data = blockStates.readNbt<NbtLongArray>("data");
                mc::anvil::unpackPalettedData<Blocks>(data.value, blockStatePalette, chunkSection.unpackedBlockData);
            } else if (!blockStatePalette.empty()) {
                chunkSection.unpackedBlockData.fill(blockStatePalette[0]);
            }

            if (section.containsNbt<NbtCompound>("biomes")) {
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
                if (biomes.containsNbt<NbtLongArray>("data")) {
                    const auto &data = biomes.readNbt<NbtLongArray>("data");
                    mc::anvil::unpackPalettedData<Biomes>(data.value, biomeIdPalette, chunkSection.unpackedBiomeData);
                } else if (!biomeIdPalette.empty()) {
                    chunkSection.unpackedBiomeData.fill(biomeIdPalette[0]);
                }
            }
            if (section.containsNbt<NbtByteArray>("BlockLight")) {
                const auto &blockLight = section.readNbt<NbtByteArray>("BlockLight");
                std::ranges::copy_n(
                    blockLight.value.begin(),
                    static_cast<int64_t>(std::min(blockLight.value.size(), chunkSection.packedBlockLightData.size())),
                    chunkSection.packedBlockLightData.begin()
                );
            }
            if (section.containsNbt<NbtByteArray>("SkyLight")) {
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
}
