/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameEventBus.hpp
*/

#ifndef GAME_EVENT_BUS_HPP
#define GAME_EVENT_BUS_HPP

#include <cstddef>
#include <vector>
#include <functional>

#include "rtype/protocol/Event.hpp"

namespace rtype::client {
class GameEventBus {
public:
    using Handler = std::function<void(const protocol::Event &)>;

    class Subscription {
    public:
        Subscription() = default;
        Subscription(Subscription &&other) noexcept;
        Subscription &operator=(Subscription &&other) noexcept;
        Subscription(const Subscription &)            = delete;
        Subscription &operator=(const Subscription &) = delete;
        ~Subscription();

        Subscription(std::size_t id, GameEventBus *bus) noexcept;

    private:
        friend class GameEventBus;
        GameEventBus *_bus = nullptr;
        std::size_t _id    = 0;
    };

    [[nodiscard]] Subscription subscribe(Handler handler);
    void publish(const protocol::Event &event);
    void dispatch();
    [[nodiscard]] std::size_t subscribers() const noexcept;

private:
    void unsubscribe(std::size_t id) noexcept;

    std::vector<std::pair<std::size_t, Handler>> _handlers;
    std::vector<protocol::Event> _pending;
    std::size_t _nextId = 1;
};
} // namespace rtype::client

#endif /* !GAME_EVENT_BUS_HPP_ */
