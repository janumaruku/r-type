/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#ifndef R_TYPE_REGISTRY_HPP
#define R_TYPE_REGISTRY_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "engine/core/Logger.hpp"
#include "engine/ecs/CommandBuffer.hpp"
#include "engine/ecs/Component.hpp"
#include "engine/ecs/IndexedView.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/View.hpp"

namespace engine::ecs {
template <class Stage>
concept StageConcept = std::is_enum_v<Stage> &&
    std::is_same_v<std::underlying_type_t<Stage>, std::uint8_t>;

class Registry;

struct SystemEntry {
    std::string name;
    std::function<void(Registry &)> run;
};

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

    /*
    ** Replacing an existing resource destroys it, so references and pointers
    ** to the old one dangle. setResource keeps the same object instead.
    */
    template <ResourceConcept Resource, class... Args>
        requires std::constructible_from<Resource, Args...>
    Resource &emplaceResource(Args &&... args);

    template <ViewComponentConcept... Components, StageConcept Stage,
        class System>
    void addSystem(Stage stage, const std::string &name, System system);

    template <StageConcept Stage>
    void runStage(Stage stage);

    [[nodiscard]] CommandBuffer &commands() noexcept;

    void applyCommands();

private:
    std::vector<std::unique_ptr<SparseSetBase>> _storages;
    std::vector<std::uint32_t> _generations;
    std::vector<bool> _alive;
    std::vector<std::uint32_t> _freeIndices;
    std::uint32_t _aliveCount = 0;
    std::vector<std::unique_ptr<detail::ResourceBase>> _resources;
    std::vector<std::vector<SystemEntry>> _stages;
    bool _running = false;
    CommandBuffer _commands;
    core::log::Logger _logger{"ecs"};

    struct RunGuard {
        explicit RunGuard(bool &value);

        RunGuard(const RunGuard &) = delete;

        RunGuard &operator=(const RunGuard &) = delete;

        ~RunGuard();

        bool &running;
    };

    void runStageAt(std::uint8_t stage);
};
}

#endif //R_TYPE_REGISTRY_HPP

#include "engine/ecs/Registry.tpp"
#include "engine/ecs/CommandBuffer.tpp"
