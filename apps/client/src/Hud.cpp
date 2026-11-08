/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hud.cpp
*/

#include "Hud.hpp"

#include "Actions.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "RenderSystems.hpp"

namespace rtype::client {

HudReactor::HudReactor(GameEventBus &bus, const ClientWorld &world):
    _world(world),
    _subscription(bus.subscribe([this](const protocol::Event &event) {
        onEvent(event);
    }))
{}

void HudReactor::onEvent(const protocol::Event &event)
{
    const auto playerId      = static_cast<std::uint8_t>(event.a);
    const std::string player = playerLabel(playerId);

    switch (event.type) {
    case protocol::EventType::PLAYER_JOINED:
        addBanner(player + " joined");
        break;

    case protocol::EventType::PLAYER_LEFT:
        switch (static_cast<protocol::LeaveReason>(event.b)) {
        case protocol::LeaveReason::QUIT:
            addBanner(player + " left");
            break;

        case protocol::LeaveReason::TIMEOUT:
            addBanner(player + " lost connection");
            break;

        case protocol::LeaveReason::KICKED:
            addBanner(player + " was kicked");
            break;
        }
        break;

    case protocol::EventType::PLAYER_KILLED: {
        const auto &roster = _world.roster();

        const auto *const found = std::ranges::find_if(roster,
            [&event](const protocol::PlayerInfo &slot) {
                return slot.state != protocol::PlayerState::EMPTY &&
                    slot.shipNetId == event.b;
            });

        if (found != roster.end())
            addBanner(playerLabel(found->playerId) + " was shot down");
        break;
    }

    case protocol::EventType::GAME_OVER:
        addBanner("GAME OVER - team score " +
            std::to_string(teamScore(_world.roster())));
        _gameOverIn = GAME_OVER_SECONDS;
        break;

    case protocol::EventType::GAME_RESTARTED:
        addBanner("New game");
        _gameOverIn.reset();
        break;

    default:
        break;
    }
}

void HudReactor::update(float dt)
{
    for (auto &banner: _banners)
        banner.remaining -= dt;

    while (!_banners.empty() && _banners.front().remaining <= 0.0F)
        _banners.pop_front();

    if (_gameOverIn.has_value())
        _gameOverIn = std::max(0.0F, *_gameOverIn - dt);
}

std::deque<Banner> &HudReactor::banners() noexcept
{
    return _banners;
}

std::optional<float> HudReactor::gameOverCountdown() const noexcept
{
    return _gameOverIn;
}

void HudReactor::addBanner(std::string text)
{
    if (_banners.size() >= MAX_BANNERS)
        _banners.pop_front();
    _banners.push_back({.text = std::move(text), .remaining = BANNER_SECONDS});
}

std::string playerLabel(std::uint8_t playerId)
{
    return "P" + std::to_string(static_cast<unsigned>(playerId) + 1);
}

std::string slotLine(const protocol::PlayerInfo &slot)
{
    const std::string player = playerLabel(slot.playerId);

    if (slot.state == protocol::PlayerState::EMPTY)
        return player + " -";

    const std::string name  = player + " " + slot.name;
    const std::string score = std::to_string(slot.score);

    switch (slot.state) {
    case protocol::PlayerState::ALIVE:
        return name + "  lives " + std::to_string(slot.lives) + "  " + score;

    case protocol::PlayerState::RESPAWNING:
        return name + "  respawning  " + score;

    case protocol::PlayerState::PLAYER_OUT:
        return name + "  out  " + score;

    case protocol::PlayerState::EMPTY:
        break;
    }

    return player + " -";
}

std::uint32_t teamScore(const std::array<protocol::PlayerInfo, 4> &roster)
    noexcept
{
    std::uint32_t score = 0;
    for (const auto &slot: roster)
        score += slot.score;
    return score;
}

static engine::render::TextCmd text(std::string value, std::uint8_t size,
    engine::core::Vec2f position, engine::render::TextAlign align)
{
    return {.font      = {},
        .text          = std::move(value),
        .characterSize = size,
        .position      = position,
        .color         = engine::core::Color::WHITE,
        .layer         = layers::HUD,
        .align         = align};
}

static std::string keyList(const std::vector<engine::input::Key> &keys)
{
    if (keys.empty())
        return "unbound";

    std::string list;
    for (const engine::input::Key key: keys) {
        if (!list.empty())
            list += ", ";
        list += engine::input::toString(key);
    }
    return list;
}

void hudSystem(engine::ecs::Registry &registry)
{
    auto &reactor        = registry.getResource<HudReactor>();
    const auto &state    = registry.getResource<HudState>();
    const auto &settings = registry.getResource<ClientSettings>();
    const auto &actions =
        registry.getResource<engine::input::ActionMap<Action>>();
    const ClientWorld &world = *registry.getResource<const ClientWorld *>();
    auto &queue   = registry.getResource<engine::render::RenderQueue>();
    const auto dt = registry.getResource<engine::ecs::DeltaTime>().seconds;

    const auto size = static_cast<std::uint8_t>(std::min(settings.hudFontSize,
        MAX_CHARACTER_SIZE));
    const auto lineHeight = static_cast<float>(size) * LINE_SPACING;
    const auto &roster    = world.roster();

    reactor.update(dt);

    for (std::size_t slot = 0; slot < roster.size(); ++slot) {
        queue.push(text(slotLine(roster[slot]), size,
            {MARGIN, MARGIN + (static_cast<float>(slot) * lineHeight)},
            engine::render::TextAlign::LEFT));
    }

    queue.push(text(std::format("team {}", teamScore(roster)), size,
        {RIGHT_COLUMN_X, MARGIN}, engine::render::TextAlign::LEFT));
    const std::string ping = state.ping
        ? std::format("ping {} ms", state.ping->count())
        : std::string{"ping -"};
    queue.push(text(ping, size, {RIGHT_COLUMN_X, MARGIN + lineHeight},
        engine::render::TextAlign::LEFT));

    const auto banners = reactor.banners();
    for (std::size_t index = 0; index < banners.size(); ++index) {
        queue.push(text(banners[index].text, size,
            {SCREEN_WIDTH / 2.0F,
                MARGIN + (static_cast<float>(index) * lineHeight)},
            engine::render::TextAlign::CENTER));
    }

    if (const auto countdown = reactor.gameOverCountdown()) {
        queue.push(text(std::format("GAME OVER - restarting in {}",
                            static_cast<int>(std::ceil(*countdown))),
            size, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F},
            engine::render::TextAlign::CENTER));
    }

    for (auto &&[label, transform]:
        registry.view<PlayerLabel, rtype::game::Transform>()) {
        queue.push(text(playerLabel(label.playerId), size,
            {transform.position.x(), transform.position.y() - LABEL_OFFSET},
            engine::render::TextAlign::CENTER));
    }

    if (state.showHelp) {
        const auto last    = static_cast<std::uint8_t>(Action::CONFIRM);
        const float helpTop = MARGIN +
            ((static_cast<float>(roster.size()) + HELP_GAP_LINES) * lineHeight);
        for (std::uint8_t value = 0; value <= last; ++value) {
            const auto action = static_cast<Action>(value);
            queue.push(text(std::format("{}: {}", toString(action),
                                keyList(actions.keysFor(action))),
                size,
                {HELP_X, helpTop + (static_cast<float>(value) * lineHeight)},
                engine::render::TextAlign::LEFT));
        }
    }
}

} // namespace rtype::client
