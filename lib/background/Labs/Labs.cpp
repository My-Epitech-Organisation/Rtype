/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Labs.cpp
*/

#include "Labs.hpp"
#include "rtype/client/GraphicsConstants.hpp"
#include "rtype/client/Components/ImageComponent.hpp"
#include "rtype/client/Components/ParallaxComponent.hpp"
#include "rtype/client/Components/ZIndexComponent.hpp"
#include "rtype/shared/Components/TransformComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

void Labs::createEntitiesBackground() {
    auto background = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(background,
                                                                   "bg_menu");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        background, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        background, rtype::games::rtype::client::GraphicsConfig::ZINDEX_BACKGROUND);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        background, cfg::PARALLAX_BACKGROUND, true);

    auto sun = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(sun,
                                                                   "bg_sun");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        sun, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        sun, cfg::ZINDEX_SUN);

    this->_listEntities = {background, sun};
}
