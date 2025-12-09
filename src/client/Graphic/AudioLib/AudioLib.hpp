/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioLib.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_
#define SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_

#include "SFML/Audio.hpp"

class AudioLib {
   private:
    std::shared_ptr<sf::Music> _currentMusic;

    float _volume = 50;

   public:
    void setLoop(const bool& loop) const;
    void setMusicVolume(const float& volume);
    [[nodiscard]] float getMusicVolume() const;

    void pauseMusic() const;
    void play() const;

    void loadMusic(std::shared_ptr<sf::Music> music);

    AudioLib() = default;
    ~AudioLib();
};

#endif  // SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_
