add_library(network STATIC
	src/Packet.cpp
	src/UdpSocket.cpp)

target_include_directories(network
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(network PUBLIC asio project_options)
