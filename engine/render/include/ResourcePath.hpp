/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ResourcePath.hpp
*/

#ifndef RESOURCE_PATH_HPP
#define RESOURCE_PATH_HPP

#include <filesystem>

namespace engine::render {

/*
** The key the caches store a file under: the path made lexically_normal(),
** so "assets/ship.png" and "assets/./ship.png" load once and share an id.
*/
[[nodiscard]] std::filesystem::path cacheKey(const std::filesystem::path &path);

}

#endif /* !RESOURCE_PATH_HPP */
