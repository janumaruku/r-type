add_library(ecs STATIC src/entity.cpp)

target_include_directories(ecs PUBLIC .)

target_link_libraries(ecs PUBLIC project_options)
