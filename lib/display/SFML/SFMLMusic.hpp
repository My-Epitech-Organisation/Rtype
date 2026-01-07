/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLMusic.hpp
*/

#ifndef R_TYPE_SFMLMUSIC_HPP
#define R_TYPE_SFMLMUSIC_HPP
#include <SFML/Audio.hpp>
#include "include/rtype/display/IDisplay.hpp"

namespace rtype::display {
    class SFMLMusic : public rtype::display::IMusic {
    public:
        bool openFromFile(const std::string& path) override;
        void setLooping(bool loop) override;
        void setVolume(float volume) override;
        void play() override;
        void pause() override;
        void stop() override;

    private:
        sf::Music _music;
    };
};


#endif //R_TYPE_SFMLMUSIC_HPP