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
template <ComponentConcept Component>
class SparseSet final: public SparseSetBase {
public:
    SparseSet() = default;

    Component &get(const Entity &entity) noexcept;

    const Component &get(const Entity &entity) const noexcept;

    Component *tryGet(const Entity &entity) noexcept;

    const Component *tryGet(const Entity &entity) const noexcept;

    Component &insertAt(const Entity &entity, const Component &value);

    Component &insertAt(const Entity &entity, Component &&value);

    template <typename... Args>
    Component &emplaceAt(const Entity &entity, Args &&... args);

    /*
    ** Swap-and-pop: the last component moves into the erased one's position.
    ** - Storage order isn't stable: never rely on it.
    ** - To erase while iterating, walk entities() from the end, so the element
    **   moved into the current slot has already been visited.
    ** - A reference or pointer to the last component is invalidated by erasing
    **   another entity: never keep one across an erase.
    */
    void erase(const Entity &entity) override;

    std::span<Component> components() noexcept;

    std::span<const Component> components() const noexcept;

private:
    std::vector<Component> _components;

    void exceptionSafeAppend(const Entity &entity);
};
}

#endif //R_TYPE_SPARSESET_HPP

#include "engine/ecs/SparseSet.tpp"
