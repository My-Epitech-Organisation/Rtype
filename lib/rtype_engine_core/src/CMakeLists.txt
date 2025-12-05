add_library(rtype_engine_core STATIC
    Time.cpp
)

target_include_directories(rtype_engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(rtype_engine_core PUBLIC cxx_std_20)

target_link_libraries(rtype_engine_core PUBLIC
    rtype_common
)
