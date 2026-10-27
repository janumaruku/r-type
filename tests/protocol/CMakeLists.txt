rtype_add_test(protocol_tests
    LIBS rtype_protocol
    SOURCES
    HeaderTests.cpp
    MessageTypeTests.cpp
    NoAsioTests.cpp
    ProtocolErrorTests.cpp
)
