/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#ifndef R_TYPE_REGISTRY_TPP
#define R_TYPE_REGISTRY_TPP

#include <cassert>
#include <string>
#include <typeinfo>

#include "Errors.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/TypeId.hpp"

namespace engine::ecs {
template <ComponentConcept Component>
SparseSet<Component> &Registry::registerComponent()
{
    const auto index = detail::typeId<Component>();

    if (index >= _storages.size())
        _storages.resize(index + 1);

    if (_storages[index] == nullptr)
        _storages[index] = std::make_unique<SparseSet<Component>>();

    assert(dynamic_cast<SparseSet<Component> *>(_storages[index].get()) !=
        nullptr);
    return static_cast<SparseSet<Component> &>(*_storages[index]);
}

template <ComponentConcept Component>
SparseSet<Component> &Registry::getComponents()
{
    const auto index = detail::typeId<Component>();

    if (index >= _storages.size() || _storages[index] == nullptr)
        throw core::EngineError(
            "component not registered: " +
            std::string{typeid(Component).name()});

    assert(dynamic_cast<SparseSet<Component> *>(_storages[index].get()) !=
        nullptr);
    return static_cast<SparseSet<Component> &>(*_storages[index]);
}

template <ComponentConcept Component>
const SparseSet<Component> &Registry::getComponents() const
{
    const auto index = detail::typeId<Component>();

    if (index >= _storages.size() || _storages[index] == nullptr)
        throw core::EngineError(
            "component not registered: " +
            std::string{typeid(Component).name()});

    assert(dynamic_cast<SparseSet<Component> *>(_storages[index].get()) !=
        nullptr);
    return static_cast<SparseSet<Component> &>(*_storages[index]);
}

template <typename Component>
    requires ComponentConcept<std::remove_cvref_t<Component>>
std::remove_cvref_t<Component> &Registry::addComponent(const Entity &entity,
    Component &&component)
{
    auto &storage = getComponents<std::remove_cvref_t<Component>>();

    if (!isAlive(entity))
        throw core::EngineError("can't add a component to a dead entity");

    return storage.insertAt(entity, std::forward<Component>(component));
}

template <ComponentConcept Component, typename... Args>
Component &Registry::emplaceComponent(const Entity &entity, Args &&... args)
{
    auto &storage = getComponents<Component>();

    if (!isAlive(entity))
        throw core::EngineError("can't add a component to a dead entity");

    return storage.emplaceAt(entity, std::forward<Args>(args)...);
}

template <ComponentConcept Component>
void Registry::removeComponent(const Entity &entity)
{
    getComponents<Component>().erase(entity);
}

template <ComponentConcept Component>
Component *Registry::getComponent(const Entity &entity)
{
    auto &storage = getComponents<Component>();

    return isAlive(entity) ? storage.tryGet(entity) : nullptr;
}

template <ComponentConcept Component>
const Component *Registry::getComponent(const Entity &entity) const
{
    const auto &storage = getComponents<Component>();

    return isAlive(entity) ? storage.tryGet(entity) : nullptr;
}

template <ViewComponentConcept... Components>
View<Components...> Registry::view()
{
    return View<Components...>(
        getComponents<std::remove_const_t<Components>>()...);
}

template <ViewComponentConcept... Components>
View<Components...> Registry::view() const
{
    static_assert((std::is_const_v<Components> && ...),
        "a const Registry can only give views of const components");

    return View<Components...>(
        getComponents<std::remove_const_t<Components>>()...);
}

template <ViewComponentConcept ... Components>
IndexedView<Components...> Registry::indexedView()
{
    return IndexedView<Components...>(
        getComponents<std::remove_const_t<Components>>()...);
}

template <ViewComponentConcept ... Components>
IndexedView<Components...> Registry::indexedView() const
{
    static_assert((std::is_const_v<Components> && ...),
        "a const Registry can only give views of const components");

    return IndexedView<Components...>(
        getComponents<std::remove_const_t<Components>>()...);
}
}

#endif
