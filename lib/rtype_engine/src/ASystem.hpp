/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ASystem - Abstract base class for all game systems
*/

#pragma once

#include <stdexcept>
#include <string>

#include "ISystem.hpp"

namespace rtype::engine {

/**
 * @brief Exception thrown when a required component is missing
 */
class MissingComponentError : public std::runtime_error {
   public:
    MissingComponentError(const std::string& systemName,
                          const std::string& componentName,
                          uint32_t entityId)
        : std::runtime_error("System '" + systemName +
                             "' requires component '" + componentName +
                             "' on entity " + std::to_string(entityId)) {}
};

/**
 * @class ASystem
 * @brief Abstract base class implementing common ISystem functionality
 *
 * This class provides default implementations for common system operations.
 * Concrete systems should inherit from this class rather than ISystem directly.
 *
 * Example usage:
 * @code
 * class MovementSystem : public ASystem {
 * public:
 *     MovementSystem() : ASystem("MovementSystem") {}
 *     void update(ECS::Registry& registry, float deltaTime) override {
 *         // Implementation
 *     }
 * };
 * @endcode
 */
class ASystem : public ISystem {
   public:
    ~ASystem() override = default;

    // Delete copy/move to ensure systems are not accidentally copied
    ASystem(const ASystem&) = delete;
    ASystem& operator=(const ASystem&) = delete;
    ASystem(ASystem&&) = delete;
    ASystem& operator=(ASystem&&) = delete;

    /**
     * @brief Get the system name for debugging/logging
     * @return System name string
     */
    [[nodiscard]] const std::string getName() const noexcept override {
        return _name;
    }

    /**
     * @brief Check if system is enabled
     * @return true if system should be updated
     */
    [[nodiscard]] bool isEnabled() const noexcept override { return _enabled; }

    /**
     * @brief Enable or disable the system
     * @param enabled New enabled state
     */
    void setEnabled(bool enabled) noexcept override { _enabled = enabled; }

   protected:
    /**
     * @brief Construct a new ASystem
     * @param name The name of the system for debugging/logging
     */
    explicit ASystem(std::string name) : _name(std::move(name)) {}

    /**
     * @brief Require a component on an entity (throws if missing)
     * @tparam Component The component type to check
     * @param registry The ECS registry
     * @param entity The entity to check
     * @param componentName Human-readable name for error messages
     * @throws MissingComponentError if component is not present
     */
    template <typename Component>
    void requireComponent(ECS::Registry& registry, ECS::Entity entity,
                          const std::string& componentName) const {
        if (!registry.hasComponent<Component>(entity)) {
            throw MissingComponentError(_name, componentName, entity.id);
        }
    }

    /**
     * @brief Check if entity has a component (non-throwing)
     * @tparam Component The component type to check
     * @param registry The ECS registry
     * @param entity The entity to check
     * @return true if component exists
     */
    template <typename Component>
    [[nodiscard]] bool hasComponent(ECS::Registry& registry,
                                    ECS::Entity entity) const {
        return registry.hasComponent<Component>(entity);
    }

   private:
    std::string _name;     ///< System name for debugging
    bool _enabled = true;  ///< Whether system is active
};

}  // namespace rtype::engine
