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
#include <vector>
#include <stdexcept>

#include "engine/core/Logger.hpp"


namespace rtype::server {

class ConfigError : public std::invalid_argument {
    using std::invalid_argument::invalid_argument;
};

class HelpRequested : public std::exception {};


struct ServerConfig {
    std::uint16_t port = 4242;
    std::optional<std::uint32_t> seed = std::nullopt;
    engine::core::log::Level logLevel = engine::core::log::Level::EG_INFO;

   // [[nodiscard]] static ServerConfig parse(int argc, char *const argv[]);
};

int parse(int argc, char *const argv[], ServerConfig &config);

}
#endif