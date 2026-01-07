/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodAttachmentSystem - Handles Force Pod positioning and attachment
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../../shared/Components/ForcePodComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::server {

class ForcePodAttachmentSystem : public ::rtype::engine::ASystem {
   public:
    ForcePodAttachmentSystem();

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    void updateAttachedPods(ECS::Registry& registry);
};

}  // namespace rtype::games::rtype::server
