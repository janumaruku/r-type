/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Resources
*/

#ifndef R_TYPE_RESOURCES_TPP
#define R_TYPE_RESOURCES_TPP

#include "engine/ecs/Resources.hpp"

namespace engine::ecs::detail {
template <ResourceConcept Resource>
template <class... Args>
ResourceHolder<Resource>::ResourceHolder(Args &&... args):
    value(std::forward<Args>(args)...)
{
}
}

#endif // R_TYPE_RESOURCES_TPP
