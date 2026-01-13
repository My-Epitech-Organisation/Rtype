/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ILevelMusic
*/

#ifndef ILEVELMUSIC_HPP_
#define ILEVELMUSIC_HPP_

#include <string>

class ILevelMusic {
public:
    virtual void loadLevelMusic() = 0;
    virtual void unloadLevelMusic() = 0;
    virtual std::string getLevelMusicName() = 0;
    virtual ~ILevelMusic() = default;
};

#endif /* !ILEVELMUSIC_HPP_ */
