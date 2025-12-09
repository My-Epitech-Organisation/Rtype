/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypePauseMenu.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEPAUSEMENU_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEPAUSEMENU_HPP_

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "ECS.hpp"
#include "Graphic/AssetManager/AssetManager.hpp"
#include "SceneManager/SceneManager.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Handles R-Type pause menu creation and management
 */
class RtypePauseMenu {
   public:
    static constexpr int kSizeXPauseMenu = 600;
    static constexpr int kSizeYPauseMenu = 600;
    static constexpr int kSizeFontPauseMenu = 40;
    static constexpr std::string_view kPauseMenuTitle = "Pause";

    /**
     * @brief Create the pause menu entities
     *
     * @param registry ECS registry
     * @param assetsManager Asset manager
     * @param switchToScene Callback to switch scenes
     * @return Vector of created entities
     */
    static std::vector<ECS::Entity> createPauseMenu(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<AssetManager> assetsManager,
        std::function<void(const SceneManager::Scene&)> switchToScene);

    /**
     * @brief Toggle pause menu visibility
     *
     * @param registry ECS registry
     */
    static void togglePauseMenu(std::shared_ptr<ECS::Registry> registry);
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEPAUSEMENU_HPP_
