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
#include <string>

#include "Actions.hpp"
#include "ClientConfig.hpp"
#include "ClientSettings.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/input/ActionMap.hpp"
#include "engine/render/FontCache.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/render/TextureCache.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/runtime/Clock.hpp"
#include "SpriteCatalog.hpp"

namespace rtype::client {
class IScene;

struct FrameContext {
    engine::input::InputState &input;
    engine::input::ActionMap<Action> &actions;
    engine::audio::IAudio &audio;
    engine::render::TextureCache &textures;
    engine::render::FontCache &fonts;
    const SpriteCatalog &sprites;
    engine::render::TextureId whitePixel;
    const ClientConfig &config;
    const ClientSettings &settings;
    engine::runtime::TimePoint now;
};

struct SceneRequest {
    enum class Kind : std::uint8_t { STAY, CHANGE, QUIT };

    Kind kind = Kind::STAY;
    std::unique_ptr<IScene> next;

    [[nodiscard]] static SceneRequest stay() noexcept;

    [[nodiscard]] static SceneRequest change(std::unique_ptr<IScene> next)
        noexcept;

    [[nodiscard]] static SceneRequest quit() noexcept;
};

class IScene {
public:
    virtual ~IScene() = default;

    virtual void onEnter(FrameContext &context) = 0;

    virtual void onExit(FrameContext &context) = 0;

    [[nodiscard]] virtual SceneRequest update(FrameContext &context,
        float dt) = 0;

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

/**
 * @brief A title and a detail line ("Server full", "Connection lost"),
 * with a hint for the keys. CONFIRM reconnects through a new
 * ConnectingScene when @p canReconnect is true, QUIT quits.
 */
class MessageScene: public IScene {
public:
    MessageScene(std::string title, std::string detail, bool canReconnect);

    void onEnter(FrameContext &context) override;

    void onExit(FrameContext &context) override;

    [[nodiscard]] SceneRequest update(FrameContext &context, float dt) override;

    /**
     * @brief The title, the detail and the key hint on the HUD layer, sized
     * from the HUD font size setting.
     */
    void draw(engine::render::RenderQueue &queue,
        const FrameContext &context) override;

    [[nodiscard]] const std::string &title() const noexcept;

    [[nodiscard]] const std::string &detail() const noexcept;

    [[nodiscard]] bool canReconnect() const noexcept;

private:
    std::string _title;
    std::string _detail;
    bool _canReconnect;
};
} // namespace rtype::client

#endif
