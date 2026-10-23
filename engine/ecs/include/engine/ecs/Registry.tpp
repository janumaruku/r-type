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

#include "../../../../core/include/engine/core/Errors.hpp"
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

template <ResourceConcept Resource> requires std::movable<Resource>
Resource &Registry::setResource(Resource resource)
{
    if (Resource *existing = tryGetResource<Resource>()) {
        *existing = std::move(resource);
        return *existing;
    }

    const auto index = detail::typeId<Resource>();
    if (index >= _resources.size())
        _resources.resize(index + 1);

    auto holder = std::make_unique<detail::ResourceHolder<Resource>>(
        std::move(resource));
    Resource &stored = holder->value;
    _resources[index] = std::move(holder);

    return stored;
}

template <ResourceConcept Resource>
Resource &Registry::getResource()
{
    Resource *resource = tryGetResource<Resource>();

    if (resource == nullptr)
        throw core::EngineError(
            "resource not set: " + std::string{typeid(Resource).name()});

    return *resource;
}

template <ResourceConcept Resource>
const Resource &Registry::getResource() const
{
    const Resource *resource = tryGetResource<Resource>();

    if (resource == nullptr)
        throw core::EngineError(
            "resource not set: " + std::string{typeid(Resource).name()});

    return *resource;
}

template <ResourceConcept Resource>
Resource *Registry::tryGetResource()
{
    const auto index = detail::typeId<Resource>();

    if (index >= _resources.size() || _resources[index] == nullptr)
        return nullptr;

    assert(dynamic_cast<detail::ResourceHolder<Resource>*>
        (_resources[index].get()) != nullptr);

    return &static_cast<detail::ResourceHolder<Resource> &>(*_resources[index]).
        value;
}

template <ResourceConcept Resource>
const Resource *Registry::tryGetResource() const
{
    const auto index = detail::typeId<Resource>();

    if (index >= _resources.size() || _resources[index] == nullptr)
        return nullptr;

    assert(dynamic_cast<detail::ResourceHolder<Resource> *>
        (_resources[index].get()) != nullptr);

    return &static_cast<const detail::ResourceHolder<Resource> &>
        (*_resources[index]).value;
}

template <ResourceConcept Resource, class... Args>
    requires std::constructible_from<Resource, Args...>
Resource &Registry::emplaceResource(Args &&... args)
{
    const auto index = detail::typeId<Resource>();
    if (index >= _resources.size())
        _resources.resize(index + 1);

    auto holder = std::make_unique<detail::ResourceHolder<Resource>>(
        std::forward<Args>(args)...);
    Resource &stored = holder->value;
    _resources[index] = std::move(holder);

    return stored;
}

template <ViewComponentConcept... Components, StageConcept Stage, class System>
void Registry::addSystem(Stage stage, const std::string &name, System system)
{
    static_assert(std::is_invocable_v<System &, Registry &,
        detail::StorageFor<Components> &...>,
        "a system must be callable as f(Registry &, storages...) "
        "in the listed order, with const for each const component");
    static_assert(std::is_copy_constructible_v<System>,
        "systems are stored in std::function, so they must "
        "be copyable; put move-only state in a resource");

    assert(!_running);
    (registerComponent<std::remove_const_t<Components>>(), ...);

    auto item = [system = std::move(system)](Registry &registry) mutable {
        system(registry,
            registry.getComponents<std::remove_const_t<Components>>()...);
    };

    const auto index = static_cast<std::uint8_t>(stage);
    if (index >= _stages.size())
        _stages.resize(index + 1);
    _stages[index].push_back({.name = name, .run = std::move(item)});
}

template <StageConcept Stage>
void Registry::runStage(Stage stage)
{
    runStageAt(static_cast<std::uint8_t>(stage));
}
}

#endif
