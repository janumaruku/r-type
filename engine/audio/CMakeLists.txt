add_library(engine_audio STATIC
        src/IAudio.cpp
        src/Volume.cpp
        src/NullAudio.cpp
        src/Voices.cpp
        src/MakeAudio.cpp
        src/raylib/RaylibAudio.cpp
)

target_include_directories(engine_audio PUBLIC include)
target_link_libraries(engine_audio PUBLIC engine_core project_options PRIVATE raylib)
