#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOSSANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOSSANIMATIONSYSTEM_HPP_

#include "ASystem.hpp"

namespace rtype::games::rtype::client {

class BossAnimationSystem : public ::rtype::engine::ASystem {
   public:
    BossAnimationSystem();
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif
