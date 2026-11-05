/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scenes.hpp
*/

#ifndef R_TYPE_SCENES_HPP
#define R_TYPE_SCENES_HPP

#include <cstdint>
#include <iosfwd>
#include <memory>

#include "ClientConfig.hpp"
#include "ClientSettings.hpp"
#include "FontCache.hpp"
#include "RenderQueue.hpp"
#include "SpriteCatalog.hpp"
#include "TextureCache.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/input/ActionMap.hpp"
#include "engine/runtime/Clock.hpp"

namespace rtype::client {
class IScene;

enum class Action : std::uint8_t {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    FIRE,
    TOGGLE_HUD_HELP,
    QUIT,
    CONFIRM,
};

std::ostream &operator<<(std::ostream &stream, const Action &action);

[[nodiscard]] std::string_view toString(Action action) noexcept;

[[nodiscard]] bool fromString(std::string_view name, Action &action) noexcept;

void bindDefaults(engine::input::ActionMap<Action> &actions);

struct FrameContext {
    engine::input::InputState &input;
    engine::input::ActionMap<Action> &actions;
    engine::audio::IAudio &audio;
    engine::render::TextureCache &textures;
    engine::render::FontCache &fonts;
    const SpriteCatalog &sprites;
    const ClientConfig &config;
    const ClientSettings &settings;
    engine::runtime::TimePoint now;
};

struct SceneRequest {
    enum class Kind : std::uint8_t { STAY, CHANGE, QUIT };

    Kind kind = Kind::STAY;
    std::unique_ptr<IScene> next;

    [[nodiscard]] static SceneRequest stay() noexcept;

    [[nodiscard]] static SceneRequest change(
        std::unique_ptr<IScene> next) noexcept;

    [[nodiscard]] static SceneRequest quit() noexcept;
};

class IScene {
public:
    virtual ~IScene() = default;

    virtual void onEnter(FrameContext &context) = 0;

    virtual void onExit(FrameContext &context) = 0;

    [[nodiscard]] virtual SceneRequest update(FrameContext &context, float dt) =
    0;

    virtual void draw(engine::render::RenderQueue &queue,
        const FrameContext &context) = 0;
};

class SceneManager {
public:
    void change(std::unique_ptr<IScene> next, FrameContext &context);

    [[nodiscard]] bool update(FrameContext &context, float dt);

    void draw(engine::render::RenderQueue &queue, const FrameContext &context);

    [[nodiscard]] IScene *current() noexcept;

private:
    std::unique_ptr<IScene> _current;
};

class MessageScene: public IScene {
public:
    MessageScene(std::string title, std::string detail, bool canReconnect);

    void onEnter(FrameContext &context) override;

    void onExit(FrameContext &context) override;

    SceneRequest update(FrameContext &context, float dt) override;

    void draw(engine::render::RenderQueue &queue,
        const FrameContext &context) override;
};
}

#endif
