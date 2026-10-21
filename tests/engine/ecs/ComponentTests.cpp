/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ComponentTests
*/

#include <concepts>
#include <memory>
#include <string>

#include "engine/ecs/Component.hpp"

using engine::ecs::ComponentConcept;

namespace {
struct Position {
    float x;
    float y;
};

struct Tag {};

struct Named {
    std::string name;
};

struct MoveOnly {
    std::unique_ptr<int> value;
};

struct NoMoveAssignment {
    NoMoveAssignment() = default;
    NoMoveAssignment(NoMoveAssignment &&) = default;
    NoMoveAssignment &operator=(NoMoveAssignment &&) = delete;
};

struct Abstract {
    virtual ~Abstract() = default;
    virtual void update() = 0;
};
} // namespace

static_assert(ComponentConcept<Position>);
static_assert(ComponentConcept<Tag>);
static_assert(ComponentConcept<Named>);
static_assert(ComponentConcept<MoveOnly>);
static_assert(!std::copyable<MoveOnly>);

static_assert(!ComponentConcept<const Position>);
static_assert(!ComponentConcept<volatile Position>);
static_assert(!ComponentConcept<Position &>);
static_assert(!ComponentConcept<void>);
static_assert(!ComponentConcept<int[3]>);
static_assert(!ComponentConcept<NoMoveAssignment>);
static_assert(!ComponentConcept<Abstract>);
