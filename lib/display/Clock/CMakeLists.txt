add_library(rtype-display-clock STATIC
    Clock.cpp
    Clock.hpp
)

# Enable Position Independent Code (PIC)
set_property(TARGET rtype-display-clock PROPERTY POSITION_INDEPENDENT_CODE ON)

