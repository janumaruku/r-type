/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameWorldTestAccess
*/

#ifndef R_TYPE_GAMEWORLDTESTACCESS_HPP
#define R_TYPE_GAMEWORLDTESTACCESS_HPP

#include "engine/ecs/Registry.hpp"
#include "rtype/game/GameWorld.hpp"

/**
 * @brief Gives the tests the world's registry, so they can place entities
 * and read components. Only the tests define it: release code can't reach
 * the registry through it.
 */
struct rtype::game::GameWorldTestAccess {
    static engine::ecs::Registry &registry(GameWorld &world)
    {
        return world._registry;
    }
};

#endif //R_TYPE_GAMEWORLDTESTACCESS_HPP
