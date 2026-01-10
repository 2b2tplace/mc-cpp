#include <mc_cpp/registry/registries/biome_colors.hpp>

namespace mc {

    DEFINE_ENTRY_FROM_JSON(ColorTriangleEntry);
    DEFINE_ENTRY_TO_JSON(ColorTriangleEntry);

    DEFINE_ENTRY_FROM_JSON(BiomeColorEntry);
    DEFINE_ENTRY_TO_JSON(BiomeColorEntry);

    BiomeColorRegistry::BiomeColorRegistry(const Registry<BiomeColorEntry> &foliageColors,
                                const Registry<BiomeColorEntry> &grassColors,
                                const Registry<BiomeColorEntry> &waterColors,
                                const Registry<ColorTriangleEntry> &colorTriangles) {
        for (const auto&[type, topLeft, bottomLeft, bottomRight] : colorTriangles.entries) {
            auto &triangle = type == "foliage" ? foliageTriangle : grassTriangle;

            triangle[0] = unpackARGB(topLeft);
            triangle[1] = unpackARGB(bottomLeft);
            triangle[2] = unpackARGB(bottomRight);
        }
        setBiomeColors(foliage, foliageOverrides, foliageColors);
        setBiomeColors(grass, grassOverrides, grassColors);
        setBiomeColors(water, waterOverrides, waterColors);
    }

    auto BiomeColorRegistry::biomeColorMultiplier(const float temperature, const float downfall, const int y, const bool isGrass) const -> RGBA {
        const float localTemperature = std::clamp(temperature - static_cast<float>(y) / 600.0f, 0.0f, 1.0f);
        const float localDownfall = std::clamp(downfall, 0.0f, 1.0f) * localTemperature;

        std::array<float, 3> triangleCoordinates{};
        triangleCoordinates[0] = localDownfall;
        triangleCoordinates[1] = localTemperature - localDownfall;
        triangleCoordinates[2] = 1.0f - localTemperature;

        std::array<float, 3> color{};

        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                const auto multiplier = (isGrass ? grassTriangle : foliageTriangle)[i][j];
                const auto component = triangleCoordinates[i] * static_cast<float>(multiplier);
                color[j] += std::max(0.0f, std::min(255.0f, component));
            }
        }
        return RGBA {
            static_cast<uint8_t>(color[0]),
            static_cast<uint8_t>(color[1]),
            static_cast<uint8_t>(color[2]),
            0xFF,
        };
    }

    auto BiomeColorRegistry::setBiomeColors(BiomeColors &colors, BiomeColorOverrides &overrides, const Registry<BiomeColorEntry> &registry) -> void {
        RGBA defaultColor;

        for (size_t biomeEnv = 0; biomeEnv < BIOME_ENVIRONMENT_COUNT; biomeEnv++) {
            RGBA color = defaultColor;
            bool useOverride{};
            for (const auto&[biomeTypeStr, colorPacked] : registry.entries) {
                if (const auto biomeEnvEntry = getBiomeType(biomeTypeStr); static_cast<size_t>(biomeEnvEntry) == biomeEnv) {
                    color = unpackARGB(colorPacked);
                    useOverride = true;
                    break;
                }
            }
            if (biomeEnv == 0) defaultColor = color;
            colors[biomeEnv] = color;
            overrides[biomeEnv] = useOverride;
        }
    }
}