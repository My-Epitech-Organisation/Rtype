/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodLaunchSystem - Implementation
*/

#include "ForcePodLaunchSystem.hpp"

#include <cmath>

namespace rtype::games::rtype::server {

using shared::ForcePodComponent;
using shared::ForcePodState;
using shared::ForcePodTag;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::TransformComponent;
using shared::VelocityComponent;

ForcePodLaunchSystem::ForcePodLaunchSystem()
    : ASystem("ForcePodLaunchSystem") {}

void ForcePodLaunchSystem::update(ECS::Registry& registry, float deltaTime) {
    updateDetachedPhysics(registry, deltaTime);
    updateReturningPods(registry, deltaTime);
    checkReattachment(registry);
}

void ForcePodLaunchSystem::handleForcePodInput(ECS::Registry& registry,
                                                std::uint32_t playerNetworkId) {
    auto it = _playerForcePods.find(playerNetworkId);
    if (it == _playerForcePods.end()) {
        return;
    }

    ECS::Entity forcePod = it->second;
    if (!registry.isAlive(forcePod) ||
        !registry.hasComponent<ForcePodComponent>(forcePod)) {
        return;
    }

    auto& forcePodComp = registry.getComponent<ForcePodComponent>(forcePod);

    auto playerView = registry.view<NetworkIdComponent, TransformComponent, PlayerTag>();
    TransformComponent playerTransform{};
    bool playerFound = false;

    playerView.each([&](ECS::Entity, const NetworkIdComponent& netId,
                       const TransformComponent& transform, const PlayerTag&) {
        if (netId.networkId == playerNetworkId) {
            playerTransform = transform;
            playerFound = true;
        }
    });

    if (!playerFound) {
        return;
    }

    if (forcePodComp.state == ForcePodState::Attached) {
        launchForcePod(registry, forcePod, playerTransform);
    } else if (forcePodComp.state == ForcePodState::Detached) {
        recallForcePod(registry, forcePod);
    }
}

void ForcePodLaunchSystem::setForcePodForPlayer(std::uint32_t playerNetworkId,
                                                ECS::Entity forcePod) {
    _playerForcePods[playerNetworkId] = forcePod;
}

void ForcePodLaunchSystem::removeForcePodForPlayer(std::uint32_t playerNetworkId) {
    _playerForcePods.erase(playerNetworkId);
}

void ForcePodLaunchSystem::launchForcePod(ECS::Registry& registry,
                                          ECS::Entity forcePod,
                                          const TransformComponent& playerTransform) {
    auto& forcePodComp = registry.getComponent<ForcePodComponent>(forcePod);
    forcePodComp.state = ForcePodState::Detached;

    if (!registry.hasComponent<VelocityComponent>(forcePod)) {
        registry.emplaceComponent<VelocityComponent>(forcePod, kLaunchSpeed, 0.0F);
    } else {
        auto& vel = registry.getComponent<VelocityComponent>(forcePod);
        vel.vx = kLaunchSpeed;
        vel.vy = 0.0F;
    }
}

void ForcePodLaunchSystem::recallForcePod(ECS::Registry& registry,
                                          ECS::Entity forcePod) {
    auto& forcePodComp = registry.getComponent<ForcePodComponent>(forcePod);
    forcePodComp.state = ForcePodState::Returning;
}

void ForcePodLaunchSystem::updateDetachedPhysics(ECS::Registry& registry,
                                                  float deltaTime) {
    auto view = registry.view<ForcePodComponent, VelocityComponent, ForcePodTag>();

    view.each([this, deltaTime](ECS::Entity, ForcePodComponent& forcePod,
                                VelocityComponent& vel, const ForcePodTag&) {
        if (forcePod.state != ForcePodState::Detached) {
            return;
        }

        if (vel.vx > 0.0F) {
            vel.vx = std::max(0.0F, vel.vx - kDeceleration * deltaTime);
        } else if (vel.vx < 0.0F) {
            vel.vx = std::min(0.0F, vel.vx + kDeceleration * deltaTime);
        }

        if (vel.vy > 0.0F) {
            vel.vy = std::max(0.0F, vel.vy - kDeceleration * deltaTime);
        } else if (vel.vy < 0.0F) {
            vel.vy = std::min(0.0F, vel.vy + kDeceleration * deltaTime);
        }
    });
}

void ForcePodLaunchSystem::updateReturningPods(ECS::Registry& registry,
                                                float deltaTime) {
    auto podView = registry.view<ForcePodComponent, TransformComponent,
                                 VelocityComponent, ForcePodTag>();

    podView.each([this, &registry, deltaTime](
                    ECS::Entity, ForcePodComponent& forcePod,
                    TransformComponent& podTransform, VelocityComponent& vel,
                    const ForcePodTag&) {
        if (forcePod.state != ForcePodState::Returning) {
            return;
        }

        auto playerView = registry.view<NetworkIdComponent, TransformComponent, PlayerTag>();
        bool playerFound = false;
        TransformComponent playerTransform{};

        playerView.each([&](ECS::Entity, const NetworkIdComponent& netId,
                           const TransformComponent& transform, const PlayerTag&) {
            if (netId.networkId == forcePod.ownerNetworkId) {
                playerTransform = transform;
                playerFound = true;
            }
        });

        if (!playerFound) {
            return;
        }

        float dx = playerTransform.x - podTransform.x;
        float dy = playerTransform.y - podTransform.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0.1F) {
            float dirX = dx / distance;
            float dirY = dy / distance;

            vel.vx = dirX * kReturnSpeed;
            vel.vy = dirY * kReturnSpeed;
        } else {
            vel.vx = 0.0F;
            vel.vy = 0.0F;
        }
    });
}

void ForcePodLaunchSystem::checkReattachment(ECS::Registry& registry) {
    auto podView = registry.view<ForcePodComponent, TransformComponent, ForcePodTag>();

    podView.each([this, &registry](ECS::Entity forcePodEntity,
                                   ForcePodComponent& forcePod,
                                   const TransformComponent& podTransform,
                                   const ForcePodTag&) {
        if (forcePod.state == ForcePodState::Attached) {
            return;
        }

        auto playerView = registry.view<NetworkIdComponent, TransformComponent, PlayerTag>();
        bool reattached = false;
        bool autoRecall = false;

        playerView.each([&](ECS::Entity, const NetworkIdComponent& netId,
                           const TransformComponent& playerTransform,
                           const PlayerTag&) {
            if (netId.networkId != forcePod.ownerNetworkId) {
                return;
            }

            float dx = playerTransform.x - podTransform.x;
            float dy = playerTransform.y - podTransform.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= kReattachDistance && forcePod.state == ForcePodState::Returning) {
                forcePod.state = ForcePodState::Attached;
                if (registry.hasComponent<VelocityComponent>(forcePodEntity)) {
                    auto& vel = registry.getComponent<VelocityComponent>(forcePodEntity);
                    vel.vx = 0.0F;
                    vel.vy = 0.0F;
                }
                reattached = true;
            } else if (distance >= kMaxDetachDistance && forcePod.state == ForcePodState::Detached) {
                autoRecall = true;
            }
        });

        if (autoRecall && !reattached) {
            forcePod.state = ForcePodState::Returning;
        }
    });
}

}  // namespace rtype::games::rtype::server
