/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ISound.hpp
*/

#ifndef R_TYPE_ISOUND_HPP
#define R_TYPE_ISOUND_HPP

namespace rtype::display {
    class ISound {
        public:
            virtual ~ISound() = default;

            enum class Status {
                Stopped,
                Paused,
                Playing
            };
            virtual void setVolume(float volume) = 0;
            virtual void play() = 0;
            virtual Status getStatus() const = 0;
    };
}

#endif //R_TYPE_ISOUND_HPP