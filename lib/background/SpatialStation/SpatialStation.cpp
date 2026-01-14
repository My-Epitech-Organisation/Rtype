/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpatialStation.cpp
*/

#include "SpatialStation.hpp"
#include "rtype/client/GraphicsConstants.hpp"
#include "rtype/client/Components/ImageComponent.hpp"
#include "rtype/client/Components/ParallaxComponent.hpp"
#include "rtype/client/Components/ZIndexComponent.hpp"
#include "rtype/shared/Components/TransformComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

void SpatialStation::createEntitiesBackground() {
    this->_assetManager->textureManager->load("bg_spatialStation", "assets/img/spatialStation.png");
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

    auto spatialStation = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(spatialStation,
                                                                   "bg_spatialStation");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        spatialStation, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        spatialStation, -3);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        spatialStation, 0.75, true);

    this->_listEntities = {background, sun, spatialStation};
}
