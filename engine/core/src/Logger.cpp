/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Logger.cpp
*/


#include <chrono>
#include <cstdio>
#include <format>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <utility>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "Logger.hpp"

namespace engine::core::log {

thread_local std::string thread_name = "?";

const std::map<Level, const char *> levelNames{
    {Level::TRACE, "TRACE"},
    {Level::DEBUG, "DEBUG"},
    {Level::INFO, "INFO"},
    {Level::WARN, "WARN"},
    {Level::ERROR, "ERR"},
    {Level::OFF, "OFF"},
};

const std::map<Level, const char *> levelColors{
    {Level::TRACE, "90"},
    {Level::DEBUG, "36"},
    {Level::INFO, "32"},
    {Level::WARN, "33"},
    {Level::ERROR, "31"},
};

const char *levelName(Level level) noexcept
{
    const auto it = levelNames.find(level);
    return it == levelNames.end() ? "OFF" : it->second;
}

const char *levelColor(Level level) noexcept
{
    const auto it = levelColors.find(level);
    return it == levelColors.end() ? "" : it->second;
}

bool isTerminal() noexcept
{
#ifdef _WIN32
    return _isatty(_fileno(stderr)) != 0;
#else
    return isatty(fileno(stderr)) != 0;
#endif
}

Logger::Logger(const std::string &context) noexcept : _context(context)
{}

Logger::Logger(Logger &&other)  noexcept : _context(std::move(other._context))
{}

Logger &Logger::operator=(Logger &&other)
 noexcept {
    if (this != &other) {
        _context = std::move(other._context);
    }
    return *this;
}

LogLine::LogLine(Level lineLevel, const std::string &lineContext, bool lineActive) noexcept
    : level(lineLevel), context(lineContext), active(lineActive)
{}

LogLine::LogLine(LogLine &&other) noexcept
    : level(other.level), context(other.context), active(other.active),
      buffer(std::move(other.buffer))
{
    other.active = false;
}

void Logger::setThreadName(const std::string &name) noexcept
{
    thread_name = name;
}

LogLine Logger::trace()
{
    return LogLine{Level::TRACE, _context, true};
}

LogLine Logger::debug()
{
    return LogLine{Level::DEBUG, _context, true};
}

LogLine Logger::info()
{
    return LogLine{Level::INFO, _context, true};
}

LogLine Logger::warn()
{
    return LogLine{Level::WARN, _context, true};
}

LogLine Logger::error()
{
    return LogLine{Level::ERROR, _context, true};
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

LogLine::~LogLine() noexcept
{
    if (!active)
        return;

    try {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::floor<std::chrono::milliseconds>(now);
        const std::string contextPart = context.empty() ? "" : std::format(" [{}]", context);
        const std::string line = std::format("[{:%T}] [{}] [{}]{} {}",
            timestamp, levelName(level), thread_name, contextPart, buffer.str());

        if (!isTerminal()) {
            std::cerr << line << '\n';
        } else {
            const char *color = levelColor(level);
            std::cerr << std::format("[{:%T}] [", timestamp)
                      << '\033' << "[" << color << "m"
                      << levelName(level) << "\033[37m] ["
                      << thread_name << "]" << contextPart << " "
                      << buffer.str() << "\033[0m\n";
        }
    } catch (...) {}
}
}