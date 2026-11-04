/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Session.cpp
*/

#include "Session.hpp"

#include <cassert>
#include <chrono>
#include <utility>

#include "RateLimits.hpp"

namespace rtype::server {

ClientSession &SessionManager::add(const asio::ip::udp::endpoint &endpoint,
    std::uint8_t playerId, std::string name, TimePoint now)
{
    assert(_sessions.size() < MAX_SESSIONS && "no free slot was reserved");
    assert(!_sessions.contains(endpoint) && "the endpoint already has one");

    return _sessions.emplace(endpoint, ClientSession{
        .endpoint  = endpoint,
        .playerId  = playerId,
        .name      = std::move(name),
        .lastHeard = now,
        .datagrams = {},
    }).first->second;
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
