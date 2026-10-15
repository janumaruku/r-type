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
LogLine Logger::trace(const std::string &format, Args... args)
{
    return this->trace() << std::vformat(format, std::make_format_args(args...));
}

template<typename... Args>
LogLine Logger::debug(const std::string &format, Args... args)
{
    return this->debug() << std::vformat(format, std::make_format_args(args...));
}

template<typename... Args>
LogLine Logger::info(const std::string &format, Args... args)
{
    return this->info() << std::vformat(format, std::make_format_args(args...));
}

template<typename... Args>
LogLine Logger::warn(const std::string &format, Args... args)
{
    return this->warn() << std::vformat(format, std::make_format_args(args...));
}
    
template<typename... Args>
LogLine Logger::error(const std::string &format, Args... args)
{
    return this->error() << std::vformat(format, std::make_format_args(args...));
}

} // namespace engine::core::log
#endif