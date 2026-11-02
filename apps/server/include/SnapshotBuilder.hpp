/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotBuilder.hpp
*/


#ifndef SNAPSHOT_BUILDER_HPP
#define SNAPSHOT_BUILDER_HPP

#include <vector>
#include "rtype/game/GameWorld.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::server {
class SnapshotBuilder {
public:
    [[nodiscard]] const std::vector<protocol::Snapshot> &build(const game::GameWorld &world);

private:
    std::vector<game::ReplicatedEntity> _entities;
    std::vector<protocol::Snapshot> _chunks;
};
}
#endif // SNAPSHOT_BUILDER_HPP