/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingScene
*/

#include "ConnectingScene.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <format>
#include <memory>

#include "GameScene.hpp"
#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "ServerLink.tpp"

namespace rtype::client {
namespace {
constexpr float HELLO_INTERVAL   = 0.5F;
constexpr float CONNECT_TIMEOUT  = 5.0F;
constexpr std::int16_t HUD_LAYER = 1000;
constexpr unsigned TEXT_SIZE     = 20;
constexpr float TEXT_X           = 40.0F;
constexpr float TEXT_Y           = 340.0F;

[[nodiscard]] SceneRequest showMessage(std::string title, std::string detail,
    bool canReconnect)
{
    return SceneRequest::change(std::make_unique<MessageScene>(std::move(title),
        std::move(detail), canReconnect));
}

[[nodiscard]] SceneRequest showRejection(protocol::RejectReason reason)
{
    if (reason == protocol::RejectReason::VERSION_MISMATCH)
        return showMessage("Version mismatch", "Update your client", false);
    return showMessage("Server full", "4 players are already playing", true);
}
} // namespace

ConnectingScene::ConnectingScene()
{
    registerCommands();
}

void ConnectingScene::registerCommands() noexcept
{
    _commands.registerCreator<WelcomeCommand>(protocol::MessageType::WELCOME);
    _commands.registerCreator<RejectCommand>(protocol::MessageType::REJECT);
}

void ConnectingScene::onEnter(FrameContext &context)
{
    try {
        _link         = std::make_unique<ServerLink>();
        _link->server = resolveServer(context.config.host, context.config.port);
        _link->socket.start();
        sendHello(context);
    } catch (const engine::core::NetworkError &error) {
        _startError = error.what();
    }
}

SceneRequest ConnectingScene::update(FrameContext &context, float dt)
{
    if (_startError)
        return showMessage("Can't reach the server", *_startError, true);
    receive(context);
    if (_welcome)
        return SceneRequest::change(
            std::make_unique<GameScene>(std::move(_link), *_welcome,
                context.sprites));
    if (_rejected)
        return showRejection(*_rejected);
    _elapsed += dt;
    if (_elapsed >= CONNECT_TIMEOUT)
        return showMessage("Server unreachable",
            std::format("{}:{} did not answer", context.config.host,
                context.config.port),
            true);
    _sinceHello += dt;
    if (_sinceHello >= HELLO_INTERVAL) {
        _sinceHello -= HELLO_INTERVAL;
        sendHello(context);
    }
    if (context.actions.wasTriggered(Action::QUIT, context.input))
        return SceneRequest::quit();
    return SceneRequest::stay();
}

void ConnectingScene::draw(engine::render::RenderQueue &queue,
    const FrameContext &context)
{
    queue.push(engine::render::TextCmd{
        .font = {},
        .text = std::format("Connecting to {}:{}... {:.0f} s",
            context.config.host, context.config.port, _elapsed),
        .characterSize = static_cast<std::uint8_t>(
            std::min(context.settings.hudFontSize, TEXT_SIZE)),
        .position = {TEXT_X, TEXT_Y},
        .color = engine::core::Color::WHITE,
        .layer = HUD_LAYER,
    });
}

/*
** Runs the command of every datagram that passes checkFromServer. A message
** with no command here (a SNAPSHOT that overtook the WELCOME) is ignored.
*/
void ConnectingScene::receive(const FrameContext &context)
{
    for (const auto &datagram: _link->socket.receiveAll()) {
        auto message = checkFromServer(datagram, *_link, _logs, context.now);

        if (!message || !_commands.hasKey(message->header.type))
            continue;
        try {
            _commands.create(message->header.type)
                     ->execute(*this, message->packet);
            _link->lastHeard = context.now;
        } catch (const net::PacketError &error) {
            if (_logs.shouldLog(protocol::classify(error), context.now))
                engine::core::log::Logger("client")
                    .warn("Dropped a message from the server: {}",
                        error.what());
        }
    }
}

void ConnectingScene::sendHello(const FrameContext &context)
{
    _link->send(protocol::MessageType::HELLO,
        protocol::Hello{.playerName = context.config.name});
}

/*
** Nothing to do: on WELCOME the link was moved into the GameScene, otherwise
** it's destroyed with this scene, and destroying it stops the socket.
*/
void ConnectingScene::onExit([[maybe_unused]] FrameContext &context)
{}

void ConnectingScene::onWelcome(const protocol::Welcome &welcome)
{
    if (!_welcome)
        _welcome = welcome;
}

void ConnectingScene::onReject(protocol::RejectReason reason)
{
    _rejected = reason;
}

bool WelcomeCommand::execute(ConnectingScene &scene, net::Packet &packet)
{
    scene.onWelcome(protocol::decode<protocol::Welcome>(packet));
    return true;
}

bool WelcomeCommand::operator()(ConnectingScene &scene, net::Packet &packet)
{
    return execute(scene, packet);
}

std::unique_ptr<IConnectCommand> WelcomeCommand::create()
{
    return std::make_unique<WelcomeCommand>();
}

bool RejectCommand::execute(ConnectingScene &scene, net::Packet &packet)
{
    scene.onReject(protocol::decode<protocol::Reject>(packet).reason);
    return true;
}

bool RejectCommand::operator()(ConnectingScene &scene, net::Packet &packet)
{
    return execute(scene, packet);
}

std::unique_ptr<IConnectCommand> RejectCommand::create()
{
    return std::make_unique<RejectCommand>();
}
} // namespace rtype::client
