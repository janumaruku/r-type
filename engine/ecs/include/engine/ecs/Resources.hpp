/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Resources
*/

#ifndef R_TYPE_RESOURCES_HPP
#define R_TYPE_RESOURCES_HPP

#include <type_traits>
#include <utility>

#include "engine/ecs/TypeId.hpp"

namespace engine::ecs {
struct DeltaTime {
    float seconds = 0.0F;
};

template <class Resource>
concept ResourceConcept =
    detail::PlainType<Resource> && std::is_object_v<Resource>;

namespace detail {
class ResourceBase {
public:
    ResourceBase() = default;

    ResourceBase(const ResourceBase &) = delete;

    ResourceBase(ResourceBase &&) = delete;

    virtual ~ResourceBase() = default;

    ResourceBase &operator=(const ResourceBase &) = delete;

    ResourceBase &operator=(ResourceBase &&) = delete;
};

template <ResourceConcept Resource>
class ResourceHolder: public ResourceBase {
public:
    template <class... Args>
    explicit ResourceHolder(Args &&... args);

    Resource value;
};
}
} // ecs
// engine

#endif // R_TYPE_RESOURCES_HPP

#include "engine/ecs/Resources.tpp"
