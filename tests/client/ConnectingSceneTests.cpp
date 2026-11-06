/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingSceneTests
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "ConnectingScene.hpp"
#include "GameScene.hpp"
#include "LoopbackHelpers.hpp"
#include "engine/audio/NullAudio.hpp"
#include "engine/core/Logger.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"

using client_tests::loopbackOf;
using client_tests::occurrences;
using client_tests::rawPacket;
using client_tests::waitForDatagrams;
using engine::input::InputEvent;
using engine::input::Key;
using rtype::client::Action;
using rtype::client::Clock;
using rtype::client::ConnectingScene;
using rtype::client::GameScene;
using rtype::client::MessageScene;
using rtype::client::SceneRequest;
using rtype::protocol::Hello;
using rtype::protocol::MessageType;
using rtype::protocol::Reject;
using rtype::protocol::RejectReason;
using rtype::protocol::Welcome;

namespace {
constexpr float STEP = 0.25F;
constexpr int STEPS_TO_TIMEOUT = 20;
constexpr auto SETTLE = std::chrono::milliseconds(100);
const std::string DROPPED = "Dropped a datagram";
}

/*
** A ConnectingScene against a test server: a net::UdpSocket on loopback that
** answers by hand. The log output is captured, since it's process-wide.
*/
class Connecting: public testing::Test {
protected:
    void SetUp() override
    {
        engine::core::log::setOutput(_output);
        _server.start();
        _config.host = "127.0.0.1";
        _config.port = _server.localEndpoint().port();
        _config.name = "alice";
    }

    void TearDown() override
    {
        engine::core::log::setOutput(std::cerr);
    }

    SceneRequest update(ConnectingScene &scene, float dt)
    {
        _context.now = Clock::now();
        return scene.update(_context, dt);
    }

