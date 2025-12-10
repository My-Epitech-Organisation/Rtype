/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SoundComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
#include <memory>
#include <SFML/Audio/SoundBuffer.hpp>

namespace rtype::games::rtype::client {
struct ButtonSoundComponent {
    std::shared_ptr<sf::SoundBuffer> hoverSFX;
    std::shared_ptr<sf::SoundBuffer> clickSFX;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SOUNDCOMPONENT_HPP_
