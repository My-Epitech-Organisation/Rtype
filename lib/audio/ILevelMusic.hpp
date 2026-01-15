/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ILevelMusic
*/

#ifndef ILEVELMUSIC_HPP_
#define ILEVELMUSIC_HPP_

#include <string>
#include <memory>

class AudioLib;

class ILevelMusic {
public:
    /**
     * @brief Loads and plays the music associated with the level.
     * 
     * @param audioLib Shared pointer to the AudioLib instance, used to load and control audio playback.
     */
    virtual void loadLevelMusic(std::shared_ptr<AudioLib> audioLib) = 0;

    /**
     * @brief Unloads and stops the level music.
     * 
     * This function should ensure that the currently playing music is stopped and
     * any resources specific to this level are released if necessary.
     */
    virtual void unloadLevelMusic() = 0;

    /**
     * @brief Retrieves the name of the level music configuration.
     * 
     * @return The name of the level music as a string.
     */
    virtual std::string getLevelMusicName() = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~ILevelMusic() = default;
};

#endif /* !ILEVELMUSIC_HPP_ */
