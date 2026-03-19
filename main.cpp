#include <iostream>
#include <fmt/ranges.h>
#include <mc_cpp/logger.hpp>
#include <mc_cpp/registry/minecraft.hpp>

int main() {
    mc::Logger log{std::cout};
    log.log<mc::INFO>("Hello, {}!", "world");
    return EXIT_SUCCESS;
}
