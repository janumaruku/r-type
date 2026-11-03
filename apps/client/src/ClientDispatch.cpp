/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientDispatch
*/

#include "ClientDispatch.hpp"

#include <chrono>

namespace rtype::client {
namespace {
constexpr std::chrono::seconds LOG_INTERVAL{1};
}

bool ClientLogLimiter::shouldLog(protocol::DecodeError code, TimePoint now)
{
    const auto [last, inserted] = _last.try_emplace(code, now);

    if (inserted)
        return true;
    if (now - last->second < LOG_INTERVAL)
        return false;
    last->second = now;
    return true;
}
}
