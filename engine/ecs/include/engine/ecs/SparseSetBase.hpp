/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetBase
*/

#ifndef R_TYPE_SPARSESETBASE_HPP
#define R_TYPE_SPARSESETBASE_HPP

#include <cstdint>
#include <span>
#include <vector>"

#include "Entity.hpp"

namespace engine::ecs {
class SparseSetBase {
public:
    using EntityType = std::uint32_t;

    virtual ~SparseSetBase() = default;

    [[nodiscard]] bool contains(EntityType e) const noexcept;

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] std::span<const EntityType> entities() const noexcept;

    virtual void erase(EntityType e) = 0;

private:
    std::vector<std::uint32_t> _sparse;
    std::vector<Entity> _dense;
};
}

#endif //R_TYPE_SPARSESETBASE_HPP
