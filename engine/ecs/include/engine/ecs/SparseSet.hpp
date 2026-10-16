/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSet
*/

#ifndef R_TYPE_SPARSESET_HPP
#define R_TYPE_SPARSESET_HPP

#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/Component.hpp"

namespace engine::ecs {
template <Component C>
class SparseSet: public SparseSetBase {
public:
    SparseSet() = default;

    const C &get(const Entity &entity) const noexcept;

    C *tryGet(const Entity &entity) const noexcept;

    C &insertAt(const Entity &entity, const C &value);

    C &insertAt(const Entity &entity, C &&value);

    template <typename... Args>
    C &emplaceAt(const Entity &entity, const Args &... args);

    void erase(const Entity &entity) noexcept override;

    std::span<C> components() const noexcept;

private:
    std::vector<C> _components;

    void exceptionSafeAppend(const Entity &entity);
};
}

#endif //R_TYPE_SPARSESET_HPP

#include "engine/ecs/SparseSet.tpp"
