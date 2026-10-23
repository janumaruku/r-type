/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CommandBuffer
*/

#ifndef R_TYPE_COMMANDBUFFER_HPP
#define R_TYPE_COMMANDBUFFER_HPP

#include <cstddef>
#include <functional>
#include <vector>

#include "engine/ecs/Component.hpp"
#include "engine/ecs/Entity.hpp"

namespace engine::ecs {
class Registry;

class CommandBuffer {
public:
    using Command = std::function<void(Registry &)>;
    using SpawnInit = std::function<void(Registry &, const Entity &)>;

    /*
    ** Records a kill. Ignored when applied if the entity is already dead, so
    ** two colliders can each record a kill for the other.
    */
    void kill(const Entity &entity);

    /*
    ** Records a spawn. When applied, the new entity is created, then init,
    ** if not empty, adds its components: its handle doesn't exist before. If
    ** init throws, the new entity is killed before the exception propagates.
    */
    void spawn(SpawnInit init = {});

    /*
    ** Records an addComponent. Ignored when applied if the entity died in the
    ** meantime. The component must be copyable, since commands are stored in
    ** std::function: add a move-only one immediately, outside any view loop.
    */
    template <ComponentConcept Component>
    void add(const Entity &entity, Component value);

    /*
    ** Records a removeComponent. Ignored when applied if the entity is dead.
    */
    template <ComponentConcept Component>
    void remove(const Entity &entity);

    /*
    ** Applies every command in recorded order, including those recorded while
    ** applying, and leaves the buffer empty. If a command throws, the rest are
    ** discarded with the buffer cleared, and the exception propagates.
    */
    void apply(Registry &registry);

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;

    void clear() noexcept;

private:
    std::vector<Command> _commands;
};
}

#endif //R_TYPE_COMMANDBUFFER_HPP
