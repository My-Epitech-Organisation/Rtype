/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypePauseMenu.cpp
*/
#include "RtypePauseMenu.hpp"

#include <memory>
#include <string>
#include <vector>

#include "AllComponents.hpp"
#include "Graphic.hpp"
#include "Graphic/EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"
#include "games/rtype/client/PauseState.hpp"

namespace rtype::games::rtype::client {

std::vector<ECS::Entity> RtypePauseMenu::createPauseMenu(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    std::vector<ECS::Entity> pauseEntities;

    auto sectionX = (Graphic::WINDOW_WIDTH - kSizeXPauseMenu) / 2.0f;
    auto sectionY = (Graphic::WINDOW_HEIGHT - kSizeYPauseMenu) / 2.0f;

    pauseEntities = EntityFactory::createSection(
        registry, assetsManager, "", {sectionX, sectionY},
        {static_cast<float>(kSizeXPauseMenu),
         static_cast<float>(kSizeYPauseMenu)});

    auto titleEntity = EntityFactory::createStaticText(
        registry, assetsManager, kPauseMenuTitle, "main_font",
        {(sectionX + kSizeXPauseMenu / 2.0f) -
             ((kPauseMenuTitle.length() - 2) * (kSizeFontPauseMenu / 2.0f)),
         sectionY},
        kSizeFontPauseMenu);

    pauseEntities.push_back(titleEntity);

    pauseEntities.push_back(EntityFactory::createButton(
        registry,
        Text("main_font", ::rtype::display::Color::White(), 30, "Menu"),
        ::rtype::games::rtype::shared::TransformComponent(
            sectionX + ((kSizeXPauseMenu / 2.0f) - (150.0f / 2.0f)),
            sectionY + kSizeYPauseMenu - 75.0f),
        Rectangle({150.0f, 55.0f}, ::rtype::display::Color::Blue(),
                  ::rtype::display::Color::Red()),
        assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(
                    "Error switching to Main Menu: " << std::string(e.what()));
            }
        })));

    pauseEntities.push_back(EntityFactory::createButton(
        registry,
        Text("main_font", ::rtype::display::Color::White(), 30, "Resume"),
        ::rtype::games::rtype::shared::TransformComponent(
            sectionX + ((kSizeXPauseMenu / 2.0f) - (150.0f / 2.0f)),
            sectionY + kSizeYPauseMenu - 150.0f),
        Rectangle({150.0f, 55.0f}, ::rtype::display::Color(0, 100, 200, 255),
                  ::rtype::display::Color(0, 140, 255, 255)),
        assetsManager, std::function<void()>([registry]() {
            RtypePauseMenu::togglePauseMenu(registry);
        })));

    for (auto& entt : pauseEntities) {
        registry->emplaceComponent<HiddenComponent>(entt, true);
        registry->emplaceComponent<PauseMenuTag>(entt);
    }

    return pauseEntities;
}

void RtypePauseMenu::togglePauseMenu(std::shared_ptr<ECS::Registry> registry) {
    if (!registry->hasSingleton<PauseState>()) {
        registry->setSingleton<PauseState>(PauseState{false});
    }

    auto& pauseState = registry->getSingleton<PauseState>();
    pauseState.isPaused = !pauseState.isPaused;

    registry->view<HiddenComponent, PauseMenuTag>().each(
        [](auto, HiddenComponent& hidden, auto) {
            hidden.isHidden = !hidden.isHidden;
        });
}

}  // namespace rtype::games::rtype::client
