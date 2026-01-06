/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SoundComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
#include <memory>

#include "rtype/display/IDisplay.hpp"

namespace rtype::games::rtype::client {
struct ButtonSoundComponent {
    std::shared_ptr<::rtype::display::ISoundBuffer> hoverSFX;
    std::shared_ptr<::rtype::display::ISoundBuffer> clickSFX;
};

struct EnemySoundComponent {
    std::shared_ptr<::rtype::display::ISoundBuffer> spawnSFX;
    std::shared_ptr<::rtype::display::ISoundBuffer> deathSFX;
};

struct PlayerSoundComponent {
    std::shared_ptr<::rtype::display::ISoundBuffer> spawnSFX;
    std::shared_ptr<::rtype::display::ISoundBuffer> deathSFX;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
