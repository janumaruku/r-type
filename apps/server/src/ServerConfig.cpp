/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfig.cpp
*/

#include "ServerConfig.hpp"
#include "engine/core/Logger.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <system_error>
#include <type_traits>

namespace rtype::server {
namespace {

template <typename T>
T getValue(std::string_view option, std::string_view value)
{
    static_assert(std::is_integral_v<T>);

    T result{};
    const char* const end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(value.data(), end, result);

    if (ec != std::errc() || ptr != end)
        throw ConfigError(std::format("{}: '{}' is not a valid number", option, value));
    return result;
}

std::uint16_t parsePort(std::string_view option, std::string_view value)
{
    const auto port = getValue<std::uint16_t>(option, value);

    if (port == 0)
        throw ConfigError(std::format("{}: '{}' is not a port from 1 to 65535", option, value));
    return port;
}

engine::core::log::Level parseLogLevel(std::string_view option, std::string_view value)
{
    const auto level = engine::core::log::parseLevel(value);

    if (!level)
        throw ConfigError(std::format("{}: '{}' is not a log level", option, value));
    return *level;
}

}  // namespace

ServerConfig ServerConfig::parse(std::span<const std::string_view> args)
{
    const auto isHelp = [](std::string_view arg) { return arg == "-h" || arg == "--help"; };
    if (std::ranges::any_of(args, isHelp))
        throw HelpRequested();

    ServerConfig config = {};
    const std::size_t size = args.size();

    for (std::size_t i = 0; i < size; ++i) {
        const std::string_view arg = args[i];
        const auto nextValue = [&]() -> std::string_view {
            if (i + 1 >= size)
                throw ConfigError(std::format("{}: missing value", arg));
            return args[++i];
        };

        if (arg == "-p" || arg == "--port")
            config.port = parsePort(arg, nextValue());
        else if (arg == "--seed")
            config.seed = getValue<std::uint32_t>(arg, nextValue());
        else if (arg == "--log-level")
            config.logLevel = parseLogLevel(arg, nextValue());
        else
            throw ConfigError(std::format("{}: unknown option", arg));
    }
    return config;
}

std::string ServerConfig::usage(std::string_view program)
{
    // Replace the body with the synopsis and options table from server.md.
    return std::format(
        "Usage: {} [options]\n"
        "\n"
        "Options:\n"
        "  -p, --port <port>        UDP port, 1-65535 (default 4242)\n"
        "      --seed <seed>        Random seed, 0-4294967295\n"
        "      --log-level <level>  trace, debug, info, warn, error or off (default info)\n"
        "  -h, --help               Show this help and exit\n",
        program);
}

}  // namespace rtype::server