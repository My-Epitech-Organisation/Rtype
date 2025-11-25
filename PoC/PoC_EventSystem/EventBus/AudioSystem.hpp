/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AudioSystem for Event Bus PoC
*/

#ifndef AUDIOSYSTEM_HPP
    #define AUDIOSYSTEM_HPP

    #include "ECS.hpp"
    #include "EventBus.hpp"
    #include "Events.hpp"

namespace PoC {

    /**
     * @brief Audio system that responds to collision events
     * 
     * This system demonstrates DECOUPLING:
     * - Subscribes to CollisionEvent without knowing about PhysicsSystem
     * - Can be added/removed without modifying other systems
     * - PhysicsSystem doesn't know AudioSystem exists
     * 
     * This is the Observer Pattern in action!
     */
    class AudioSystem {
    public:
        AudioSystem(EventBus& eventBus, ECS::Registry& registry);
        ~AudioSystem();

        /**
         * @brief Initialize audio system
         */
        void initialize();

        /**
         * @brief Cleanup audio resources
         */
        void shutdown();

    private:
        EventBus& _eventBus;
        ECS::Registry& _registry;
        EventBus::CallbackId _collisionCallbackId;
        int _nextSoundId = 1000;

        /**
         * @brief Handles collision events - plays collision sound
         */
        void onCollision(const CollisionEvent& event);

        /**
         * @brief Simulates playing a sound
         */
        void playSound(const std::string& soundName, float x, float y);
    };

} // namespace PoC

#endif // AUDIOSYSTEM_HPP
