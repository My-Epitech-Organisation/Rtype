/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BehaviorRegistry - Registry for AI behaviors
*/

#pragma once

#include <memory>
#include <unordered_map>

#include "IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class BehaviorRegistry
 * @brief Singleton registry for AI behavior strategies
 *
 * Manages all available AI behaviors. New behaviors can be registered
 * at runtime, allowing for easy extension.
 *
 * Usage:
 * @code
 * // Get or register behaviors
 * auto& registry = BehaviorRegistry::instance();
 * registry.registerBehavior<MoveLeftBehavior>();
 *
 * // Get behavior for an entity
 * auto behavior = registry.getBehavior(AIBehavior::MoveLeft);
 * if (behavior) {
 *     behavior->apply(ai, transform, velocity, dt);
 * }
 * @endcode
 */
class BehaviorRegistry {
   public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the global BehaviorRegistry
     */
    static BehaviorRegistry& instance() {
        static BehaviorRegistry registry;
        return registry;
    }

    // Delete copy/move
    BehaviorRegistry(const BehaviorRegistry&) = delete;
    BehaviorRegistry& operator=(const BehaviorRegistry&) = delete;
    BehaviorRegistry(BehaviorRegistry&&) = delete;
    BehaviorRegistry& operator=(BehaviorRegistry&&) = delete;

    /**
     * @brief Register a behavior type
     * @tparam T Behavior class deriving from IAIBehavior
     * @tparam Args Constructor argument types
     * @param args Arguments to forward to behavior constructor
     */
    template <typename T, typename... Args>
    void registerBehavior(Args&&... args) {
        static_assert(std::is_base_of_v<IAIBehavior, T>,
                      "T must derive from IAIBehavior");
        auto behavior = std::make_shared<T>(std::forward<Args>(args)...);
        _behaviors[behavior->getType()] = std::move(behavior);
    }

    /**
     * @brief Get a behavior by type
     * @param type The AIBehavior enum value
     * @return Shared pointer to the behavior, or nullptr if not registered
     */
    [[nodiscard]] std::shared_ptr<IAIBehavior> getBehavior(
        AIBehavior type) const {
        auto it = _behaviors.find(type);
        return (it != _behaviors.end()) ? it->second : nullptr;
    }

    /**
     * @brief Check if a behavior is registered
     * @param type The AIBehavior enum value
     * @return true if registered
     */
    [[nodiscard]] bool hasBehavior(AIBehavior type) const {
        return _behaviors.find(type) != _behaviors.end();
    }

    /**
     * @brief Get the number of registered behaviors
     * @return Count of registered behaviors
     */
    [[nodiscard]] std::size_t size() const noexcept {
        return _behaviors.size();
    }

    /**
     * @brief Clear all registered behaviors
     */
    void clear() { _behaviors.clear(); }

   private:
    BehaviorRegistry() = default;

    struct AIBehaviorHash {
        std::size_t operator()(AIBehavior b) const noexcept {
            return static_cast<std::size_t>(b);
        }
    };

    std::unordered_map<AIBehavior, std::shared_ptr<IAIBehavior>, AIBehaviorHash>
        _behaviors;
};

/**
 * @brief Helper to register all default behaviors
 *
 * Call this once at startup to register all built-in behaviors.
 */
void registerDefaultBehaviors();

}  // namespace rtype::games::rtype::shared
