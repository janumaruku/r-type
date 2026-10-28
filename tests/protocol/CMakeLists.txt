rtype_add_test(protocol_tests
    LIBS rtype_protocol
    SOURCES
    DirectionTests.cpp
    EventTests.cpp
    FixedMessageTests.cpp
    HeaderTests.cpp
    LoopbackTests.cpp
    MalformedInputTests.cpp
    MessageTypeTests.cpp
    NoAsioTests.cpp
    ProtocolErrorTests.cpp
    SanitizeNameTests.cpp
    SnapshotTests.cpp
    SplitSnapshotTests.cpp
)
