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