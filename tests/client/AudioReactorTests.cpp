/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AudioReactorTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "AudioReactor.hpp"
#include "ClientWorld.hpp"
#include "GameEventBus.hpp"
#include "SoundBank.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/audio/Volume.hpp"
#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::client {
namespace {
using engine::audio::IAudio;
using engine::audio::MusicId;
using engine::audio::SoundId;
using engine::audio::Volume;
using protocol::Event;
using protocol::EventType;

constexpr std::uint32_t OUR_SHIP = 42;
constexpr std::uint32_t OTHER_SHIP = 7;

/** An IAudio that records every call instead of playing it. */
class RecordingAudio final : public IAudio {
public:
    struct Played {
        SoundId sound;
        float volume;
    };

    SoundId loadSound(const std::filesystem::path &) override
    {
        return SoundId{};
    }
    MusicId loadMusic(const std::filesystem::path &) override
    {
        return MusicId{};
    }
    void playSound(SoundId id, float volume) override
    {
        sounds.push_back({id, volume});
    }
    void playMusic(MusicId, bool) override { ++musicCalls; }
    void stopMusic() override { ++musicCalls; }
    void setMasterVolume(float volume) override { _volumes.setMaster(volume); }
    void setSfxVolume(float volume) override { _volumes.setSfx(volume); }
    void setMusicVolume(float volume) override { _volumes.setMusic(volume); }
    void setMuted(bool) override {}
    [[nodiscard]] const Volume &volumes() const override { return _volumes; }
    void update() override {}

    std::vector<Played> sounds;
    int musicCalls = 0;

private:
    Volume _volumes;
};

Event event(EventType type, std::uint32_t a = 0)
{
    Event value;

    value.type = type;
    value.a = a;
    return value;
}

void emit(GameEventBus &bus, const Event &value)
{
    bus.publish(value);
    bus.dispatch();
}

class AudioReactorFixture: public ::testing::Test {
protected:
    AudioReactorFixture()
    {
        // Player 0 flies OUR_SHIP. The reactor reads the local ship from here.
        std::array<protocol::PlayerInfo, 4> roster{};

        roster[0].playerId = 0;
        roster[0].shipNetId = OUR_SHIP;
        _world.setLocalPlayer(0);
        _world.applyRoster(roster);
    }

    GameEventBus _bus;
    ClientWorld _world{SpriteCatalog{}};
    RecordingAudio _audio;
    SoundBank _sounds{
        .shot = SoundId{1},
        .explosionSmall = SoundId{2},
        .explosionBig = SoundId{3},
        .join = SoundId{4},
        .leave = SoundId{5},
        .stage1 = MusicId{1},
    };
};

TEST_F(AudioReactorFixture, OurShipFiringPlaysTheShotAtFullVolume)
{
    AudioReactor reactor{_bus, _audio, _sounds, _world};

    emit(_bus, event(EventType::PLAYER_FIRED, OUR_SHIP));

    ASSERT_EQ(_audio.sounds.size(), 1U);
    EXPECT_EQ(_audio.sounds[0].sound, _sounds.shot);
    EXPECT_FLOAT_EQ(_audio.sounds[0].volume, 1.0F);
}

TEST_F(AudioReactorFixture, AnotherShipFiringPlaysTheShotAtOtherShotVolume)
{
    AudioReactor reactor{_bus, _audio, _sounds, _world};

    emit(_bus, event(EventType::PLAYER_FIRED, OTHER_SHIP));

    ASSERT_EQ(_audio.sounds.size(), 1U);
    EXPECT_EQ(_audio.sounds[0].sound, _sounds.shot);
    EXPECT_FLOAT_EQ(_audio.sounds[0].volume, 0.4F);
    EXPECT_FLOAT_EQ(_audio.sounds[0].volume, OTHER_SHOT_VOLUME);
}

TEST_F(AudioReactorFixture, OurShotAndAnotherShipsShotGetDifferentVolumes)
{
    AudioReactor reactor{_bus, _audio, _sounds, _world};

    emit(_bus, event(EventType::PLAYER_FIRED, OUR_SHIP));
    emit(_bus, event(EventType::PLAYER_FIRED, OTHER_SHIP));

    ASSERT_EQ(_audio.sounds.size(), 2U);
    EXPECT_FLOAT_EQ(_audio.sounds[0].volume, 1.0F);
    EXPECT_FLOAT_EQ(_audio.sounds[1].volume, 0.4F);
}

TEST_F(AudioReactorFixture, EachListedEventPlaysItsSound)
{
    AudioReactor reactor{_bus, _audio, _sounds, _world};

    emit(_bus, event(EventType::ENEMY_KILLED));
    emit(_bus, event(EventType::PLAYER_KILLED));
    emit(_bus, event(EventType::PLAYER_JOINED));
    emit(_bus, event(EventType::PLAYER_LEFT));

    ASSERT_EQ(_audio.sounds.size(), 4U);
    EXPECT_EQ(_audio.sounds[0].sound, _sounds.explosionSmall);
    EXPECT_EQ(_audio.sounds[1].sound, _sounds.explosionBig);
    EXPECT_EQ(_audio.sounds[2].sound, _sounds.join);
    EXPECT_EQ(_audio.sounds[3].sound, _sounds.leave);
}

TEST_F(AudioReactorFixture, EventsWithoutASoundPlayNothing)
{
    AudioReactor reactor{_bus, _audio, _sounds, _world};

    emit(_bus, event(EventType::GAME_OVER));
    emit(_bus, event(EventType::GAME_RESTARTED));
    emit(_bus, event(EventType::ENEMY_FIRED));

    EXPECT_TRUE(_audio.sounds.empty());
    EXPECT_EQ(_audio.musicCalls, 0);
}

TEST_F(AudioReactorFixture, ADestroyedReactorPlaysNothingAndDoesNotCrash)
{
    {
        AudioReactor reactor{_bus, _audio, _sounds, _world};
    }

    emit(_bus, event(EventType::ENEMY_KILLED));

    EXPECT_TRUE(_audio.sounds.empty());
    EXPECT_EQ(_bus.subscribers(), 0U);
}

} // namespace
} // namespace rtype::client