    /*
    ** Updates with dt = 0, so no timer moves, until the scene asks for
    ** something other than STAY or a second has passed.
    */
    SceneRequest updateUntilAnswered(ConnectingScene &scene)
    {
        const auto deadline = Clock::now() + std::chrono::seconds(1);

        while (Clock::now() < deadline) {
            SceneRequest request = update(scene, 0.0F);

            if (request.kind != SceneRequest::Kind::STAY)
                return request;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return SceneRequest::stay();
    }

    /*
    ** Updates with dt = 0 for SETTLE, so whatever was sent has arrived and
    ** been handled, and checks that the scene kept waiting.
    */
    void settle(ConnectingScene &scene)
    {
        const auto end = Clock::now() + SETTLE;

        while (Clock::now() < end) {
            ASSERT_EQ(update(scene, 0.0F).kind, SceneRequest::Kind::STAY);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    /*
    ** Enters the scene and returns the first HELLO the test server got.
    */
    net::Datagram enter(ConnectingScene &scene)
    {
        scene.onEnter(_context);

        auto hellos = waitForDatagrams(_server, 1);

        if (hellos.empty())
            throw std::runtime_error("the test server got no HELLO");
        return std::move(hellos.front());
    }

    template <class Message>
    void answer(const net::Datagram &hello, MessageType type,
        const Message &message)
    {
        _server.sendTo(hello.from,
            rtype::protocol::makePacket(type, ++_serverSequence, message));
    }

    [[nodiscard]] std::size_t droppedLines() const
    {
        return occurrences(_output.str(), DROPPED);
    }

    std::ostringstream _output;
    net::UdpSocket _server{0};
    std::uint32_t _serverSequence = 0;
    engine::input::InputState _input;
    engine::input::ActionMap<Action> _actions;
    engine::audio::NullAudio _audio;
    engine::render::TextureCache _textures;
    engine::render::FontCache _fonts;
    rtype::client::SpriteCatalog _sprites;
    rtype::client::ClientConfig _config;
    rtype::client::ClientSettings _settings;
    rtype::client::FrameContext _context{.input = _input,
        .actions = _actions,
        .audio = _audio,
        .textures = _textures,
        .fonts = _fonts,
        .sprites = _sprites,
        .config = _config,
        .settings = _settings,
        .now = Clock::now()};
};

namespace {
const MessageScene &messageOf(const SceneRequest &request)
{
    const auto *message = dynamic_cast<const MessageScene *>(
        request.next.get());

    if (message == nullptr)
        throw std::runtime_error("the request isn't a MessageScene");
    return *message;
}
}

TEST_F(Connecting, FirstHelloCarriesVersionOneAndTheName)
{
    ConnectingScene scene;
    net::Datagram hello = enter(scene);
    net::Packet packet(hello.bytes);
    rtype::protocol::Header header;

    packet >> header;
    EXPECT_EQ(header.type, MessageType::HELLO);
    EXPECT_EQ(header.sequence, 1U);
    EXPECT_EQ(rtype::protocol::decode<Hello>(packet),
        (Hello{.protocolVersion = 1, .playerName = "alice"}));
}

TEST_F(Connecting, WelcomeSwitchesToTheGameSceneWithThatPlayerId)
{
    ConnectingScene scene;
    const net::Datagram hello = enter(scene);

    answer(hello, MessageType::WELCOME, Welcome{.playerId = 2});

    const SceneRequest request = updateUntilAnswered(scene);

    ASSERT_EQ(request.kind, SceneRequest::Kind::CHANGE);

    auto *game = dynamic_cast<GameScene *>(request.next.get());

    ASSERT_NE(game, nullptr);
    EXPECT_EQ(game->playerId(), 2);
    scene.onExit(_context);
    game->onExit(_context);

    const auto goodbyes = waitForDatagrams(_server, 3);

    ASSERT_EQ(goodbyes.size(), 3U);
    EXPECT_EQ(goodbyes.front().from.port(), hello.from.port());
}

TEST_F(Connecting, SilentServerGetsTenHellosThenServerUnreachable)
{
    ConnectingScene scene;

    scene.onEnter(_context);
    for (int step = 1; step < STEPS_TO_TIMEOUT; ++step)
        ASSERT_EQ(update(scene, STEP).kind, SceneRequest::Kind::STAY)
            << "step " << step;

    const SceneRequest request = update(scene, STEP);

    ASSERT_EQ(request.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(request).title(), "Server unreachable");
    EXPECT_EQ(messageOf(request).detail(),
        "127.0.0.1:" + std::to_string(_config.port) + " did not answer");
    EXPECT_TRUE(messageOf(request).canReconnect());

    const auto hellos = waitForDatagrams(_server, 10);

    ASSERT_EQ(hellos.size(), 10U);
    for (std::uint32_t i = 0; i < hellos.size(); ++i) {
        net::Packet packet(hellos[i].bytes);
        rtype::protocol::Header header;

        packet >> header;
        EXPECT_EQ(header.type, MessageType::HELLO);
        EXPECT_EQ(header.sequence, i + 1);
    }
    std::this_thread::sleep_for(SETTLE);
    EXPECT_TRUE(_server.receiveAll().empty());
}

TEST_F(Connecting, ServerFullCanReconnectButVersionMismatchCannot)
{
    ConnectingScene full;
    const net::Datagram fullHello = enter(full);

    answer(fullHello, MessageType::REJECT,
        Reject{.reason = RejectReason::SERVER_FULL});

    const SceneRequest fullRequest = updateUntilAnswered(full);

    ASSERT_EQ(fullRequest.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(fullRequest).title(), "Server full");
    EXPECT_EQ(messageOf(fullRequest).detail(),
        "4 players are already playing");
    EXPECT_TRUE(messageOf(fullRequest).canReconnect());

    ConnectingScene outdated;
    const net::Datagram outdatedHello = enter(outdated);

    answer(outdatedHello, MessageType::REJECT,
        Reject{.reason = RejectReason::VERSION_MISMATCH});

    const SceneRequest outdatedRequest = updateUntilAnswered(outdated);

    ASSERT_EQ(outdatedRequest.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(outdatedRequest).title(), "Version mismatch");
    EXPECT_EQ(messageOf(outdatedRequest).detail(), "Update your client");
    EXPECT_FALSE(messageOf(outdatedRequest).canReconnect());
}

TEST_F(Connecting, WelcomeFromAnotherAddressIsIgnored)
{
    ConnectingScene scene;
    const net::Datagram hello = enter(scene);
    net::UdpSocket stranger{0};

    stranger.start();
    stranger.sendTo(hello.from, rtype::protocol::makePacket(
        MessageType::WELCOME, 1, Welcome{.playerId = 3}));
    settle(scene);
    answer(hello, MessageType::WELCOME, Welcome{.playerId = 1});

    const SceneRequest request = updateUntilAnswered(scene);
    const auto *game = dynamic_cast<const GameScene *>(request.next.get());

    ASSERT_NE(game, nullptr);
    EXPECT_EQ(game->playerId(), 1);
    EXPECT_EQ(droppedLines(), 0U);
}

TEST_F(Connecting, MalformedAndWrongDirectionAreDroppedAndLoggedOnce)
{
    ConnectingScene scene;
    const net::Datagram hello = enter(scene);
    const auto helloBack = rtype::protocol::makePacket(MessageType::HELLO, 1,
        Hello{.playerName = "server"});

    for (int i = 0; i < 2; ++i) {
        _server.sendTo(hello.from, rawPacket({1, 2, 3}));
        _server.sendTo(hello.from, helloBack);
    }
    settle(scene);
    EXPECT_EQ(droppedLines(), 2U);
    answer(hello, MessageType::WELCOME, Welcome{.playerId = 0});
    EXPECT_EQ(updateUntilAnswered(scene).kind, SceneRequest::Kind::CHANGE);
}

TEST_F(Connecting, UnknownHostShowsCantReachTheServer)
{
    ConnectingScene scene;

    _config.host = "no.such.host.invalid";
    scene.onEnter(_context);

    const SceneRequest request = update(scene, 0.0F);

    ASSERT_EQ(request.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(request).title(), "Can't reach the server");
    EXPECT_NE(messageOf(request).detail().find("no.such.host.invalid"),
        std::string::npos);
    EXPECT_TRUE(messageOf(request).canReconnect());
}

TEST_F(Connecting, QuitActionQuits)
{
    ConnectingScene scene;

    _actions.bind(Action::QUIT, Key::ESCAPE);
    scene.onEnter(_context);
    _input.beginFrame();
    _input.apply(InputEvent{.type = InputEvent::Type::KEY_PRESSED,
        .key = Key::ESCAPE});
    EXPECT_EQ(update(scene, 0.0F).kind, SceneRequest::Kind::QUIT);
}

TEST_F(Connecting, DrawShowsTheServerAndTheSecondsOnTheHudLayer)
{
    ConnectingScene scene;
    engine::render::RenderQueue first;
    engine::render::RenderQueue second;

    scene.onEnter(_context);
    ASSERT_EQ(update(scene, 2.0F).kind, SceneRequest::Kind::STAY);
    scene.draw(first, _context);
    scene.draw(second, _context);
    ASSERT_EQ(first.commands().size(), 1U);
    ASSERT_EQ(second.commands().size(), 1U);

    const auto &text = std::get<engine::render::TextCmd>(first.commands()[0]);
    const auto &again = std::get<engine::render::TextCmd>(
        second.commands()[0]);

    EXPECT_EQ(text.text, "Connecting to 127.0.0.1:"
        + std::to_string(_config.port) + "... 2 s");
    EXPECT_EQ(text.layer, 1000);
    EXPECT_EQ(again.text, text.text);
    EXPECT_EQ(again.layer, text.layer);
}
