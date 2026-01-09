/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioLib.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_
#define SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_

#include <list>

#include "rtype/display/IDisplay.hpp"

class AudioLib {
   private:
    std::shared_ptr<::rtype::display::IMusic> _currentMusic;
    std::shared_ptr<::rtype::display::IDisplay> _display;

    float _volumeMusic = 50;
    float _volumeSFX = 25;

    std::list<std::shared_ptr<::rtype::display::ISound>> _sounds;

   public:
    void setLoop(const bool& loop) const;
    void setMusicVolume(const float& volume);
    [[nodiscard]] float getMusicVolume() const;

    void setSFXVolume(const float& volume);
    [[nodiscard]] float getSFXVolume() const;

    void pauseMusic() const;
    void play() const;

    void playSFX(std::shared_ptr<rtype::display::ISoundBuffer> sfx);

    void loadMusic(std::shared_ptr<::rtype::display::IMusic> music);

    explicit AudioLib(std::shared_ptr<::rtype::display::IDisplay> display)
        : _display(display) {}
    ~AudioLib();
};

#endif  // SRC_CLIENT_GRAPHIC_AUDIOLIB_AUDIOLIB_HPP_
