add_library(rtype_engine_core STATIC
    Time.cpp
)

target_include_directories(rtype_engine_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(rtype_engine_core PUBLIC cxx_std_20)
