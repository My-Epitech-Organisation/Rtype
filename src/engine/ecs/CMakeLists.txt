add_library(rtype_engine_ecs STATIC
    Entity.cpp
    Registry.cpp
)

target_include_directories(rtype_engine_ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(rtype_engine_ecs PUBLIC cxx_std_20)
