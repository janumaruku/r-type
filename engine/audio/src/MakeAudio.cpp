/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MakeAudio.cpp
*/

#include "engine/audio/MakeAudio.hpp"

#include "engine/audio/NullAudio.hpp"
#include "engine/audio/RaylibAudio.hpp"
#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"

namespace engine::audio {

std::unique_ptr<IAudio> makeAudio()
{
    core::log::Logger log{"audio"};

    try {
        auto audio = std::make_unique<RaylibAudio>();
        log.info("sound device opened");
        return audio;
    } catch (const core::EngineError &error) {
        log.warn("no sound, falling back to NullAudio: {}", std::string{error.what()});
        return std::make_unique<NullAudio>();
    }
}

}
