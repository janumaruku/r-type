/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientConfig.hpp
*/

#ifndef R_TYPE_CLIENTCONFIG_HPP
#define R_TYPE_CLIENTCONFIG_HPP

#include "engine/core/Logger.hpp"
#include <string>
#include <cstdint>
#include <span>

namespace rtype::client {
struct ClientConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 4242;
    std::string name;
    bool fullscreen = false;
    engine::core::log::Level logLevel = engine::core::log::Level::EG_INFO;

    [[nodiscard]] static ClientConfig parse(
        std::span<const std::string_view> args);

    // [[nodiscard]] static std::string  usage(std::string_view program);
    [[nodiscard]] static std::string defaultName();
};


class ConfigError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class HelpRequested: public std::exception {
};

}
#endif // namespace rtype::client
