/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LobbyBackground.cpp
*/

#include "LobbyBackground.hpp"

#include "rtype/client/GraphicsConstants.hpp"
#include "rtype/client/Components/ImageComponent.hpp"
#include "rtype/client/Components/ParallaxComponent.hpp"
#include "rtype/client/Components/ZIndexComponent.hpp"
#include "rtype/shared/Components/TransformComponent.hpp"

namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

void LobbyBackground::createEntitiesBackground() {
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

    auto bigAsteroids = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        bigAsteroids, "bg_big_asteroids");
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        bigAsteroids, cfg::PARALLAX_BIG_ASTEROIDS, true);
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        bigAsteroids, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        bigAsteroids, cfg::ZINDEX_BIG_ASTEROIDS);

    auto smallAsteroids = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        smallAsteroids, "bg_small_asteroids");
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        smallAsteroids, cfg::PARALLAX_SMALL_ASTEROIDS, true);
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        smallAsteroids, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        smallAsteroids, cfg::ZINDEX_SMALL_ASTEROIDS);

    auto firstPlanAsteroids = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        firstPlanAsteroids, "bg_fst_plan_asteroids");
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        firstPlanAsteroids, cfg::PARALLAX_ASTEROIDS_FST_PLAN, true);
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        firstPlanAsteroids, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        firstPlanAsteroids, cfg::ZINDEX_FST_PLAN_ASTEROIDS);

    auto secondPlanAsteroids = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        secondPlanAsteroids, "bg_snd_plan_asteroids");
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        secondPlanAsteroids, cfg::PARALLAX_ASTEROIDS_SND_PLAN, true);
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        secondPlanAsteroids, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        secondPlanAsteroids, cfg::ZINDEX_SND_PLAN_ASTEROIDS);

    auto planet1 = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet1, "bg_planet_1");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet1, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet1, cfg::PARALLAX_PLANET_1, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet1, cfg::ZINDEX_PLANETS);

    auto planet2 = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet2, "bg_planet_2");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet2, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet2, cfg::PARALLAX_PLANET_2, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet2, cfg::ZINDEX_PLANETS);

    auto planet3 = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        planet3, "bg_planet_3");
    this->_registry->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
        planet3, 0, 0);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Parallax>(
        planet3, cfg::PARALLAX_PLANET_3, true);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        planet3, cfg::ZINDEX_PLANETS);
    this->_listEntities = {background, sun, bigAsteroids, smallAsteroids,
                firstPlanAsteroids, secondPlanAsteroids, planet1, planet2,
                planet3};
}


