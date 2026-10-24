rtype_add_test(engine_render_tests
    LIBS engine_render
    SOURCES
    RenderQueueTests.cpp
    ResourcePathTests.cpp
)

rtype_add_test(engine_render_raylib_tests
    LIBS engine_input raylib
    SOURCES
    KeyTableTests.cpp
)

target_include_directories(engine_render_raylib_tests PRIVATE "${PROJECT_SOURCE_DIR}/engine/render/src")
