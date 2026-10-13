add_library(engine_ecs STATIC src/Entity.cpp)

target_include_directories(engine_ecs PUBLIC include)

target_link_libraries(engine_ecs PUBLIC project_options)
