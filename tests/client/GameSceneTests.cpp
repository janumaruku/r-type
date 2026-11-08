/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameSceneTests
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "Actions.hpp"
#include "GameScene.hpp"
#include "LoopbackHelpers.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/audio/Volume.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"

using client_tests::loopbackOf;
using client_tests::waitForDatagrams;
using engine::audio::MusicId;
using engine::audio::SoundId;
using engine::input::InputEvent;
using engine::input::Key;
using rtype::client::Action;
using rtype::client::Clock;
using rtype::client::GameScene;
using rtype::client::MessageScene;
using rtype::client::SceneRequest;
using rtype::client::ServerLink;
using rtype::protocol::EventType;
using rtype::protocol::MessageType;

namespace {
constexpr auto QUIET = std::chrono::milliseconds(100);
constexpr SoundId JOIN_SOUND{4};
constexpr MusicId STAGE_MUSIC{1};

/** An IAudio that records the sounds and music the scene plays. */
class RecordingAudio final: public engine::audio::IAudio {
public:
    SoundId loadSound(const std::filesystem::path &) override
    {
        return SoundId{};
    }
    MusicId loadMusic(const std::filesystem::path &) override
    {
        return MusicId{};
    }
    void playSound(SoundId id, float) override
    {
        sounds.push_back(id);
    }
    void playMusic(MusicId id, bool loop) override
    {
        music     = id;
        looping   = loop;
        ++musicStarts;
    }
    void stopMusic() override
    {
        ++musicStops;
    }
    void setMasterVolume(float) override
    {}
    void setSfxVolume(float) override
    {}
    void setMusicVolume(float) override
    {}
    void setMuted(bool) override
    {}
    [[nodiscard]] const engine::audio::Volume &volumes() const override
    {
        return _volumes;
    }
    void update() override
    {}

    std::vector<SoundId> sounds;
    MusicId music;
    bool looping    = false;
    int musicStarts = 0;
    int musicStops  = 0;

private:
    engine::audio::Volume _volumes;
};

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
        _context.sounds.join   = JOIN_SOUND;
        _context.sounds.stage1 = STAGE_MUSIC;
        _scene = std::make_unique<GameScene>(std::move(link),
            rtype::protocol::Welcome{.playerId = 0}, _context);
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

    /* Updates the scene for `duration`, so it reads what the server sent. */
    void updateFor(std::chrono::milliseconds duration)
    {
        const auto deadline = Clock::now() + duration;

        while (Clock::now() < deadline) {
            EXPECT_EQ(update(0.0F).kind, SceneRequest::Kind::STAY);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    /* Draws one frame and returns the text of every TextCmd in it. */
    std::vector<std::string> drawnTexts()
    {
        engine::render::RenderQueue queue;
        std::vector<std::string> texts;

        _scene->draw(queue, _context);
        for (const auto &command: queue.commands()) {
            if (const auto *text =
                    std::get_if<engine::render::TextCmd>(&command))
                texts.push_back(text->text);
        }
        return texts;
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

    void release(Key key)
    {
        _input.beginFrame();
        _input.apply(InputEvent{.type = InputEvent::Type::KEY_RELEASED,
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
    RecordingAudio _audio;
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
        .whitePixel = {},
        .sounds = {},
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

TEST_F(Game, EnteringStartsTheLoopingStageMusic)
{
    EXPECT_EQ(_audio.musicStarts, 1);
    EXPECT_EQ(_audio.music, STAGE_MUSIC);
    EXPECT_TRUE(_audio.looping);
}

TEST_F(Game, LeavingStopsTheMusic)
{
    _scene->onExit(_context);
    EXPECT_EQ(_audio.musicStops, 1);
}

TEST_F(Game, LeavingAfterTheServerIsGoneStillStopsTheMusic)
{
    _context.now = Clock::now() + std::chrono::seconds(6);
    ASSERT_EQ(_scene->update(_context, 0.0F).kind,
        SceneRequest::Kind::CHANGE);

    _scene->onExit(_context);
    EXPECT_EQ(_audio.musicStops, 1);
}

TEST_F(Game, AnEventFromTheServerPlaysItsSound)
{
    send(MessageType::EVENT, rtype::protocol::Event{.eventId = 1,
        .type = EventType::PLAYER_JOINED, .a = 1});
    updateFor(QUIET);

    EXPECT_EQ(_audio.sounds, std::vector<SoundId>{JOIN_SOUND});
}

TEST_F(Game, TheSameEventReceivedThreeTimesPlaysOneSound)
{
    const rtype::protocol::Event joined{.eventId = 7,
        .type = EventType::PLAYER_JOINED, .a = 1};

    for (int copy = 0; copy < 3; ++copy)
        send(MessageType::EVENT, joined);
    updateFor(QUIET);

    EXPECT_EQ(_audio.sounds, std::vector<SoundId>{JOIN_SOUND});
}

static bool contains(const std::vector<std::string> &texts, const std::string &text)
{
    return std::ranges::find(texts, text) != texts.end();
}

static bool showsHelp(const std::vector<std::string> &texts)
{
    return std::ranges::any_of(texts, [](const std::string &text) {
        return text.starts_with("TOGGLE_HUD_HELP: ");
    });
}

TEST_F(Game, EachFrameDrawsTheHud)
{
    update(0.0F);

    const auto texts = drawnTexts();

    EXPECT_TRUE(contains(texts, "team 0"));
    EXPECT_TRUE(contains(texts, "ping -"));
    EXPECT_FALSE(showsHelp(texts));
}

TEST_F(Game, F1TogglesTheHelpOnAndOff)
{
    press(Key::F1);
    update(0.0F);
    EXPECT_TRUE(showsHelp(drawnTexts()));

    release(Key::F1);
    update(0.0F);
    EXPECT_TRUE(showsHelp(drawnTexts()));

    press(Key::F1);
    update(0.0F);
    EXPECT_FALSE(showsHelp(drawnTexts()));
}

TEST_F(Game, AnEventFromTheServerShowsItsBanner)
{
    send(MessageType::EVENT, rtype::protocol::Event{.eventId = 1,
        .type = EventType::PLAYER_JOINED, .a = 1});
    updateFor(QUIET);

    EXPECT_TRUE(contains(drawnTexts(), "P2 joined"));
}
