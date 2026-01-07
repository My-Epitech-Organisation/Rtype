/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IMusic.hpp
*/

#ifndef R_TYPE_IMUSIC_HPP
#define R_TYPE_IMUSIC_HPP
#include <string>

namespace rtype::display {
    class IMusic {
    public:
        virtual ~IMusic() = default;

        virtual bool openFromFile(const std::string& path) = 0;
        virtual void setLooping(bool loop) = 0;
        virtual void setVolume(float volume) = 0;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
    };
}
#endif //R_TYPE_IMUSIC_HPP