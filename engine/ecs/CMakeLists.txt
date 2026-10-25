add_library(engine_ecs STATIC src/Entity.cpp
        src/TypeId.cpp
        src/SparseSetBase.cpp
        src/SparseSet.cpp
        src/Registry.cpp
        src/CommandBuffer.cpp
)

target_include_directories(engine_ecs PUBLIC include)

target_link_libraries(engine_ecs PUBLIC engine_core project_options)
