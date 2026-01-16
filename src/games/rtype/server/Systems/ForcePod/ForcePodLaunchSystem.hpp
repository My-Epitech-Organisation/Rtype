/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodLaunchSystem - Handles Force Pod launch and recall mechanics
*/

#pragma once

#include <functional>
#include <unordered_map>

#include <rtype/engine.hpp>

#include "../../../shared/Components/ForcePodComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"
#include "../../../shared/Components/VelocityComponent.hpp"

namespace rtype::games::rtype::server {

class ForcePodLaunchSystem : public ::rtype::engine::ASystem {
   public:
    using ForcePodInputCallback =
        std::function<void(std::uint32_t playerNetworkId, bool launch)>;

    ForcePodLaunchSystem();

    void update(ECS::Registry& registry, float deltaTime) override;

    void handleForcePodInput(ECS::Registry& registry,
                             std::uint32_t playerNetworkId);

    void setForcePodForPlayer(std::uint32_t playerNetworkId,
                              ECS::Entity forcePod);

    void removeForcePodForPlayer(std::uint32_t playerNetworkId);

   private:
    void launchForcePod(ECS::Registry& registry, ECS::Entity forcePod,
                        const shared::TransformComponent& playerTransform);

    void recallForcePod(ECS::Registry& registry, ECS::Entity forcePod);

    void updateDetachedPhysics(ECS::Registry& registry, float deltaTime);

    void updateReturningPods(ECS::Registry& registry, float deltaTime);

    void checkReattachment(ECS::Registry& registry);

    std::unordered_map<std::uint32_t, ECS::Entity> _playerForcePods;

    static constexpr float kLaunchSpeed = 400.0F;
    static constexpr float kReturnSpeed = 500.0F;
    static constexpr float kDeceleration = 50.0F;
    static constexpr float kReattachDistance = 50.0F;
    static constexpr float kMaxDetachDistance = 800.0F;
};

}  // namespace rtype::games::rtype::server
