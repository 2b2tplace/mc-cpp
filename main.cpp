#include <iostream>
#include <fmt/ranges.h>
#include <mc_cpp/logger.hpp>
#include <mc_cpp/registry/minecraft.hpp>

int main() {
    mc::Logger logger{std::cout};
    const auto res = mc::MinecraftRegistry::load("registries/765");
    if (!res.has_value()) {
        std::cerr << res.error() << '\n';
        return EXIT_FAILURE;
    }
    const auto &registry = res.value();

    const auto plainsBiomeType = registry.biomeType("plains");
    const auto grassBlock = registry.blockType("grass_block").min;
    const auto grassBlockColorPlains = registry.biomeBlockStateColor(plainsBiomeType, 69, grassBlock);

    logger.log<mc::INFO>("grass block color in plains: {}", grassBlockColorPlains);
    return EXIT_SUCCESS;
}
