/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scenes
*/

#include "Scenes.hpp"

#include <algorithm>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "ConnectingScene.hpp"

namespace rtype::client {
namespace {
constexpr std::int16_t HUD_LAYER = 1000;
constexpr float TEXT_X = 40.0F;
constexpr float TITLE_Y = 280.0F;
constexpr float LINE_GAP = 1.5F;
constexpr unsigned MAX_TEXT_SIZE = 255;

[[nodiscard]] std::uint8_t textSize(unsigned size) noexcept
{
    return static_cast<std::uint8_t>(std::min(size, MAX_TEXT_SIZE));
}

[[nodiscard]] engine::render::TextCmd hudText(std::string text,
    std::uint8_t size, float y)
{
    return {.font = {},
        .text = std::move(text),
        .characterSize = size,
        .position = {TEXT_X, y},
        .color = engine::core::Color::WHITE,
        .layer = HUD_LAYER};
}
}

std::ostream &operator<<(std::ostream &stream, const Action &action)
{
    return stream << toString(action);
}

std::string_view toString(Action action) noexcept
{
    switch (action) {
    case Action::MOVE_UP:
        return "MOVE_UP";
    case Action::MOVE_DOWN:
        return "MOVE_DOWN";
    case Action::MOVE_LEFT:
        return "MOVE_LEFT";
    case Action::MOVE_RIGHT:
        return "MOVE_RIGHT";
    case Action::FIRE:
        return "FIRE";
    case Action::TOGGLE_HUD_HELP:
        return "TOGGLE_HUD_HELP";
    case Action::QUIT:
        return "QUIT";
    case Action::CONFIRM:
        return "CONFIRM";
    }

    return "UNKNOWN";
}

bool fromString(std::string_view name, Action &action) noexcept
{
    static const std::unordered_map<std::string_view, Action> conversions = {
        {"MOVE_UP", Action::MOVE_UP},
        {"MOVE_DOWN", Action::MOVE_DOWN},
        {"MOVE_LEFT", Action::MOVE_LEFT},
        {"MOVE_RIGHT", Action::MOVE_RIGHT},
        {"FIRE", Action::FIRE},
        {"TOGGLE_HUD_HELP", Action::TOGGLE_HUD_HELP},
        {"QUIT", Action::QUIT},
        {"CONFIRM", Action::CONFIRM},
    };
    const auto it = conversions.find(name);

    if (it == conversions.end())
        return false;
    action = it->second;
    return true;
}

void bindDefaults(engine::input::ActionMap<Action> &actions)
{
    actions.bind(Action::MOVE_UP, engine::input::Key::UP);
    actions.bind(Action::MOVE_UP, engine::input::Key::KEY_W);
    actions.bind(Action::MOVE_DOWN, engine::input::Key::DOWN);
    actions.bind(Action::MOVE_DOWN, engine::input::Key::KEY_S);
    actions.bind(Action::MOVE_LEFT, engine::input::Key::LEFT);
    actions.bind(Action::MOVE_LEFT, engine::input::Key::KEY_A);
    actions.bind(Action::MOVE_RIGHT, engine::input::Key::RIGHT);
    actions.bind(Action::MOVE_RIGHT, engine::input::Key::KEY_D);
    actions.bind(Action::FIRE, engine::input::Key::SPACE);
    actions.bind(Action::FIRE, engine::input::Key::LCTRL);
    actions.bind(Action::TOGGLE_HUD_HELP, engine::input::Key::F1);
    actions.bind(Action::QUIT, engine::input::Key::ESCAPE);
    actions.bind(Action::CONFIRM, engine::input::Key::ENTER);
}

SceneRequest SceneRequest::stay() noexcept
{
    return {.kind = Kind::STAY, .next = nullptr};
}

SceneRequest SceneRequest::change(std::unique_ptr<IScene> next) noexcept
{
    if (next == nullptr)
        return stay();
    return {.kind = Kind::CHANGE, .next = std::move(next)};
}

SceneRequest SceneRequest::quit() noexcept
{
    return {.kind = Kind::QUIT, .next = nullptr};
}

void SceneManager::change(std::unique_ptr<IScene> next, FrameContext &context)
{
    if (next == nullptr)
        return;
    if (_current != nullptr)
        _current->onExit(context);
    _current = std::move(next);
    _current->onEnter(context);
}

bool SceneManager::update(FrameContext &context, float dt)
{
    if (_current == nullptr)
        return false;
    SceneRequest request = _current->update(context, dt);

    switch (request.kind) {
    case SceneRequest::Kind::STAY:
        return true;
    case SceneRequest::Kind::CHANGE:
        change(std::move(request.next), context);
        return true;
    case SceneRequest::Kind::QUIT:
        return false;
    }
    return true;
}

void SceneManager::draw(engine::render::RenderQueue &queue,
    const FrameContext &context)
{
    if (_current != nullptr)
        _current->draw(queue, context);
}

IScene *SceneManager::current() noexcept
{
    return _current.get();
}

MessageScene::MessageScene(std::string title, std::string detail,
    bool canReconnect):
    _title(std::move(title)), _detail(std::move(detail)),
    _canReconnect(canReconnect)
{
}

void MessageScene::onEnter([[maybe_unused]] FrameContext &context)
{
}

void MessageScene::onExit([[maybe_unused]] FrameContext &context)
{
}

SceneRequest MessageScene::update(FrameContext &context,
    [[maybe_unused]] float dt)
{
    if (_canReconnect && context.actions.wasTriggered(Action::CONFIRM,
            context.input))
        return SceneRequest::change(std::make_unique<ConnectingScene>());
    if (context.actions.wasTriggered(Action::QUIT, context.input))
        return SceneRequest::quit();
    return SceneRequest::stay();
}

void MessageScene::draw(engine::render::RenderQueue &queue,
    const FrameContext &context)
{
    const std::uint8_t size = textSize(context.settings.hudFontSize);
    const std::uint8_t titleSize = textSize(context.settings.hudFontSize * 2);
    const float detailY = TITLE_Y + static_cast<float>(titleSize) * LINE_GAP;
    const float hintY = detailY + static_cast<float>(size) * LINE_GAP * 2;

    queue.push(hudText(_title, titleSize, TITLE_Y));
    queue.push(hudText(_detail, size, detailY));
    queue.push(hudText(_canReconnect ? "ENTER to reconnect, ESCAPE to quit"
        : "ESCAPE to quit", size, hintY));
}

const std::string &MessageScene::title() const noexcept
{
    return _title;
}

const std::string &MessageScene::detail() const noexcept
{
    return _detail;
}

bool MessageScene::canReconnect() const noexcept
{
    return _canReconnect;
}
}
