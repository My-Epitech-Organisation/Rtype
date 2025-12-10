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

    float _volumeMusic = 0;
    float _volumeSFX = 100;

   public:
    void setLoop(const bool& loop) const;
    void setMusicVolume(const float& volume);
    [[nodiscard]] float getMusicVolume() const;

    void pauseMusic() const;
    void play() const;

    static void playSFX(sf::SoundBuffer sfx);

    void loadMusic(std::shared_ptr<sf::Music> music);

    AudioLib() = default;
    ~AudioLib();
};

#endif  // SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_
