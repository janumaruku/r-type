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

#include "engine/ecs/Entity.hpp"

namespace engine::ecs {
class SparseSetBase {
public:
    using size_type = std::uint32_t; // NOLINT

    static constexpr size_type NPOS = std::numeric_limits<size_type>::max();

    SparseSetBase() = default;

    SparseSetBase(const SparseSetBase &sparse) = delete;

    SparseSetBase(SparseSetBase &&sparse) = delete;

    virtual ~SparseSetBase() = default;

    SparseSetBase &operator=(const SparseSetBase &sparse) = delete;

    SparseSetBase &operator=(SparseSetBase &&sparse) = delete;

    [[nodiscard]] bool contains(const Entity &entity) const noexcept;

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] std::span<const Entity> entities() const noexcept;

    virtual void erase(const Entity &entity) = 0;

    [[nodiscard]] bool empty() const noexcept;

protected:
    std::vector<size_type> _sparse;
    std::vector<Entity> _dense;

    [[nodiscard]] size_type positionOf(const Entity &entity) const noexcept;

    void swapAndPop(const Entity &entity) noexcept;

    void append(const Entity &entity);
};
}

#endif //R_TYPE_SPARSESETBASE_HPP
