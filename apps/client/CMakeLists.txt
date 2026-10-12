add_executable(r-type_client src/main.cpp)
target_link_libraries(r-type_client PRIVATE rtype_game rtype_engine)

configure_file(${CMAKE_SOURCE_DIR}/config/client.cfg ${CMAKE_CURRENT_BINARY_DIR}/client.cfg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/keybindings.cfg ${CMAKE_CURRENT_BINARY_DIR}/keybindings.cfg COPYONLY)