/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerLink
*/

#include "ServerLink.hpp"

#include <string>

#include "engine/core/Errors.hpp"

namespace rtype::client {

asio::ip::udp::endpoint resolveServer(const std::string &host,
    std::uint16_t port)
{
    asio::io_context context;
    asio::ip::udp::resolver resolver(context);
    asio::error_code error;
    const auto results = resolver.resolve(asio::ip::udp::v4(), host,
        std::to_string(port), error);

    if (error || results.empty())
        throw engine::core::NetworkError("cannot resolve server '" + host
            + "'" + (error ? ": " + error.message() : ""));
    return results.begin()->endpoint();
}
}
