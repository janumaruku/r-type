add_library(rtype_engine INTERFACE)
target_link_libraries(rtype_engine INTERFACE project_options raylib)
target_include_directories(rtype_engine INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/core/include
	${CMAKE_CURRENT_SOURCE_DIR}/ecs/include
	${CMAKE_CURRENT_SOURCE_DIR}/runtime/include
	${CMAKE_CURRENT_SOURCE_DIR}/render/include
	${CMAKE_CURRENT_SOURCE_DIR}/input/include
	${CMAKE_CURRENT_SOURCE_DIR}/audio/include)

add_subdirectory(core)
add_subdirectory(ecs)
add_subdirectory(runtime)
add_subdirectory(render)
add_subdirectory(input)
add_subdirectory(audio)