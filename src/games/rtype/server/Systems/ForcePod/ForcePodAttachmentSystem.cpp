/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodAttachmentSystem - Implementation
*/

#include "ForcePodAttachmentSystem.hpp"
#include "ForcePodLaunchSystem.hpp"

namespace rtype::games::rtype::server {

using shared::ForcePodComponent;
using shared::ForcePodState;
using shared::ForcePodTag;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::TransformComponent;

ForcePodAttachmentSystem::ForcePodAttachmentSystem()
    : ASystem("ForcePodAttachmentSystem") {}

void ForcePodAttachmentSystem::update(ECS::Registry& registry,
                                      float /*deltaTime*/) {
    updateAttachedPods(registry);
}

void ForcePodAttachmentSystem::updateAttachedPods(ECS::Registry& registry) {
    auto podView =
        registry.view<ForcePodTag, ForcePodComponent, TransformComponent, NetworkIdComponent>();

    podView.each([&registry, this](ECS::Entity podEntity, const ForcePodTag&,
                             const ForcePodComponent& forcePod,
                             TransformComponent& podTransform,
                             const NetworkIdComponent& podNetId) {
        // Register pod with launch system if not already done
        if (_launchSystem && forcePod.ownerNetworkId != 0) {
            _launchSystem->setForcePodForPlayer(forcePod.ownerNetworkId, podEntity);
        }

        if (forcePod.state != ForcePodState::Attached) {
            return;
        }

        auto playerView =
            registry.view<PlayerTag, NetworkIdComponent, TransformComponent>();

        playerView.each([&forcePod, &podTransform](
                            ECS::Entity /*playerEntity*/, const PlayerTag&,
                            const NetworkIdComponent& networkId,
                            const TransformComponent& playerTransform) {
            if (networkId.networkId == forcePod.ownerNetworkId) {
                podTransform.x = playerTransform.x + forcePod.offsetX;
                podTransform.y = playerTransform.y + forcePod.offsetY;
                podTransform.rotation = playerTransform.rotation;
            }
        });
    });
}

}  // namespace rtype::games::rtype::server
