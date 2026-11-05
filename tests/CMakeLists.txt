include(GoogleTest)

add_subdirectory(engine)

add_subdirectory(server)

add_subdirectory(network)

add_subdirectory(protocol)

add_subdirectory(game)

if (RTYPE_BUILD_CLIENT)
    add_subdirectory(client)
endif ()
