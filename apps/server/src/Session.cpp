/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Session.cpp
*/

#include "Session.hpp"

#include <chrono>

#include "RateLimits.hpp"

namespace rtype::server {

ClientSession &SessionManager::add(const asio::ip::udp::endpoint &endpoint,
    std::uint8_t playerId, const std::string &name, TimePoint now)
{
    ClientSession session{
        .endpoint  = endpoint,
        .playerId  = playerId,
        .name      = name,
        .lastHeard = now,
        .datagrams = {},
    };
    _sessions.emplace(endpoint, session);
    return _sessions.at(endpoint);
}

ClientSession *SessionManager::find(const asio::ip::udp::endpoint &endpoint)
{
    auto it = _sessions.find(endpoint);

    if (it == _sessions.end())
        return nullptr;
    return &it->second;
}

void SessionManager::remove(const asio::ip::udp::endpoint &endpoint)
{
    auto it = _sessions.find(endpoint);

    if (it == _sessions.end())
        return;
    _sessions.erase(it);
}

std::vector<asio::ip::udp::endpoint> SessionManager::timedOut(TimePoint now)
    const
{
    std::vector<asio::ip::udp::endpoint> result;

    for (const auto &[endpoint, session]: _sessions) {
        if (now - session.lastHeard >= TIMEOUT)
            result.push_back(endpoint);
    }

    return result;
}

std::map<asio::ip::udp::endpoint, ClientSession> &SessionManager::all() noexcept
{
    return _sessions;
}

std::size_t SessionManager::size() const noexcept
{
    return _sessions.size();
}

} // namespace rtype::server
