/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotBuilder.cpp
*/

#include "SnapshotBuilder.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include "Conversions.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::server {
namespace {
/*
** Goes through toProtocol, the one place that maps a ReplicatedEntity, so
** no field (the variant, the player's colour) can be forgotten here.
*/
std::vector<protocol::EntityState> convertEntities(const std::vector<
    game::ReplicatedEntity> &entities)
{
    std::vector<protocol::EntityState> result;

    result.reserve(entities.size());
    std::ranges::transform(entities, std::back_inserter(result),
        [](const game::ReplicatedEntity &entity) {
            return toProtocol(entity);
        });
    return result;
}
} // namespace

std::vector<protocol::Snapshot> &SnapshotBuilder::build(const game::GameWorld
        &world)
{
    world.collectReplicated(_entities);
    const auto roster = rosterToProtocol(world);
    protocol::Snapshot s{
        .serverTick         = world.tick(),
        .lastProcessedInput = 0,
        .roster =
            std::vector<protocol::PlayerInfo>(roster.begin(), roster.end()),
        .entities = convertEntities(_entities),
    };
    _chunks = protocol::splitSnapshot(s);
    return _chunks;
}
} // namespace rtype::server
