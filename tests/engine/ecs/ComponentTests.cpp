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

using engine::ecs::Component;

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

static_assert(Component<Position>);
static_assert(Component<Tag>);
static_assert(Component<Named>);
static_assert(Component<MoveOnly>);
static_assert(!std::copyable<MoveOnly>);

static_assert(!Component<const Position>);
static_assert(!Component<volatile Position>);
static_assert(!Component<Position &>);
static_assert(!Component<void>);
static_assert(!Component<int[3]>);
static_assert(!Component<NoMoveAssignment>);
static_assert(!Component<Abstract>);
