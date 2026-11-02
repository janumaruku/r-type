rtype_add_test(server_conversions_tests
    LIBS server_logic
    SOURCES
        ConversionsTests.cpp
)

add_subdirectory(config)
add_subdirectory(session)