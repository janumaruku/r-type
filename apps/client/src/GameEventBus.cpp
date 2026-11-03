/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameEventBus.cpp
*/

#include "GameEventBus.hpp"
#include <algorithm>
#include <memory>

namespace rtype::client {

GameEventBus::Subscription::Subscription(std::size_t id,
    GameEventBus *bus) noexcept: _bus(bus), _id(id)
{}

GameEventBus::Subscription::Subscription(Subscription &&other) noexcept
    : _bus(other._bus), _id(other._id)
{
    other._bus = nullptr;
}

GameEventBus::Subscription &GameEventBus::Subscription::operator=(
    Subscription &&other) noexcept
{
    if (this != &other) {
        _bus = other._bus;
        _id  = other._id;
        other._bus = nullptr;
    }
    return *this;
}

GameEventBus::Subscription::~Subscription()
{
    if (_bus != nullptr)
        _bus->unsubscribe(_id);
}

GameEventBus::Subscription GameEventBus::subscribe(Handler handler)
{
    std::size_t id = _nextId++;
    _handlers.emplace_back(id, std::move(handler));
    return {id, this};
}

void GameEventBus::publish(const protocol::Event &event)
{
    _pending.push_back(event);
}

void GameEventBus::dispatch()
{
    for (const auto &event : _pending) {
        for (const auto &[_, handler] : _handlers) {
            handler(event);
        }
    }
    _pending.clear();
}

std::size_t GameEventBus::subscribers() const noexcept
{
    return _handlers.size();
}

void GameEventBus::unsubscribe(std::size_t id) noexcept
{
    _handlers.erase(std::ranges::remove_if(_handlers,
        [id](const auto &pair) { return pair.first == id; }).begin(),
        _handlers.end());
}

} // namespace rtype::client
