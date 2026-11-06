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
constexpr float CENTER_X         = 640.0F;
constexpr float TITLE_Y          = 300.0F;
constexpr float LINE_GAP         = 1.5F;
constexpr unsigned MAX_TEXT_SIZE = 255;

[[nodiscard]] std::uint8_t textSize(unsigned size) noexcept
{
    return static_cast<std::uint8_t>(std::min(size, MAX_TEXT_SIZE));
}

[[nodiscard]] engine::render::TextCmd hudText(std::string text,
    std::uint8_t size, float y)
{
    return {.font      = {},
        .text          = std::move(text),
        .characterSize = size,
        .position      = {CENTER_X, y},
        .color         = engine::core::Color::WHITE,
        .layer         = HUD_LAYER,
        .align         = engine::render::TextAlign::CENTER};
}
} // namespace

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
    if (_current != nullptr)
        _current->onExit(context);
    _current = std::move(next);
    if (_current != nullptr)
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
{}

void MessageScene::onEnter([[maybe_unused]] FrameContext &context)
{}

void MessageScene::onExit([[maybe_unused]] FrameContext &context)
{}

SceneRequest MessageScene::update(FrameContext &context,
    [[maybe_unused]] float dt)
{
    if (_canReconnect &&
        context.actions.wasTriggered(Action::CONFIRM, context.input))
        return SceneRequest::change(std::make_unique<ConnectingScene>());
    if (context.actions.wasTriggered(Action::QUIT, context.input))
        return SceneRequest::quit();
    return SceneRequest::stay();
}

void MessageScene::draw(engine::render::RenderQueue &queue,
    const FrameContext &context)
{
    const std::uint8_t size      = textSize(context.settings.hudFontSize);
    const std::uint8_t titleSize = textSize(context.settings.hudFontSize * 2);
    const float detailY = TITLE_Y + static_cast<float>(titleSize) * LINE_GAP;
    const float hintY   = detailY + static_cast<float>(size) * LINE_GAP * 2;

    queue.push(hudText(_title, titleSize, TITLE_Y));
    queue.push(hudText(_detail, size, detailY));
    queue.push(hudText(_canReconnect ? "ENTER: reconnect \u00B7 ESC: quit"
                                     : "ESC: quit",
        size, hintY));
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
} // namespace rtype::client
