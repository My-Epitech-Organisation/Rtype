/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BlackHole.cpp
*/

#include "BlackHole.hpp"
#include "rtype/client/GraphicsConstants.hpp"
#include "rtype/client/Components/ImageComponent.hpp"
#include "rtype/client/Components/ParallaxComponent.hpp"
#include "rtype/client/Components/ZIndexComponent.hpp"
#include "rtype/shared/Components/TransformComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

void BlackHole::createEntitiesBackground() {
    this->_assetManager->textureManager->load("bg_blackHole", "assets/img/blackHole.png");
    auto background = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   "bg_menu");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        background, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        background, rtype::games::rtype::client::GraphicsConfig::ZINDEX_BACKGROUND);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, rtype::games::rtype::client::GraphicsConfig::PARALLAX_BACKGROUND, true);

    auto sun = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(sun,
                                                                   "bg_sun");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        sun, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        sun, cfg::ZINDEX_SUN);

    auto blackHole = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(blackHole,
                                                                   "bg_blackHole");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        blackHole, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        blackHole, -3);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        blackHole, 0.02, true);

    this->_listEntities = {background, sun, blackHole};
}
