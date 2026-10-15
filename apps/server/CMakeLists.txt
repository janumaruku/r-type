add_executable(r-type_server src/main.cpp)

target_link_libraries(r-type_server PRIVATE project_options network)

set_target_properties(r-type_server PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")
