/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Voices.cpp
*/

#include "engine/audio/Voices.hpp"

#include "engine/core/Errors.hpp"

namespace engine::audio {

std::size_t chooseVoice(std::span<const Voice> voices)
{
    if (voices.empty())
        throw core::EngineError("chooseVoice: a sound needs at least one voice");

    std::size_t oldest = 0;

    for (std::size_t i = 0; i < voices.size(); ++i) {
        if (!voices[i].playing)
            return i;
        if (voices[i].startedAt < voices[oldest].startedAt)
            oldest = i;
    }
    return oldest;
}

}
