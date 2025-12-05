/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ISystem - Base interface for all game systems
*/

#pragma once

#include <string>

#include "ecs/ECS.hpp"

namespace rtype::engine {

/**
 * @class ISystem
 * @brief Pure interface for all game systems
 *
 * Systems are responsible for updating entities based on their components.
 * Each system should have a single responsibility (Single Responsibility
 * Principle).
 *
 * Systems can be shared between client and server, or specific to one side.
 *
 * @note Concrete systems should inherit from ASystem rather than this
 * interface directly.
 */
class ISystem {
   public:
    virtual ~ISystem() = default;

    // Delete copy/move to ensure systems are not accidentally copied
    ISystem(const ISystem&) = delete;
    ISystem& operator=(const ISystem&) = delete;
    ISystem(ISystem&&) = delete;
    ISystem& operator=(ISystem&&) = delete;

    /**
     * @brief Update the system
     * @param registry The ECS registry containing entities
     * @param deltaTime Time elapsed since last update in seconds
     */
    virtual void update(ECS::Registry& registry, float deltaTime) = 0;

    /**
     * @brief Get the system name for debugging/logging
     * @return System name string
     */
    [[nodiscard]] virtual const std::string getName() const noexcept = 0;

    /**
     * @brief Check if system is enabled
     * @return true if system should be updated
     */
    [[nodiscard]] virtual bool isEnabled() const noexcept = 0;

    /**
     * @brief Enable or disable the system
     * @param enabled New enabled state
     */
    virtual void setEnabled(bool enabled) noexcept = 0;

   protected:
    ISystem() = default;
};

}  // namespace rtype::engine
