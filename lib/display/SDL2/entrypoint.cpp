/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entrypoint.cpp - SDL2 Display entry point for dynamic loading
*/

#include "SDL2Display.hpp"

#if defined(_WIN32) || defined(_WIN64)
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

/**
 * @brief Create instance of SDL2 Display
 * @return Pointer to new SDL2Display instance implementing IDisplay
 */
RTYPE_EXPORT rtype::display::IDisplay* createInstanceDisplay(void) {
    return new rtype::display::SDL2Display();
}

}
