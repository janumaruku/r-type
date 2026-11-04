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

bool EventFilter::firstTime(std::uint32_t eventId)
{
    auto it = std::ranges::find(_recent, eventId);

    if (it != _recent.end())
        return false;
    _recent.push_back(eventId);

    if (_recent.size() > CAPACITY)
        _recent.pop_front();

    return true;
}

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
        if (_bus != nullptr)
            _bus->unsubscribe(_id);
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
    std::vector<protocol::Event> events;
    events.swap(_pending);

    std::vector<std::size_t> handlerIds;
    handlerIds.reserve(_handlers.size());
    for (const auto &[id, _] : _handlers)
        handlerIds.push_back(id);

    for (const auto &event : events) {
        for (const auto id : handlerIds) {
            const auto it = std::ranges::find_if(_handlers,
                [id](const auto &entry) { return entry.first == id; });
            if (it == _handlers.end())
                continue;
            auto handler = it->second;
            handler(event);
        }
    }
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
