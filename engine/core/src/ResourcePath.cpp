/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ResourcePath.cpp
*/

#include "engine/core/ResourcePath.hpp"

namespace engine::core {

std::filesystem::path cacheKey(const std::filesystem::path &path)
{
    return path.lexically_normal();
}

}
