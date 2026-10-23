/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Logger.cpp
*/


#include <atomic>
#include <chrono>
#include <cstdio>
#include <format>
#include <iostream>
#include <map>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "../include/engine/core/Logger.hpp"

namespace engine::core::log {

thread_local std::string threadName = "?";

const std::map<Level, const char *> LEVEL_NAMES{
    {Level::EG_TRACE, "TRACE"},
    {Level::EG_DEBUG, "DEBUG"},
    {Level::EG_INFO, "INFO"},
    {Level::EG_WARN, "WARN"},
    {Level::EG_ERROR, "ERR"},
    {Level::EG_OFF, "OFF"},
};

const std::map<Level, const char *> LEVEL_COLORS{
    {Level::EG_TRACE, "90"},
    {Level::EG_DEBUG, "36"},
    {Level::EG_INFO, "32"},
    {Level::EG_WARN, "33"},
    {Level::EG_ERROR, "31"},
};

const char *levelName(Level level) noexcept
{
    const auto it = LEVEL_NAMES.find(level);
    return it == LEVEL_NAMES.end() ? "OFF" : it->second;
}

const char *levelColor(Level level) noexcept
{
    const auto it = LEVEL_COLORS.find(level);
    return it == LEVEL_COLORS.end() ? "" : it->second;
}

bool isTerminal() noexcept
{
    #ifdef _WIN32
    return _isatty(_fileno(stderr)) != 0;
    #else
    return isatty(fileno(stderr)) != 0;
    #endif
}

/*
** Built on first use, so a Logger used during static initialization never
** sees it uninitialized. The level is read without the lock on every call;
** the output and the colours are only touched under the mutex.
*/
struct State {
    std::atomic<Level> level{Level::EG_INFO};
    std::mutex mutex;
    std::ostream *output = &std::cerr;
    bool colors = isTerminal();
};

State &state()
{
    static State instance;

    return instance;
}

void setLevel(Level threshold) noexcept
{
    state().level.store(threshold, std::memory_order_relaxed);
}

Level level() noexcept
{
    return state().level.load(std::memory_order_relaxed);
}

bool isEnabled(Level lineLevel) noexcept
{
    return lineLevel != Level::EG_OFF && lineLevel >= level();
}

std::optional<Level> parseLevel(std::string_view name) noexcept
{
    static constexpr std::pair<std::string_view, Level> NAMES[]{
        {"trace", Level::EG_TRACE},
        {"debug", Level::EG_DEBUG},
        {"info", Level::EG_INFO},
        {"warn", Level::EG_WARN},
        {"error", Level::EG_ERROR},
        {"off", Level::EG_OFF},
    };

    for (const auto &[text, value]: NAMES)
        if (text == name)
            return value;
    return std::nullopt;
}

void setOutput(std::ostream &stream)
{
    State &shared = state();
    const std::lock_guard lock(shared.mutex);

    shared.output = &stream;
    shared.colors = &stream == &std::cerr && isTerminal();
}

void setColors(bool enabled)
{
    State &shared = state();
    const std::lock_guard lock(shared.mutex);

    shared.colors = enabled;
}

Logger::Logger(const std::string &context) noexcept: _context(context)
{
}

Logger::Logger(Logger &&other) noexcept: _context(std::move(other._context))
{
}

Logger &Logger::operator=(Logger &&other)
    noexcept
{
    if (this != &other) {
        _context = std::move(other._context);
    }
    return *this;
}

LogLine::LogLine(Level lineLevel, const std::string &lineContext,
    bool lineActive) noexcept
    : level(lineLevel), context(lineContext), active(lineActive)
{
}

LogLine::LogLine(LogLine &&other) noexcept
    : level(other.level), context(other.context), active(other.active),
      buffer(std::move(other.buffer))
{
    other.active = false;
}

void Logger::setThreadName(const std::string &name) noexcept
{
    threadName = name;
}

LogLine Logger::makeLine(Level lineLevel)
{
    return LogLine{lineLevel, _context, isEnabled(lineLevel)};
}

LogLine Logger::trace()
{
    return makeLine(Level::EG_TRACE);
}

LogLine Logger::debug()
{
    return makeLine(Level::EG_DEBUG);
}

LogLine Logger::info()
{
    return makeLine(Level::EG_INFO);
}

LogLine Logger::warn()
{
    return makeLine(Level::EG_WARN);
}

LogLine Logger::error()
{
    return makeLine(Level::EG_ERROR);
}


LogLine &LogLine::operator<<(const std::string &message) &
{
    if (active)
        buffer << message;
    return *this;
}

LogLine &&LogLine::operator<<(const std::string &message) &&
{
    if (active)
        buffer << message;
    return std::move(*this);
}

/*
** One write per line, under the lock, so lines from the network and game
** threads never mix. ERR is red from start to end; the other levels colour
** the prefix only.
*/
LogLine::~LogLine() noexcept
{
    if (!active)
        return;

    try {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::floor<
            std::chrono::milliseconds>(now);
        const std::string contextPart = context.empty()
            ? ""
            : std::format(" [{}]", context);
        const std::string prefix = std::format("[{:%T}] [{}] [{}]{}",
            timestamp, levelName(level), threadName, contextPart);
        const std::string message = buffer.str();
        State &shared = state();
        const std::lock_guard lock(shared.mutex);

        if (!shared.colors)
            *shared.output << std::format("{} {}\n", prefix, message);
        else if (level == Level::EG_ERROR)
            *shared.output << std::format("\033[{}m{} {}\033[0m\n",
                levelColor(level), prefix, message);
        else
            *shared.output << std::format("\033[{}m{}\033[0m {}\n",
                levelColor(level), prefix, message);
    } catch (...) {
    }
}
}
