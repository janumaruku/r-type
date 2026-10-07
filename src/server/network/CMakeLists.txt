add_library(network STATIC
	Packet.cpp
	UdpSocket.cpp)

target_include_directories(network
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(network PUBLIC asio project_options)
