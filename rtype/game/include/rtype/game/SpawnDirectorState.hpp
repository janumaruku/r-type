/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpawnDirectorState
*/

#ifndef R_TYPE_SPAWN_DIRECTOR_STATE_HPP
#define R_TYPE_SPAWN_DIRECTOR_STATE_HPP

namespace rtype::game {
struct SpawnDirectorState {
    float elapsed = 0.0F;
    float untilNextSpawn = 0.0F;
};
}

#endif //R_TYPE_SPAWN_DIRECTOR_STATE_HPP
