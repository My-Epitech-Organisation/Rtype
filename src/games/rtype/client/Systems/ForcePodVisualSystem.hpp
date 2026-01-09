/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodVisualSystem - Manages Force Pod visual effects
*/

#pragma once

#include <memory>

#include <rtype/engine.hpp>

#include "../../shared/Components/ForcePodComponent.hpp"
#include "../../shared/Components/Tags.hpp"
#include "../Components/ForcePodVisualComponent.hpp"
#include "../Components/ImageComponent.hpp"

namespace rtype::games::rtype::client {

class ForcePodVisualSystem : public ::rtype::engine::ASystem {
   public:
    ForcePodVisualSystem();
    ~ForcePodVisualSystem() override = default;

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    void updateGlowEffect(ECS::Registry& registry,
                          const shared::ForcePodComponent& pod,
                          ForcePodVisual& visual, Image& image,
                          float deltaTime);

    void createTrailParticle(ECS::Registry& registry, float x, float y);
};

}  // namespace rtype::games::rtype::client
