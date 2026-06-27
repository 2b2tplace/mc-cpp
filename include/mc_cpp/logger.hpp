#pragma once

#include <iomanip>
#include <mutex>
#include <string_view>
#include <fmt/format.h>
#include <sstream>

namespace mc {
    static constexpr auto LEVEL_INFO_STR = "INFO";
    static constexpr auto LEVEL_WARN_STR = "WARN";
    static constexpr auto LEVEL_ERROR_STR = "ERROR";
    static constexpr auto LEVEL_FATAL_STR = "FATAL";
    static constexpr auto LEVEL_DEBUG_STR = "DEBUG";
    static constexpr auto LEVEL_UNKNOWN_STR = "UNKNOWN";

    static constexpr auto LEVEL_INFO_COLOR = 0;
    static constexpr auto LEVEL_WARN_COLOR = 33;
    static constexpr auto LEVEL_ERROR_COLOR = 31;
    static constexpr auto LEVEL_FATAL_COLOR = 31;
    static constexpr auto LEVEL_DEBUG_COLOR = 34;
    static constexpr auto LEVEL_UNKNOWN_COLOR = 37;

    enum LogLevel {
        INFO,
        WARN,
        ERROR,
        FATAL,
        DEBUG
    };

    [[nodiscard]]
    constexpr auto logLevelToString(const LogLevel level) -> std::string_view {
        switch (level) {
            case INFO: return LEVEL_INFO_STR;
            case WARN: return LEVEL_WARN_STR;
            case ERROR: return LEVEL_ERROR_STR;
            case FATAL: return LEVEL_FATAL_STR;
            case DEBUG: return LEVEL_DEBUG_STR;
        }
        return LEVEL_UNKNOWN_STR;
    }

    [[nodiscard]]
    constexpr auto logLevelToColor(const LogLevel level) -> int {
        switch (level) {
            case INFO: return LEVEL_INFO_COLOR;
            case WARN: return LEVEL_WARN_COLOR;
            case ERROR: return LEVEL_ERROR_COLOR;
            case FATAL: return LEVEL_FATAL_COLOR;
            case DEBUG: return LEVEL_DEBUG_COLOR;
        }
        return LEVEL_UNKNOWN_COLOR;
    }

    [[nodiscard]]
    auto currentTimeFormatted() -> std::string;

    class Logger {
    public:
        bool enableLogPrefix;

        explicit Logger(std::ostream &out, const bool enableLogPrefix = true): enableLogPrefix(enableLogPrefix), out(out) {}

        auto print(const std::string &logMessage, int color, bool newline = false) -> void;

        auto println(const std::string &logMessage, const int color) -> void {
            print(logMessage, color, true);
        }

        template<LogLevel L, typename... T>
        auto logInline(const bool newline, fmt::format_string<T...> fmt, T&&... args) -> void {
#ifndef DEBUG_MODE
            if constexpr (L == DEBUG) return;
#endif
            const auto logMessage = fmt::format(fmt, std::forward<T>(args)...);
            if (!enableLogPrefix) {
                print(logMessage, logLevelToColor(L), newline);
                return;
            }
            std::ostringstream msg;
            msg << "[" << currentTimeFormatted() << "] [" << logLevelToString(L) << "] " << logMessage;

            print(msg.str(), logLevelToColor(L), newline);
        }

        template<LogLevel L, typename... T>
        auto log(fmt::format_string<T...> fmt, T &&... args) -> void {
            logInline<L>(true, fmt, std::forward<T>(args)...);
        }

        [[nodiscard]]
        auto stream() const -> std::ostream& {
            return out;
        }

    private:
        std::mutex mtx;
        std::ostream &out;
    };
}
