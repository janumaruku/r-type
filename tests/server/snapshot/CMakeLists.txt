rtype_add_test(server_snapshot_builder_tests
    LIBS server_logic
    SOURCES
        SnapshotBuilderTests.cpp
)

target_include_directories(server_snapshot_builder_tests
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/.."
)
