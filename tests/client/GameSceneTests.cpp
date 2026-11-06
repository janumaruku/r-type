/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameSceneTests
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "Actions.hpp"
#include "GameScene.hpp"
#include "LoopbackHelpers.hpp"
#include "engine/audio/NullAudio.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"

using client_tests::loopbackOf;
using client_tests::waitForDatagrams;
using engine::input::InputEvent;
using engine::input::Key;
using rtype::client::Action;
using rtype::client::Clock;
using rtype::client::GameScene;
using rtype::client::MessageScene;
using rtype::client::SceneRequest;
using rtype::client::ServerLink;
using rtype::protocol::MessageType;

namespace {
constexpr auto QUIET = std::chrono::milliseconds(100);

[[nodiscard]] MessageType typeOf(const net::Datagram &datagram)
{
    net::Packet packet(datagram.bytes);
    rtype::protocol::Header header;

    packet >> header;
    return header.type;
}

[[nodiscard]] std::size_t countOf(const std::vector<net::Datagram> &datagrams,
    MessageType type)
{
    std::size_t count = 0;

    for (const auto &datagram: datagrams)
        if (typeOf(datagram) == type)
            ++count;
    return count;
}

[[nodiscard]] const MessageScene &messageOf(const SceneRequest &request)
{
    const auto *message = dynamic_cast<const MessageScene *>(
        request.next.get());

    if (message == nullptr)
        throw std::runtime_error("the request isn't a MessageScene");
    return *message;
}
}

/*
** A GameScene whose link points at a test server: a net::UdpSocket on
** loopback that reads what the scene sends and answers by hand. The
** catalog is empty, since nothing here spawns an entity.
*/
class Game: public testing::Test {
protected:
    void SetUp() override
    {
        _server.start();
        rtype::client::bindDefaults(_actions);

        auto link = std::make_unique<ServerLink>();

        link->server = loopbackOf(_server);
        link->socket.start();
        link->lastHeard = Clock::now();
        _client = loopbackOf(link->socket);
        _scene = std::make_unique<GameScene>(std::move(link),
            rtype::protocol::Welcome{.playerId = 0}, _sprites);
        _scene->onEnter(_context);
    }

    SceneRequest update(float dt)
    {
        _context.now = Clock::now();
        return _scene->update(_context, dt);
    }

    SceneRequest updateUntilAnswered()
    {
        const auto deadline = Clock::now() + std::chrono::seconds(1);

        while (Clock::now() < deadline) {
            SceneRequest request = update(0.0F);

            if (request.kind != SceneRequest::Kind::STAY)
                return request;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return SceneRequest::stay();
    }

    template <class Message>
    void send(MessageType type, const Message &message)
    {
        _server.sendTo(_client,
            rtype::protocol::makePacket(type, ++_serverSequence, message));
    }

    void press(Key key)
    {
        _input.beginFrame();
        _input.apply(InputEvent{.type = InputEvent::Type::KEY_PRESSED,
            .key = key});
    }

    /*
    ** Runs 2 s of frames of `dt` and returns every INPUT sequence the test
    ** server got, in order.
    */
    std::vector<std::uint32_t> inputsOverTwoSeconds(int framesPerSecond)
    {
        const float dt = 1.0F / static_cast<float>(framesPerSecond);
        std::vector<std::uint32_t> sequences;

        for (int frame = 0; frame < framesPerSecond * 2; ++frame)
            EXPECT_EQ(update(dt).kind, SceneRequest::Kind::STAY);
        for (auto &datagram: waitForDatagrams(_server, 121, QUIET * 5)) {
            net::Packet packet(datagram.bytes);
            rtype::protocol::Header header;

            packet >> header;
            if (header.type == MessageType::INPUT)
                sequences.push_back(rtype::protocol::decode<
                    rtype::protocol::Input>(packet).inputSequence);
        }
        return sequences;
    }

    net::UdpSocket _server{0};
    asio::ip::udp::endpoint _client;
    std::uint32_t _serverSequence = 0;
    std::unique_ptr<GameScene> _scene;
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

class GameInputRate: public Game,
    public testing::WithParamInterface<int> {};

TEST_P(GameInputRate, SendsOneHundredTwentyInputsInTwoSeconds)
{
    std::vector<std::uint32_t> expected(120);

    for (std::uint32_t index = 0; index < expected.size(); ++index)
        expected[index] = index + 1;
    EXPECT_EQ(inputsOverTwoSeconds(GetParam()), expected);
}

INSTANTIATE_TEST_SUITE_P(FrameRates, GameInputRate,
    testing::Values(30, 60, 144));

TEST_F(Game, SilentServerGivesConnectionLostAndNoDisconnect)
{
    _context.now = Clock::now() + std::chrono::seconds(6);

    const SceneRequest request = _scene->update(_context, 0.0F);

    ASSERT_EQ(request.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(request).title(), "Connection lost");
    _scene->onExit(_context);
    EXPECT_EQ(countOf(waitForDatagrams(_server, 1, QUIET),
        MessageType::DISCONNECT), 0U);
}

TEST_F(Game, QuitThenOnExitSendsThreeDisconnects)
{
    press(Key::ESCAPE);
    EXPECT_EQ(update(0.0F).kind, SceneRequest::Kind::QUIT);
    _scene->onExit(_context);
    EXPECT_EQ(countOf(waitForDatagrams(_server, 3),
        MessageType::DISCONNECT), 3U);
}

TEST_F(Game, WindowClosingSendsThreeDisconnects)
{
    _scene->onExit(_context);
    EXPECT_EQ(countOf(waitForDatagrams(_server, 3),
        MessageType::DISCONNECT), 3U);
}

TEST_F(Game, ShutdownThenKickShowsServerClosedAndNoDisconnect)
{
    send(MessageType::SHUTDOWN, rtype::protocol::Shutdown{});
    send(MessageType::KICK, rtype::protocol::Kick{
        .reason = rtype::protocol::KickReason::RATE_LIMITED});
    std::this_thread::sleep_for(QUIET);

    const SceneRequest request = updateUntilAnswered();

    ASSERT_EQ(request.kind, SceneRequest::Kind::CHANGE);
    EXPECT_EQ(messageOf(request).title(), "Server closed");
    _scene->onExit(_context);
    EXPECT_EQ(countOf(waitForDatagrams(_server, 1, QUIET),
        MessageType::DISCONNECT), 0U);
}
