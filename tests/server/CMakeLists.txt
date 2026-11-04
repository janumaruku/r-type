rtype_add_test(server_conversions_tests
    LIBS server_logic
    SOURCES
        ConversionsTests.cpp
)

rtype_add_test(server_dispatch_tests
    LIBS server_logic
    SOURCES
        DispatchTests.cpp
)

rtype_add_test(server_commands_tests
    LIBS server_logic
    SOURCES
        CommandsTests.cpp
)

add_subdirectory(config)
add_subdirectory(session)
add_subdirectory(event)
add_subdirectory(goodbyes)
add_subdirectory(snapshot)

rtype_add_test(server_integration_tests
    LIBS server_logic
    SOURCES
        IntegrationTests.cpp
)

target_include_directories(server_integration_tests
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}"
)

rtype_add_test(server_app_tests
    LIBS server_logic
    SOURCES
        ServerAppTests.cpp
)