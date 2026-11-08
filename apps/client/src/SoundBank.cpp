/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SoundBank.cpp
*/

#include "SoundBank.hpp"

#include "engine/core/AssetLocator.hpp"

namespace rtype::client {

namespace {
constexpr const char *SHOT_FILE            = "sounds/shot.wav";
constexpr const char *EXPLOSION_SMALL_FILE = "sounds/explosion_small.wav";
constexpr const char *EXPLOSION_BIG_FILE   = "sounds/explosion_big.wav";
constexpr const char *JOIN_FILE            = "sounds/ui_join.wav";
constexpr const char *LEAVE_FILE           = "sounds/ui_leave.wav";
constexpr const char *STAGE1_FILE          = "music/stage1.ogg";
} // namespace

/*
** The files are resolved by AssetLocator before IAudio sees them, so a
** missing one throws ResourceError even with NullAudio, which loads nothing.
*/
SoundBank SoundBank::load(engine::audio::IAudio &audio)
{
    const engine::core::AssetLocator assets;

    return {
        .shot           = audio.loadSound(assets.resolve(SHOT_FILE)),
        .explosionSmall = audio.loadSound(assets.resolve(EXPLOSION_SMALL_FILE)),
        .explosionBig   = audio.loadSound(assets.resolve(EXPLOSION_BIG_FILE)),
        .join           = audio.loadSound(assets.resolve(JOIN_FILE)),
        .leave          = audio.loadSound(assets.resolve(LEAVE_FILE)),
        .stage1         = audio.loadMusic(assets.resolve(STAGE1_FILE)),
    };
}
} // namespace rtype::client
