/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfig.hpp
*/

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <span>
#include <stdexcept>

#include "engine/core/Logger.hpp"


namespace rtype::server {

constexpr std::array<std::pair<std::string_view, engine::core::log::Level>, 6> LEVEL_STR = {{
    {"trace", engine::core::log::Level::EG_TRACE},
    {"debug", engine::core::log::Level::EG_DEBUG},
    {"info",  engine::core::log::Level::EG_INFO},
    {"warn",  engine::core::log::Level::EG_WARN},
    {"error", engine::core::log::Level::EG_ERROR},
    {"off",   engine::core::log::Level::EG_OFF},
},};

class ConfigError : public std::invalid_argument {
    using std::invalid_argument::invalid_argument;
};

class HelpRequested : public std::exception {};


struct ServerConfig {
    std::uint16_t port = 4242;
    std::optional<std::uint32_t> seed = std::nullopt;
    engine::core::log::Level logLevel = engine::core::log::Level::EG_INFO;

    [[nodiscard]] static ServerConfig parse(std::span<const std::string_view> args);
    [[nodiscard]] static std::string usage(std::string_view program);
};

}
#endif