/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSet
*/

#ifndef R_TYPE_SPARSESET_HPP
#define R_TYPE_SPARSESET_HPP

#include <span>
#include <vector>

#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/Component.hpp"

namespace engine::ecs {
template <Component C>
class SparseSet final: public SparseSetBase {
public:
    SparseSet() = default;

    C &get(const Entity &entity) noexcept;

    const C &get(const Entity &entity) const noexcept;

    C *tryGet(const Entity &entity) noexcept;

    const C *tryGet(const Entity &entity) const noexcept;

    C &insertAt(const Entity &entity, const C &value);

    C &insertAt(const Entity &entity, C &&value);

    template <typename... Args>
    C &emplaceAt(const Entity &entity, Args &&... args);

    /*
    ** Swap-and-pop: the last component moves into the erased one's position.
    ** - Storage order isn't stable: never rely on it.
    ** - To erase while iterating, walk entities() from the end, so the element
    **   moved into the current slot has already been visited.
    ** - A reference or pointer to the last component is invalidated by erasing
    **   another entity: never keep one across an erase.
    */
    void erase(const Entity &entity) override;

    std::span<C> components() noexcept;

    std::span<const C> components() const noexcept;

private:
    std::vector<C> _components;

    void exceptionSafeAppend(const Entity &entity);
};
}

#endif //R_TYPE_SPARSESET_HPP

#include "engine/ecs/SparseSet.tpp"
