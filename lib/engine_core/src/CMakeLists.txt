add_library(engine_core STATIC
    Time.cpp
)

target_include_directories(engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(engine_core PUBLIC cxx_std_20)

target_link_libraries(engine_core PUBLIC
    common
)
