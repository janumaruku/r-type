add_library(network STATIC)
target_sources(network PRIVATE src/Packet.cpp)
target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC project_options)