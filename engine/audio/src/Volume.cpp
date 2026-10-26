/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Volume.cpp
*/

#include "engine/audio/Volume.hpp"

#include <algorithm>
#include <cmath>

namespace engine::audio {

namespace {

/*
** std::clamp lets a NaN through: every comparison with NaN is false.
*/
float clampVolume(float volume) noexcept
{
    if (std::isnan(volume))
        return 0.0F;
    return std::clamp(volume, 0.0F, 1.0F);
}

}

void Volume::setMaster(float volume) noexcept
{
    _master = clampVolume(volume);
}

void Volume::setSfx(float volume) noexcept
{
    _sfx = clampVolume(volume);
}

void Volume::setMusic(float volume) noexcept
{
    _music = clampVolume(volume);
}

void Volume::setMuted(bool muted) noexcept
{
    _muted = muted;
}

float Volume::master() const noexcept
{
    return _master;
}

float Volume::sfx() const noexcept
{
    return _sfx;
}

float Volume::music() const noexcept
{
    return _music;
}

bool Volume::muted() const noexcept
{
    return _muted;
}

float Volume::effectiveSfx(float playVolume) const noexcept
{
    if (_muted)
        return 0.0F;
    return _master * _sfx * clampVolume(playVolume);
}

float Volume::effectiveMusic() const noexcept
{
    if (_muted)
        return 0.0F;
    return _master * _music;
}

}
