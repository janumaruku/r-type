include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.5
    OPTIONS
        "BUILD_EXAMPLES OFF"
        "BUILD_GAMES OFF")

set(GTEST_OPTIONS
    "INSTALL_GTEST OFF"
    "BUILD_GMOCK OFF"
    "gtest_build_tests OFF")

if(WIN32)
    list(APPEND GTEST_OPTIONS "gtest_force_shared_crt ON")
endif()

CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.15.2
    OPTIONS ${GTEST_OPTIONS})