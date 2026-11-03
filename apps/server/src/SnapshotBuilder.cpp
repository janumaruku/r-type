/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotBuilder.cpp
*/

#include "SnapshotBuilder.hpp"

#include <vector>

#include "Conversions.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::server {

std::vector<protocol::EntityState> convertEntities(const std::vector<
    game::ReplicatedEntity> &entities)
{
    std::vector<protocol::EntityState> result;
    result.reserve(entities.size());
    for (const auto &entity: entities) {
        result.push_back(protocol::EntityState{
            .netId = entity.netId,
            .kind  = static_cast<protocol::EntityKind>(entity.kind),
            .x     = entity.position.x(),
            .y     = entity.position.y(),
            .vx    = entity.velocity.x(),
            .vy    = entity.velocity.y(),
        });
    }
    return result;
}

const std::vector<protocol::Snapshot> &SnapshotBuilder::
    build(const game::GameWorld &world, std::uint32_t lastProcessedInput)
{
    world.collectReplicated(_entities);
    const auto roster = rosterToProtocol(world.roster());
    protocol::Snapshot s{
        .serverTick = world.tick(),
        .lastProcessedInput = lastProcessedInput,
        .roster = std::vector<protocol::PlayerInfo>(roster.begin(), roster.end()),
        .entities = convertEntities(_entities),
    };
    _chunks = protocol::splitSnapshot(s);
    return _chunks;
}
} // namespace rtype::server