add_subdirectory(core)
add_subdirectory(input)
add_subdirectory(ecs)

add_subdirectory(runtime)
if (RTYPE_BUILD_CLIENT)
    add_subdirectory(render)
endif ()
