/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entrypoint.cpp
*/

#ifdef _WIN32
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT
#endif

#include <iostream>
#include "SFMLDisplay.hpp"

extern "C" {
    RTYPE_EXPORT rtype::display::IDisplay* createInstanceDisplay(void) {
        return new rtype::display::SFMLDisplay();
    }
}
