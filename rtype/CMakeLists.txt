add_subdirectory(protocol)
add_subdirectory(game)

add_library(rtype_game INTERFACE)
target_link_libraries(rtype_game INTERFACE rtype_engine rtype_network rtype_protocol)
target_include_directories(rtype_game INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/game/include)