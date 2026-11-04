add_subdirectory(core)

add_subdirectory(ecs)

add_subdirectory(runtime)

add_subdirectory(input)

add_subdirectory(kollection)

if (RTYPE_BUILD_CLIENT)
    add_subdirectory(render)
    add_subdirectory(audio)
endif ()
