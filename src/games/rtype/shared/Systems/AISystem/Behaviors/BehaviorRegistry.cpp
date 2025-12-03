/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BehaviorRegistry - Implementation
*/

#include "BehaviorRegistry.hpp"

#include "ChaseBehavior.hpp"
#include "MoveLeftBehavior.hpp"
#include "PatrolBehavior.hpp"
#include "SineWaveBehavior.hpp"
#include "StationaryBehavior.hpp"

namespace rtype::games::rtype::shared {

void registerDefaultBehaviors() {
    auto& registry = BehaviorRegistry::instance();

    registry.registerBehavior<MoveLeftBehavior>();
    registry.registerBehavior<SineWaveBehavior>();
    registry.registerBehavior<ChaseBehavior>();
    registry.registerBehavior<PatrolBehavior>();
    registry.registerBehavior<StationaryBehavior>();
}

}  // namespace rtype::games::rtype::shared
