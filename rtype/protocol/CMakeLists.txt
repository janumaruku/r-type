add_library(rtype_protocol STATIC
        src/MessageType.cpp
        src/ProtocolError.cpp
        src/Header.cpp
        src/Messages.cpp
        src/Snapshot.cpp
        src/Event.cpp)

target_include_directories(rtype_protocol PUBLIC include)

target_link_libraries(rtype_protocol PUBLIC network engine_core project_options)
