/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entrypoint.cpp
*/

#include <iostream>
#include <unistd.h>
#include "SFMLDisplay.hpp"
#include "SFMLMusic.hpp"
#include "SFMLTexture.hpp"

extern "C" {
    rtype::display::IDisplay* createInstanceDisplay(void) {
        return new rtype::display::SFMLDisplay();
    }
}