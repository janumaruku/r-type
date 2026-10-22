/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Logger.hpp
*/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdint>
#include <format>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

namespace engine::core::log {

enum class Level : std::uint8_t {
    EG_TRACE,
    EG_DEBUG,
    EG_INFO,
    EG_WARN,
    EG_ERROR,
    EG_OFF,
};

struct LogLine {
    Level level;
    const std::string &context;
    bool active;
    std::ostringstream buffer;
    LogLine(Level level, const std::string &context, bool active) noexcept;
    LogLine(const LogLine &) = delete;
    LogLine &operator=(const LogLine &) = delete;
    LogLine(LogLine &&other) noexcept;
    LogLine &operator=(LogLine &&) = delete;
    LogLine &operator<<(const std::string &message) &;
    LogLine &&operator<<(const std::string &message) &&;
    ~LogLine() noexcept;
};

class Logger {
public:
    Logger() = default;
    ~Logger() = default;

    explicit Logger(const std::string &context) noexcept;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    Logger(Logger &&) noexcept;
    Logger &operator=(Logger &&) noexcept;

    LogLine trace();
    LogLine debug();
    LogLine info();
    LogLine warn();
    LogLine error();

    void setThreadName(const std::string &name) noexcept;

    template<typename... Args>
    LogLine trace(const std::string &format, Args... args);
    template<typename... Args>
    LogLine debug(const std::string &format, Args... args);
    template<typename... Args>
    LogLine info(const std::string &format, Args... args);
    template<typename... Args>
    LogLine warn(const std::string &format, Args... args);
    template<typename... Args>
    LogLine error(const std::string &format, Args... args);

private:
    LogLine makeLine(Level lineLevel);

    template<typename... Args>
    LogLine formatted(Level lineLevel, const std::string &format,
        Args &... args);

    std::string _context;
    std::string _threadName;
};

void setLevel(Level threshold) noexcept;

[[nodiscard]] Level level() noexcept;

[[nodiscard]] bool isEnabled(Level lineLevel) noexcept;

[[nodiscard]] std::optional<Level> parseLevel(std::string_view name) noexcept;

/*
** Turns colours off unless the stream is std::cerr on a terminal, so a
** captured or redirected log has no escape codes. Call setColors after it
** to force them.
*/
void setOutput(std::ostream &stream);

void setColors(bool enabled);

}

#endif

#include "Logger.tpp"