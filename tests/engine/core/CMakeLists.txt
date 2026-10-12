add_executable(engine_core_tests)
target_sources(engine_core_tests PRIVATE
    Vec2Tests.cpp
    Rec2tests.cpp
    ColorTests.cpp
    ErrorTests.cpp
    LoggerTests.cpp
    RngTests.cpp
)
target_link_libraries(engine_core_tests PRIVATE engine_core GTest::gtest_main)
gtest_discover_tests(engine_core_tests)