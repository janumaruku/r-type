add_library(network STATIC)
target_sources(network PRIVATE
    src/Packet.cpp
    src/Serializers.cpp
    src/UdpSocket.cpp
)
target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC asio engine_core)
