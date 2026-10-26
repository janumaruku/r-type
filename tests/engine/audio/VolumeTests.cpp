/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** VolumeTests
*/

#include <gtest/gtest.h>

#include <limits>

#include "engine/audio/Volume.hpp"

using engine::audio::Volume;

TEST(VolumeTests, EveryLevelStartsAtOneAndUnmuted)
{
    const Volume volume;

    EXPECT_FLOAT_EQ(volume.master(), 1.0F);
    EXPECT_FLOAT_EQ(volume.sfx(), 1.0F);
    EXPECT_FLOAT_EQ(volume.music(), 1.0F);
    EXPECT_FALSE(volume.muted());
}

TEST(VolumeTests, ASoundIsMasterTimesSfxTimesItsOwnVolume)
{
    Volume volume;

    volume.setMaster(0.5F);
    volume.setSfx(0.8F);
    EXPECT_FLOAT_EQ(volume.effectiveSfx(0.5F), 0.2F);
}

TEST(VolumeTests, TheMusicIsMasterTimesMusic)
{
    Volume volume;

    volume.setMaster(0.5F);
    volume.setMusic(0.6F);
    EXPECT_FLOAT_EQ(volume.effectiveMusic(), 0.3F);
}

TEST(VolumeTests, LevelsAreClampedToZeroAndOne)
{
    Volume volume;

    volume.setMaster(1.7F);
    EXPECT_FLOAT_EQ(volume.master(), 1.0F);
    volume.setSfx(-0.3F);
    EXPECT_FLOAT_EQ(volume.sfx(), 0.0F);
}

TEST(VolumeTests, NaNBecomesZero)
{
    Volume volume;

    volume.setMusic(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FLOAT_EQ(volume.music(), 0.0F);
    EXPECT_FLOAT_EQ(volume.effectiveSfx(std::numeric_limits<float>::quiet_NaN()), 0.0F);
}

TEST(VolumeTests, APlayVolumeAboveOneIsClamped)
{
    const Volume volume;

    EXPECT_FLOAT_EQ(volume.effectiveSfx(3.0F), 1.0F);
}

TEST(VolumeTests, MuteSilencesEverythingAndUnmuteRestoresTheLevels)
{
    Volume volume;

    volume.setMaster(0.5F);
    volume.setSfx(0.8F);
    volume.setMusic(0.6F);
    volume.setMuted(true);
    EXPECT_FLOAT_EQ(volume.effectiveSfx(1.0F), 0.0F);
    EXPECT_FLOAT_EQ(volume.effectiveMusic(), 0.0F);

    volume.setMuted(false);
    EXPECT_FLOAT_EQ(volume.master(), 0.5F);
    EXPECT_FLOAT_EQ(volume.sfx(), 0.8F);
    EXPECT_FLOAT_EQ(volume.music(), 0.6F);
    EXPECT_FLOAT_EQ(volume.effectiveSfx(1.0F), 0.4F);
    EXPECT_FLOAT_EQ(volume.effectiveMusic(), 0.3F);
}

TEST(VolumeTests, ChangingALevelWhileMutedIsKeptForTheUnmute)
{
    Volume volume;

    volume.setMuted(true);
    volume.setMaster(0.25F);
    volume.setMuted(false);
    EXPECT_FLOAT_EQ(volume.effectiveMusic(), 0.25F);
}
