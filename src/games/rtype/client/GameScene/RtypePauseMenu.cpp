/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypePauseMenu.cpp
*/

#include "RtypePauseMenu.hpp"

#include "AllComponents.hpp"
#include "Graphic.hpp"
#include "Graphic/EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

namespace rtype::games::rtype::client {

std::vector<ECS::Entity> RtypePauseMenu::createPauseMenu(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::function<void(const SceneManager::Scene&)> switchToScene) {
    std::vector<ECS::Entity> pauseEntities;

    auto sectionX = (Graphic::WINDOW_WIDTH - kSizeXPauseMenu) / 2;
    auto sectionY = (Graphic::WINDOW_HEIGHT - kSizeYPauseMenu) / 2;

    pauseEntities = EntityFactory::createSection(
        registry, assetsManager, "",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(kSizeXPauseMenu, kSizeYPauseMenu)));

    auto titleEntity = EntityFactory::createStaticText(
        registry, assetsManager, kPauseMenuTitle, "title_font",
        sf::Vector2f(
            (sectionX + kSizeXPauseMenu / 2) -
                ((kPauseMenuTitle.length() - 2) * (kSizeFontPauseMenu / 2)),
            sectionY),
        kSizeFontPauseMenu);

    auto& titleText = registry->getComponent<Text>(titleEntity);
    sf::FloatRect bounds = titleText.text.getLocalBounds();
    float centeredX = sectionX + (kSizeXPauseMenu - bounds.size.x) / 2;
    auto& titlePos =
        registry->getComponent<::rtype::games::rtype::shared::Position>(
            titleEntity);
    titlePos.x = centeredX;
    pauseEntities.push_back(titleEntity);

    pauseEntities.push_back(EntityFactory::createButton(
        registry,
        Text(assetsManager->fontManager->get("title_font"), sf::Color::White,
             30, "Menu"),
        ::rtype::games::rtype::shared::Position(
            sectionX + ((kSizeXPauseMenu / 2) - (150 / 2)),
            sectionY + kSizeYPauseMenu - 75),
        Rectangle({150, 55}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(
                    "Error switching to Main Menu: " << std::string(e.what()));
            }
        })));

    for (auto& entt : pauseEntities) {
        registry->emplaceComponent<HiddenComponent>(entt, true);
        registry->emplaceComponent<PauseMenuTag>(entt);
    }

    return pauseEntities;
}

void RtypePauseMenu::togglePauseMenu(std::shared_ptr<ECS::Registry> registry) {
    registry->view<HiddenComponent, PauseMenuTag>().each(
        [](auto, HiddenComponent& hidden, auto) {
            hidden.isHidden = !hidden.isHidden;
        });
}

}  // namespace rtype::games::rtype::client
