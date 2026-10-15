/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetBase
*/

#ifndef R_TYPE_SPARSESETBASE_HPP
#define R_TYPE_SPARSESETBASE_HPP

#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "Entity.hpp"

namespace engine::ecs {
class SparseSetBase {
public:
    using size_type = std::size_t; // NOLINT

    static constexpr size_type NPOS = std::numeric_limits<size_type>::max();

    virtual ~SparseSetBase() = default;

    [[nodiscard]] bool contains(const Entity &entity) const noexcept;

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] std::span<const Entity> entities() const noexcept;

    virtual void erase(const Entity &entity) = 0;

protected:
    std::vector<size_type> _sparse;
    std::vector<Entity> _dense;
};
}

#endif //R_TYPE_SPARSESETBASE_HPP
