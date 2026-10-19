/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** TextureCache.hpp
*/

#ifndef TEXTURE_CACHE_HPP
#define TEXTURE_CACHE_HPP

#include <filesystem>
#include "Vec2.hpp"

namespace engine::render {

struct TextureId {
    std::uint16_t value{};

    TextureId() = default;
    explicit TextureId(std::uint16_t id) : value{id} {}

    bool operator==(TextureId const &) const = default;
};

class TextureCache {
public:
    TextureCache();
    ~TextureCache();
    TextureId load(const std::filesystem::path &path);
    [[nodiscard]] core::Vec2u size(TextureId) const;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
    std::uint32_t _cacheSize = 0;
};

}

template <>
struct std::hash<engine::render::TextureId> {
    std::size_t operator()(engine::render::TextureId id) const noexcept
    {
        return std::hash<std::uint32_t>{}(id.value);
    }
};

#endif