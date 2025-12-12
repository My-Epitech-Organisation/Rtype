/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BehaviorRegistry - Implementation
*/

#include "BehaviorRegistry.hpp"

#include "Chase/ChaseBehavior.hpp"
#include "MoveLeft/MoveLeftBehavior.hpp"
#include "DiveBomb/DiveBombBehavior.hpp"
#include "Patrol/PatrolBehavior.hpp"
#include "SineWave/SineWaveBehavior.hpp"
#include "Stationary/StationaryBehavior.hpp"
#include "ZigZag/ZigZagBehavior.hpp"

namespace rtype::games::rtype::shared {

void registerDefaultBehaviors() {
    auto& registry = BehaviorRegistry::instance();

    registry.registerBehavior<MoveLeftBehavior>();
    registry.registerBehavior<SineWaveBehavior>();
    registry.registerBehavior<ChaseBehavior>();
    registry.registerBehavior<PatrolBehavior>();
    registry.registerBehavior<StationaryBehavior>();
    registry.registerBehavior<ZigZagBehavior>();
    registry.registerBehavior<DiveBombBehavior>();
}

}  // namespace rtype::games::rtype::shared
