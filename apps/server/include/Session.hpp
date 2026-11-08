/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Session.hpp
*/

#ifndef SESSION_HPP
#define SESSION_HPP

#include <asio.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "RateLimits.hpp"

namespace rtype::server {

struct ClientSession {
    asio::ip::udp::endpoint endpoint;
    std::uint8_t playerId = 0;
    std::string name;
    TimePoint lastHeard;
    std::uint32_t sendSequence = 0;
    WindowCounter offences{10, std::chrono::seconds(1)};
    FloodTracker datagrams;
};

class SessionManager {
public:
    static constexpr std::size_t MAX_SESSIONS = 4;
    static constexpr std::chrono::seconds TIMEOUT{5};

    /**
     * @brief Opens the session of a client whose slot was just reserved.
     *
     * The caller guarantees both preconditions, so they are asserted:
     * fewer than @ref MAX_SESSIONS sessions exist, and `endpoint` has none.
     * @return The new session, valid until it is removed.
     */
    ClientSession &add(const asio::ip::udp::endpoint &endpoint,
        std::uint8_t playerId, std::string name, TimePoint now);
    [[nodiscard]] ClientSession *find(const asio::ip::udp::endpoint &endpoint);
    void remove(const asio::ip::udp::endpoint &endpoint);
    [[nodiscard]] std::vector<asio::ip::udp::endpoint> timedOut(TimePoint now)
        const;
    [[nodiscard]] std::map<asio::ip::udp::endpoint, ClientSession> &all()
        noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::map<asio::ip::udp::endpoint, ClientSession> _sessions;
};
} // namespace rtype::server

#endif
