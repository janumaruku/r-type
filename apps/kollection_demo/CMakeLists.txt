add_executable(r-type_kollection_demo src/main.cpp)

target_link_libraries(r-type_kollection_demo PRIVATE kollection_core engine_render engine_input engine_core project_options)

set_target_properties(r-type_kollection_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")
