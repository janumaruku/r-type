/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Logger.tpp
*/

#ifndef LOGGER_TPP
#define LOGGER_TPP

#include "Logger.hpp"

namespace engine::core::log {

template<typename... Args>
LogLine Logger::formatted(Level lineLevel, const std::string &format,
    Args &... args)
{
    LogLine line = makeLine(lineLevel);

    if (line.active)
        line << std::vformat(format, std::make_format_args(args...));
    return line;
}

template<typename... Args>
LogLine Logger::trace(const std::string &format, Args... args)
{
    return formatted(Level::EG_TRACE, format, args...);
}

template<typename... Args>
LogLine Logger::debug(const std::string &format, Args... args)
{
    return formatted(Level::EG_DEBUG, format, args...);
}

template<typename... Args>
LogLine Logger::info(const std::string &format, Args... args)
{
    return formatted(Level::EG_INFO, format, args...);
}

template<typename... Args>
LogLine Logger::warn(const std::string &format, Args... args)
{
    return formatted(Level::EG_WARN, format, args...);
}

template<typename... Args>
LogLine Logger::error(const std::string &format, Args... args)
{
    return formatted(Level::EG_ERROR, format, args...);
}

} // namespace engine::core::log
#endif