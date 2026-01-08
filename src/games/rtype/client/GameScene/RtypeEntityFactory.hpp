/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeEntityFactory.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEENTITYFACTORY_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEENTITYFACTORY_HPP_

#include <memory>

#include "../../../../client/Graphic/AssetManager/AssetManager.hpp"
#include "../../../../client/network/ClientNetworkSystem.hpp"
#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Factory for creating R-Type specific entities
 *
 * Handles the creation of game entities like players, enemies, and missiles
 * with their appropriate components.
 */
class RtypeEntityFactory {
   public:
    /**
     * @brief Create entity factory callback for network system
     *
     * @param registry ECS registry
     * @param assetsManager Asset manager for textures
     * @return Entity factory function
     */
    static ::rtype::client::ClientNetworkSystem::EntityFactory
    createNetworkEntityFactory(std::shared_ptr<ECS::Registry> registry,
                               std::shared_ptr<AssetManager> assetsManager);

    /**
     * @brief Create a player entity with all components
     *
     * @param reg ECS registry
     * @param assetsManager Asset manager
     * @param entity Entity to configure
     * @param userId User ID for the player (used to determine player color)
     */
    static void setupPlayerEntity(ECS::Registry& reg,
                                  std::shared_ptr<AssetManager> assetsManager,
                                  ECS::Entity entity, std::uint32_t userId = 0);

    /**
     * @brief Create a Bydos enemy entity with all components
     *
     * @param reg ECS registry
     * @param assetsManager Asset manager
     * @param entity Entity to configure
     * @param subType Enemy variant (0=Basic, 1=Shooter, 2=Chaser, etc...)
     */
    static void setupBydosEntity(ECS::Registry& reg,
                                 std::shared_ptr<AssetManager> assetsManager,
                                 ECS::Entity entity, std::uint8_t subType = 0);

    /**
     * @brief Create a missile entity with all components
     *
     * @param reg ECS registry
     * @param entity Entity to configure
     */
    static void setupMissileEntity(ECS::Registry& reg,
                                   std::shared_ptr<AssetManager> assetsManager,
                                   ECS::Entity entity);
    /**
     * @brief Create a pickup entity with all components
     *
     * @param registry ECS registry
     * @param entity Entity to configure
     * @param networkId Network identifier for the entity
     * @param subType Power-up variant type
     */
    static void setupPickupEntity(ECS::Registry& registry, ECS::Entity entity,
                                  std::uint32_t networkId, uint8_t subType);
    /**
     * @brief Create an obstacle entity with all components
     *
     * @param registry ECS registry
     * @param assetsManager Asset manager
     * @param entity Entity to configure
     * @param networkId Network identifier for the entity
     */
    static void setupObstacleEntity(ECS::Registry& registry,
                                    std::shared_ptr<AssetManager> assetsManager,
                                    ECS::Entity entity,
                                    std::uint32_t networkId);

    /**
     * @brief Create a Force Pod entity with all components
     *
     * @param registry ECS registry
     * @param assetsManager Asset manager
     * @param entity Entity to configure
     */
    static void setupForcePodEntity(ECS::Registry& registry,
                                    std::shared_ptr<AssetManager> assetsManager,
                                    ECS::Entity entity);
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEENTITYFACTORY_HPP_
