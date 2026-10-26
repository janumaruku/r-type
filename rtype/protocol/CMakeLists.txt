add_library(rtype_protocol STATIC
    src/MessageType.cpp)

target_include_directories(rtype_protocol PUBLIC include)

target_link_libraries(rtype_protocol PUBLIC network engine_core project_options)
