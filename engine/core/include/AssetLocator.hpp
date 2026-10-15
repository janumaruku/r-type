/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AssetLocator.hpp
*/

#ifndef ASSETLOCATOR_HPP
#define ASSETLOCATOR_HPP

#include <filesystem>
#include <optional>
#include <string>

namespace engine::core {

class AssetLocator {
public:
    explicit AssetLocator(std::optional<std::filesystem::path> overrideRoot = std::nullopt);

    AssetLocator(const AssetLocator &) = delete;
    AssetLocator &operator=(const AssetLocator &) = delete;

    AssetLocator(AssetLocator &&) noexcept = default;
    AssetLocator &operator=(AssetLocator &&) noexcept = default;

    [[nodiscard]] std::filesystem::path resolve(const std::string &relative) const;
    [[nodiscard]] const std::filesystem::path &root() const noexcept;
private:
    std::filesystem::path _root;
};

} // namespace engine::core
#endif