/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCache.hpp
*/

#ifndef FONT_CACHE_HPP
#define FONT_CACHE_HPP

#include <cstdint>
#include <filesystem>
#include <raylib.h>
#include "Vec2.hpp"

namespace engine::render {

struct FontId {
    std::uint32_t value{};

    explicit FontId(std::uint32_t id) : value{id} {}
};

class FontCache {
public:
    FontCache();
    ~FontCache();
    FontId load(const std::filesystem::path &path);
private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
};

#endif