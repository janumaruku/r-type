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
#include "engine/ecs/IndexedView.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/View.hpp"

namespace engine::ecs {
class Registry {
public:
    Registry() = default;

    Registry(const Registry &) = delete;

    Registry(Registry &&) = default;

    Registry &operator=(const Registry &) = delete;

    Registry &operator=(Registry &&) = default;

    template <ComponentConcept Component>
    SparseSet<Component> &registerComponent();

    template <ComponentConcept Component>
    SparseSet<Component> &getComponents();

    template <ComponentConcept Component>
    const SparseSet<Component> &getComponents() const;

    Entity spawnEntity();

    [[nodiscard]] bool isAlive(const Entity &entity) const noexcept;

    void killEntity(const Entity &entity);

    [[nodiscard]] std::size_t aliveCount() const noexcept;

    template <typename Component>
        requires ComponentConcept<std::remove_cvref_t<Component>>
    std::remove_cvref_t<Component> &addComponent(const Entity &entity,
        Component &&component);

    template <ComponentConcept Component, typename... Args>
    Component &emplaceComponent(const Entity &entity, Args &&... args);

    template <ComponentConcept Component>
    void removeComponent(const Entity &entity);

    template <ComponentConcept Component>
    Component *getComponent(const Entity &entity);

    template <ComponentConcept Component>
    const Component *getComponent(const Entity &entity) const;

    template <ViewComponentConcept... Components>
    View<Components...> view();

    template <ViewComponentConcept... Components>
    View<Components...> view() const;

    template <ViewComponentConcept... Components>
    IndexedView<Components...> indexedView();

    template <ViewComponentConcept... Components>
    IndexedView<Components...> indexedView() const;

    template <ResourceConcept Resource>
        requires std::movable<Resource>
    Resource &setResource(Resource resource);

    template <ResourceConcept Resource>
    Resource &getResource();

    template <ResourceConcept Resource>
    const Resource &getResource() const;

    template <ResourceConcept Resource>
    Resource *tryGetResource();

    template <ResourceConcept Resource>
    const Resource *tryGetResource() const;

private:
    std::vector<std::unique_ptr<SparseSetBase>> _storages;
    std::vector<std::uint32_t> _generations;
    std::vector<bool> _alive;
    std::vector<std::uint32_t> _freeIndices;
    std::uint32_t _aliveCount = 0;
    std::vector<std::unique_ptr<detail::ResourceBase>> _resources;
};
}

#endif //R_TYPE_REGISTRY_HPP

#include "engine/ecs/Registry.tpp"
