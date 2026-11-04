add_executable(r-type_client src/main.cpp
    src/ClientConfig.cpp
    src/ClientSettings.cpp)

target_link_libraries(r-type_client PRIVATE project_options)
target_link_libraries(r-type_client PUBLIC include)

set_target_properties(r-type_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")