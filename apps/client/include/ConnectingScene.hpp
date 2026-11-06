/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingScene
*/

#ifndef R_TYPE_CONNECTING_SCENE_HPP
#define R_TYPE_CONNECTING_SCENE_HPP

#include "../../../engine/render/include/engine/render/RenderQueue.hpp"
#include "ClientConfig.hpp"
#include "ClientDispatch.hpp"
#include "rtype/protocol/Messages.hpp"
#include "Scenes.hpp"
#include "ServerLink.hpp"

namespace rtype::client {

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

    /**
     * @brief Handles the server's answers, then moves on: to the game on
     * WELCOME, to a message on REJECT, a start error or after 5 s of
     * silence, and out on QUIT. Resends HELLO every 0.5 s meanwhile.
     */
    [[nodiscard]] SceneRequest update(FrameContext &context, float dt) override;

    /**
     * @brief "Connecting to host:port..." with the seconds waited, on the
     * HUD layer. Changes nothing, so drawing twice gives the same picture.
     */
    void draw(engine::render::RenderQueue &queue,
        const FrameContext &context) override;

private:
    void registerCommands() noexcept;

    void receive(const FrameContext &context);

    void sendHello(const FrameContext &context);

    std::unique_ptr<ServerLink> _link;
    ConnectCommandFactory _commands;
    float _sinceHello = 0.0F;
    float _elapsed    = 0.0F;
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

/*
** TODO(RTP-126): temporary stub of GameScene, so ConnectingScene::update can
** hand it the link on WELCOME. Delete it once RTP-126 is merged into dev;
** the real constructor also takes `const SpriteCatalog &` (RTP-122).
*/
class GameScene: public IScene {
public:
    GameScene(std::unique_ptr<ServerLink> link, protocol::Welcome welcome):
        _link(std::move(link)), _welcome(welcome)
    {}

    void onEnter([[maybe_unused]] FrameContext &context) override
    {}

    void onExit([[maybe_unused]] FrameContext &context) override
    {}

    [[nodiscard]] SceneRequest update([[maybe_unused]] FrameContext &context,
        [[maybe_unused]] float dt) override
    {
        return {};
    }

    void draw([[maybe_unused]] engine::render::RenderQueue &queue,
        [[maybe_unused]] const FrameContext &context) override
    {}

    [[nodiscard]] const ServerLink &link() const noexcept
    {
        return *_link;
    }

    [[nodiscard]] const protocol::Welcome &welcome() const noexcept
    {
        return _welcome;
    }

private:
    std::unique_ptr<ServerLink> _link;
    protocol::Welcome _welcome;
};
} // namespace rtype::client

#endif // R_TYPE_CONNECTING_SCENE_HPP
