/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ISound.hpp
*/

#ifndef R_TYPE_ISOUND_HPP
#define R_TYPE_ISOUND_HPP

namespace rtype::display {
    /**
     * @brief Interface for short audio effects (SFX).
     * * This interface represents a sound instance that is typically
     * loaded entirely into memory via an ISoundBuffer.
     */
    class ISound {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~ISound() = default;

        /**
         * @brief Possible states for the sound playback.
         */
        enum class Status {
            Stopped, /**< Sound is not playing and is at the beginning. */
            Paused,  /**< Sound is frozen at the current playback position. */
            Playing  /**< Sound is currently playing. */
        };

        /**
         * @brief Sets the volume of the sound.
         * @param volume A value between 0.0f and 100.0f.
         */
        virtual void setVolume(float volume) = 0;

        /**
         * @brief Starts or resumes the sound playback.
         */
        virtual void play() = 0;

        /**
         * @brief Retrieves the current playback status.
         * @return The current Status (Stopped, Paused, or Playing).
         */
        virtual Status getStatus() const = 0;
    };
}

#endif //R_TYPE_ISOUND
