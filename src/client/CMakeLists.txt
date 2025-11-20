add_subdirectory(Graphic)

add_executable(r-type_client main.cpp)
target_link_libraries(r-type_client graphic)
set_target_properties(r-type_client PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)