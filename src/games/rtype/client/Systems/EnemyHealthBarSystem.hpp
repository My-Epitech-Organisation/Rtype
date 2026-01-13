/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EnemyHealthBarSystem.hpp - System to render health bars above enemies
*/

#pragma once

#include <memory>
#include <unordered_map>

#include "ECS.hpp"
#include "rtype/display/IDisplay.hpp"
#include "rtype/engine.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System that creates and updates health bars above enemies
 *
 * This system:
 * - Detects new enemies and creates health bar entities
 * - Updates health bar position to follow enemy
 * - Updates health bar fill based on current health
 * - Removes health bars when enemy dies
 */
class EnemyHealthBarSystem : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<ECS::Registry> _registry;
    std::unordered_map<std::uint32_t, ECS::Entity> _healthBarBgs;
    std::unordered_map<std::uint32_t, ECS::Entity> _healthBarFills;

    static constexpr float HEALTH_BAR_WIDTH = 50.0f;
    static constexpr float HEALTH_BAR_HEIGHT = 4.0f;
    static constexpr float HEALTH_BAR_OFFSET_Y = -30.0f;

   public:
    explicit EnemyHealthBarSystem(std::shared_ptr<ECS::Registry> registry);
    ~EnemyHealthBarSystem() override = default;

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    /**
     * @brief Create health bar entities for a new enemy
     * @param enemy The enemy entity
     * @param registry The ECS registry
     */
    void createHealthBar(ECS::Entity enemy, ECS::Registry& registry);

    /**
     * @brief Update health bar position and size
     * @param enemy The enemy entity
     * @param registry The ECS registry
     */
    void updateHealthBar(ECS::Entity enemy, ECS::Registry& registry);

    /**
     * @brief Remove health bar entities for a dead enemy
     * @param enemy The enemy entity
     * @param registry The ECS registry
     */
    void removeHealthBar(ECS::Entity enemy, ECS::Registry& registry);
};

}  // namespace rtype::games::rtype::client
