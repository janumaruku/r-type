/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ResourcePath.cpp
*/

#include "ResourcePath.hpp"

namespace engine::render {

std::filesystem::path cacheKey(const std::filesystem::path &path)
{
    return path.lexically_normal();
}

}
