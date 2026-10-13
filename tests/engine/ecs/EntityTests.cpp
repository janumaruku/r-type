/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EtityTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "engine/ecs/Entity.hpp"

using engine::ecs::Entity;
using engine::ecs::detail::EntityAccess;

static_assert(sizeof(Entity) == 8);
static_assert(std::is_trivially_copyable_v<Entity>);
static_assert(!std::is_default_constructible_v<Entity>);
static_assert(!std::is_convertible_v<Entity, std::size_t>);
static_assert(!std::is_convertible_v<Entity, std::uint32_t>);
static_assert(!std::is_constructible_v<Entity, std::uint32_t>);
static_assert(!std::is_constructible_v<Entity, std::uint32_t, std::uint32_t>);

static_assert(EntityAccess::make(7, 3).index() == 7);
static_assert(EntityAccess::make(7, 3).generation() == 3);
static_assert(Entity::NONE != EntityAccess::make(0, 0));
