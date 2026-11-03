add_library(client_logic STATIC
        src/ServerLink.cpp
)

target_include_directories(client_logic PUBLIC include)

target_link_libraries(client_logic PUBLIC engine_core network rtype_protocol project_options)

add_executable(r-type_client src/main.cpp)

target_link_libraries(r-type_client PRIVATE project_options)

set_target_properties(r-type_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")