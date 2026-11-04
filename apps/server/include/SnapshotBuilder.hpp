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
    /**
     * @brief Builds this tick's snapshot once for every client: the roster
     * and every replicated entity, split into chunks. `lastProcessedInput`
     * is left at 0, since it's the only field that differs per client: the
     * caller sets it on each chunk just before sending it.
     */
    [[nodiscard]] std::vector<protocol::Snapshot> &build(
        const game::GameWorld &world);

private:
    std::vector<game::ReplicatedEntity> _entities;
    std::vector<protocol::Snapshot> _chunks;
};
}
#endif // SNAPSHOT_BUILDER_HPP