/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ADisplay.hpp
*/

#ifndef R_TYPE_ADISPLAY_HPP
#define R_TYPE_ADISPLAY_HPP
#include "include/rtype/display/IDisplay.hpp"

namespace rtype::display {

    class ADisplay : public IDisplay{
    protected:
        unsigned int _windowSizeWidth = 1920;
        unsigned int _windowSizeHeight = 1080;
        bool _windowIsFullscreen = false;
        std::string _windowTitleName = "";
    public:
        [[nodiscard]] std::string getLibName(void) const override;
    };
}

#endif //R_TYPE_ADISPLAY_HPP