add_subdirectory(rtype)

# Only build snake if BUILD_SNAKE is enabled
if(BUILD_SNAKE)
    add_subdirectory(snake)
endif()
