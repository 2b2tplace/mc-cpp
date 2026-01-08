#include <mc_cpp/logger.hpp>

namespace mc {
    auto currentTimeFormatted() -> std::string {
        const auto currentTime = time(nullptr);
        const auto localTime = *std::localtime(&currentTime);

        std::ostringstream oss;
        oss << std::put_time(&localTime, "%H:%M:%S");

        return oss.str();
    }

    auto Logger::print(const std::string &logMessage, const int color, const bool newline) -> void {
        std::lock_guard lock(mtx);
        out << "\x1B[" << color << 'm' << logMessage << "\x1B[0m";
        if (newline) out << '\n';
    }
}
