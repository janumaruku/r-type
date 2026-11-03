/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingScene
*/

#ifndef R_TYPE_CONNECTING_SCENE_HPP
#define R_TYPE_CONNECTING_SCENE_HPP
#include "ClientDispatch.hpp"
#include "FontCache.hpp"
#include "RenderQueue.hpp"
#include "ServerLink.hpp"
#include "TextureCache.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/input/ActionMap.hpp"
#include "rtype/protocol/Messages.hpp"

namespace rtype::client {
/*
** TODO(RTP-121): temporary copies of Actions.hpp and Scenes.hpp, from the
** class IScene forward declaration down to the end of IScene. Delete them
** all once RTP-121 is merged into dev, and include those two headers instead
** (along with FontCache.hpp, RenderQueue.hpp, TextureCache.hpp, IAudio.hpp
** and ActionMap.hpp, which only these copies need).
*/
class IScene;

/*
** TODO(RTP-121): temporary, belongs to Actions.hpp.
*/
enum class Action : std::uint8_t {
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, FIRE, TOGGLE_HUD_HELP, QUIT,
    CONFIRM,
};

/*
** TODO(RTP-121): temporary, belongs to Actions.hpp.
*/
[[nodiscard]] std::string_view toString(Action action) noexcept;

/*
** TODO(RTP-121): temporary, belongs to Actions.hpp.
*/
[[nodiscard]] bool fromString(std::string_view name, Action &action) noexcept;

/*
** TODO(RTP-121): temporary, belongs to Actions.hpp.
*/
void bindDefaults(engine::input::ActionMap<Action> & actions);

/*
** TODO(RTP-120): temporary stub with only the fields ConnectingScene reads.
** Delete it once RTP-120 is merged into dev and include ClientConfig.hpp.
*/
struct ClientConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 4242;
    std::string name;
};

/*
** TODO(RTP-121): temporary, belongs to Scenes.hpp.
** TODO(RTP-122): the real one has `const SpriteCatalog &sprites`.
** TODO(RTP-120): the real one also has `const ClientSettings &settings`.
*/
struct FrameContext {
    engine::input::InputState &input;
    engine::input::ActionMap<Action> &actions;
    engine::audio::IAudio &audio;
    engine::render::TextureCache &textures;
    engine::render::FontCache &fonts;
    // const SpriteCatalog &sprites;
    const ClientConfig &config;
    // const ClientSettings &settings;
    TimePoint now;
};

/*
** TODO(RTP-121): temporary, belongs to Scenes.hpp.
*/
struct SceneRequest {
    enum class Kind : std::uint8_t { STAY, CHANGE, QUIT };

    Kind kind = Kind::STAY;
    std::unique_ptr<IScene> next;

    [[nodiscard]] static SceneRequest stay();

    [[nodiscard]] static SceneRequest change(std::unique_ptr<IScene> next);

    [[nodiscard]] static SceneRequest quit();
};

/*
** TODO(RTP-121): temporary, belongs to Scenes.hpp.
*/
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

class ConnectingScene: public IScene {
public:
    ConnectingScene();

    void onWelcome(const protocol::Welcome &welcome);

    void onReject(protocol::RejectReason reason);

    /**
     * @brief Opens the link: resolves the server, starts the socket and
     * sends the first HELLO. A @ref engine::core::NetworkError is kept in
     * `_startError` for the first @ref update to show.
     */
    void onEnter(FrameContext &context) override;

    void onExit(FrameContext &context) override;

    // the 3 other IScene functions
private:
    void registerCommands() noexcept;

    std::unique_ptr<ServerLink> _link;
    ConnectCommandFactory _commands;
    float _sinceHello = 0.0F;
    float _elapsed = 0.0F;
    std::optional<protocol::Welcome> _welcome;
    std::optional<protocol::RejectReason> _rejected;
    std::optional<std::string> _startError;
    ClientLogLimiter _logs;
};

/**
 * @brief Decodes a WELCOME and hands it to @ref ConnectingScene::onWelcome.
 */
class WelcomeCommand: public IConnectCommand {
public:
    bool execute(ConnectingScene &scene, net::Packet &packet) override;

    bool operator()(ConnectingScene &scene, net::Packet &packet) override;

    [[nodiscard]] static std::unique_ptr<IConnectCommand> create();
};

/**
 * @brief Decodes a REJECT and hands its reason to
 * @ref ConnectingScene::onReject.
 */
class RejectCommand: public IConnectCommand {
public:
    bool execute(ConnectingScene &scene, net::Packet &packet) override;

    bool operator()(ConnectingScene &scene, net::Packet &packet) override;

    [[nodiscard]] static std::unique_ptr<IConnectCommand> create();
};
}

#endif //R_TYPE_CONNECTING_SCENE_HPP
