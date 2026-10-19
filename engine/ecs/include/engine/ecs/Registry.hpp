/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#ifndef R_TYPE_REGISTRY_HPP
#define R_TYPE_REGISTRY_HPP

#include <memory>
#include <vector>
#include <cstdint>

#include "engine/ecs/Component.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "engine/ecs/SparseSetBase.hpp"

namespace engine::ecs {
class Registry {
public:
    Registry() = default;

    Registry(const Registry &) = delete;

    Registry(Registry &&) = default;

    Registry &operator=(const Registry &) = delete;

    Registry & operator=(Registry &&) = default;

    template <Component C>
    SparseSet<C> &registerComponent();

    template <Component C>
    SparseSet<C> &getComponents();

    template <Component C>
    const SparseSet<C> &getComponents() const;

    Entity spawnEntity();

    [[nodiscard]] bool isAlive(const Entity &entity) const noexcept;

    void killEntity(const Entity &entity);

    [[nodiscard]] std::size_t aliveCount() const noexcept;

private:
    std::vector<std::unique_ptr<SparseSetBase>> _storages;
    std::vector<std::uint32_t> _generations;
    std::vector<bool> _alive;
    std::vector<std::uint32_t> _freeIndices;
    std::uint32_t _aliveCount = 0;
};
}

#endif //R_TYPE_REGISTRY_HPP

#include "engine/ecs/Registry.tpp"
