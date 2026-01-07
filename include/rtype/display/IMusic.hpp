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
    /**
     * @brief Interface for music streaming.
     * Unlike sounds, music is typically streamed from a file.
     */
    class IMusic {
    public:
        virtual ~IMusic() = default;

        /**
         * @brief Open a music file for streaming.
         * @param path Path to the music file.
         * @return true if opening succeeded.
         */
        virtual bool openFromFile(const std::string& path) = 0;

        /**
         * @brief Set whether the music should loop after reaching the end.
         * @param loop True to enable looping.
         */
        virtual void setLooping(bool loop) = 0;

        /**
         * @brief Set the volume of the music.
         * @param volume Volume between 0 and 100.
         */
        virtual void setVolume(float volume) = 0;

        /**
         * @brief Start or resume playing the music.
         */
        virtual void play() = 0;

        /**
         * @brief Pause the music.
         */
        virtual void pause() = 0;

        /**
         * @brief Stop the music and reset to the beginning.
         */
        virtual void stop() = 0;
    };
}
#endif //R_TYPE_IMUSIC_HPP
