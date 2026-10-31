/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameSystemsFixture
*/

#ifndef R_TYPE_GAMESYSTEMSFIXTURE_HPP
#define R_TYPE_GAMESYSTEMSFIXTURE_HPP

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include "engine/core/Rng.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/NetIdAllocator.hpp"
#include "rtype/game/PlayerRoster.hpp"
#include "rtype/game/Rules.hpp"
#include "rtype/game/Systems.hpp"

namespace game_tests {

enum class Stage : std::uint8_t { SIMULATION };

inline constexpr float DT = 1.0F / 60.0F;
inline constexpr std::uint32_t SEED = 7;

/*
** A registry set up like GameWorld: every component registered and every
** resource the builders and systems read. No system is added: each test
** adds the ones it exercises, in the documented tick order.
*/
class GameSystems: public testing::Test {
protected:
    void SetUp() override
    {
        rtype::game::registerComponents(_registry);
        _registry.setResource(rtype::game::Rules{});
        _registry.emplaceResource<engine::core::Rng>(SEED);
        _registry.setResource(rtype::game::NetIdAllocator{});
        _registry.setResource(engine::ecs::DeltaTime{});
        _registry.emplaceResource<rtype::game::PlayerRoster>();
        _registry.emplaceResource<rtype::game::EventOutbox>();
    }

    void addPlayerInputSystem()
    {
        _registry.addSystem<rtype::game::PlayerShip, rtype::game::Velocity,
            rtype::game::Weapon>(Stage::SIMULATION, "playerInputSystem",
            rtype::game::playerInputSystem);
    }

    void addStraightMoverSystem()
    {
        _registry.addSystem<const rtype::game::StraightMover,
            rtype::game::Velocity>(Stage::SIMULATION, "straightMoverSystem",
            rtype::game::straightMoverSystem);
    }

    void addSineMoverSystem()
    {
        _registry.addSystem<rtype::game::SineMover, rtype::game::Velocity>(
            Stage::SIMULATION, "sineMoverSystem",
            rtype::game::sineMoverSystem);
    }

    void addAimedShooterSystem()
    {
        _registry.addSystem<rtype::game::AimedShooter,
            const rtype::game::Transform, rtype::game::Weapon>(
            Stage::SIMULATION, "aimedShooterSystem",
            rtype::game::aimedShooterSystem);
    }

    void addWeaponSystem()
    {
        _registry.addSystem<rtype::game::Weapon, const rtype::game::Transform,
            const rtype::game::Replicated>(Stage::SIMULATION, "weaponSystem",
            rtype::game::weaponSystem);
    }

    void addMovementSystem()
    {
        _registry.addSystem<rtype::game::Transform,
            const rtype::game::Velocity>(Stage::SIMULATION, "movementSystem",
            rtype::game::movementSystem);
    }

    void addCollisionSystem()
    {
        _registry.addSystem<const rtype::game::Collider,
            const rtype::game::Transform, rtype::game::Health>(
            Stage::SIMULATION, "collisionSystem",
            rtype::game::collisionSystem);
    }

    void addHealthSystem()
    {
        _registry.addSystem<const rtype::game::Health,
            const rtype::game::Replicated>(Stage::SIMULATION, "healthSystem",
            rtype::game::healthSystem);
    }

    void addLifetimeSystem()
    {
        _registry.addSystem<rtype::game::Lifetime>(Stage::SIMULATION,
            "lifetimeSystem", rtype::game::lifetimeSystem);
    }

    void tick(const int count = 1)
    {
        for (int i = 0; i < count; ++i) {
            _registry.getResource<engine::ecs::DeltaTime>().seconds = DT;
            _registry.runStage(Stage::SIMULATION);
        }
    }

    /*
    ** Reserves the lowest free slot, joins it with `lives`, and gives it an
    ** ALIVE ship that is no longer invulnerable.
    */
    engine::ecs::Entity spawnAliveShip(const std::uint8_t lives = 3)
    {
        auto &players = roster();
        const std::uint8_t playerId = players.reserve().value();
        const engine::ecs::Entity ship = _registry.spawnEntity();

        EXPECT_TRUE(players.join(playerId, "player", lives));
        EXPECT_TRUE(players.onShipSpawned(playerId,
            rtype::game::makePlayerShip(_registry, ship, playerId)));
        component<rtype::game::PlayerShip>(ship).invulnerableFor = 0.0F;
        return ship;
    }

    engine::ecs::Entity spawnBydos(const engine::core::Vec2f position)
    {
        const engine::ecs::Entity bydos = _registry.spawnEntity();

        (void)rtype::game::makeBydosSlave(_registry, bydos, position,
            rtype::game::MovePattern::STRAIGHT);
        return bydos;
    }

    engine::ecs::Entity spawnPlayerMissile(const rtype::game::NetId owner,
        const std::uint8_t ownerPlayerId, const engine::core::Vec2f position)
    {
        const engine::ecs::Entity missile = _registry.spawnEntity();

        (void)rtype::game::makePlayerMissile(_registry, missile, owner,
            ownerPlayerId, position);
        return missile;
    }

    engine::ecs::Entity spawnEnemyMissile(const rtype::game::NetId owner,
        const engine::core::Vec2f position)
    {
        const engine::ecs::Entity missile = _registry.spawnEntity();

        (void)rtype::game::makeEnemyMissile(_registry, missile, owner,
            position, {-1.0F, 0.0F});
        return missile;
    }

    template <typename Component>
    Component &component(const engine::ecs::Entity entity)
    {
        auto *found = _registry.getComponent<Component>(entity);

        if (found == nullptr)
            throw std::logic_error("the entity lacks the component");
        return *found;
    }

    template <typename Component>
    [[nodiscard]] bool has(const engine::ecs::Entity entity)
    {
        return _registry.getComponent<Component>(entity) != nullptr;
    }

    template <typename Component>
    [[nodiscard]] std::size_t countOf()
    {
        std::size_t count = 0;

        for ([[maybe_unused]] auto &&[entity, found]:
             _registry.indexedView<const Component>())
            ++count;
        return count;
    }

    [[nodiscard]] rtype::game::NetId netIdOf(const engine::ecs::Entity entity)
    {
        return component<rtype::game::Replicated>(entity).netId;
    }

    void moveTo(const engine::ecs::Entity entity,
        const engine::core::Vec2f position)
    {
        component<rtype::game::Transform>(entity).position = position;
    }

    [[nodiscard]] std::optional<engine::ecs::Entity> find(
        const rtype::game::NetId netId)
    {
        return _registry.getResource<rtype::game::NetIdAllocator>().find(
            netId, _registry);
    }

    /*
    ** The entity a NetId names. Throws, failing the test, if it's unknown or
    ** dead.
    */
    [[nodiscard]] engine::ecs::Entity entityOf(const rtype::game::NetId netId)
    {
        const auto found = find(netId);

        if (!found)
            throw std::logic_error("no live entity has this NetId");
        return *found;
    }

    rtype::game::PlayerRoster &roster()
    {
        return _registry.getResource<rtype::game::PlayerRoster>();
    }

    /*
    ** Drains the outbox and keeps the events of one type, in push order.
    */
    template <typename Event>
    [[nodiscard]] std::vector<Event> eventsOf()
    {
        std::vector<Event> kept;

        for (const auto &event:
             _registry.getResource<rtype::game::EventOutbox>().drain())
            if (const auto *found = std::get_if<Event>(&event))
                kept.push_back(*found);
        return kept;
    }

    engine::ecs::Registry _registry;
    rtype::game::Rules _rules;
};

}

#endif //R_TYPE_GAMESYSTEMSFIXTURE_HPP
