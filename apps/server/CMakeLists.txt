add_executable(r-type_server src/main.cpp)
target_link_libraries(r-type_server PRIVATE rtype_game rtype_network)

configure_file(${CMAKE_SOURCE_DIR}/config/client.cfg ${CMAKE_CURRENT_BINARY_DIR}/client.cfg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/keybindings.cfg ${CMAKE_CURRENT_BINARY_DIR}/keybindings.cfg COPYONLY)