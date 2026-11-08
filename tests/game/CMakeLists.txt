rtype_add_test(game_tests
    LIBS rtype_game
    SOURCES
    BuildersTests.cpp
    CollisionTests.cpp
    ComponentsTests.cpp
    DeterminismTests.cpp
    EnemyBehaviourTests.cpp
    EventOutboxTests.cpp
    GameOverTests.cpp
    GamePhaseTests.cpp
    GameWorldTests.cpp
    HealthTests.cpp
    LifetimeTests.cpp
    MotionSystemsTests.cpp
    MotionTests.cpp
    NetIdAllocatorTests.cpp
    NetIdCleanupTests.cpp
    PlayerRosterTests.cpp
    RespawnTests.cpp
    RulesLoadTests.cpp
    RulesTests.cpp
    ScenarioTests.cpp
    SpawnDirectorTests.cpp
    TypesTests.cpp
    WeaponTests.cpp
)

target_compile_definitions(game_tests PRIVATE
    RTYPE_CONFIG_DIR="${PROJECT_SOURCE_DIR}/config")
