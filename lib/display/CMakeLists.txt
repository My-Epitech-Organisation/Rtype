add_subdirectory(SFML)

# Only build SDL2 if not doing coverage builds (tests only use SFML)
if(NOT ENABLE_COVERAGE)
    add_subdirectory(SDL2)
endif()

add_subdirectory(Clock)
