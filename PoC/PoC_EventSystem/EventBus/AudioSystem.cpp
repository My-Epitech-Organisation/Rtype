/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AudioSystem implementation
*/

#include "AudioSystem.hpp"
#include "Components.hpp"
#include <iostream>

namespace PoC {

    AudioSystem::AudioSystem(EventBus& eventBus, ECS::Registry& registry)
        : _eventBus(eventBus), _registry(registry), _collisionCallbackId(0) {
        std::cout << "[AudioSystem] Initialized" << std::endl;
    }

    AudioSystem::~AudioSystem() {
        shutdown();
    }

    void AudioSystem::initialize() {
        std::cout << "[AudioSystem] Subscribing to CollisionEvent" << std::endl;
        
        // ✅ SUBSCRIBE TO EVENT - Loose coupling!
        // AudioSystem doesn't need to know about PhysicsSystem
        _collisionCallbackId = _eventBus.subscribe<CollisionEvent>(
            [this](const CollisionEvent& event) {
                this->onCollision(event);
            }
        );

        std::cout << "[AudioSystem] Ready to handle collision sounds" << std::endl;
    }

    void AudioSystem::shutdown() {
        if (_collisionCallbackId != 0) {
            std::cout << "[AudioSystem] Unsubscribing from events" << std::endl;
            _eventBus.unsubscribe<CollisionEvent>(_collisionCallbackId);
            _collisionCallbackId = 0;
        }
    }

    void AudioSystem::onCollision(const CollisionEvent& event) {
        std::cout << "[AudioSystem] Received CollisionEvent for entities " 
                  << event.entityA.index() << " and " << event.entityB.index() << std::endl;
        
        // Play collision sound at collision position
        playSound("collision.wav", event.posX, event.posY);

        // Mark entities as having played audio
        if (_registry.isAlive(event.entityA)) {
            _registry.emplaceComponent<AudioPlayed>(event.entityA, _nextSoundId);
        }
        if (_registry.isAlive(event.entityB)) {
            _registry.emplaceComponent<AudioPlayed>(event.entityB, _nextSoundId);
        }
        
        _nextSoundId++;
    }

    void AudioSystem::playSound(const std::string& soundName, float x, float y) {
        // Simulate playing sound
        std::cout << "[AudioSystem] 🔊 Playing '" << soundName 
                  << "' at position (" << x << ", " << y << ")" << std::endl;
        std::cout << "[AudioSystem] Sound ID: " << _nextSoundId << std::endl;
    }

} // namespace PoC
