/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ShaderRenderSystem.hpp
*/

#pragma once

#include <memory>

#include "ASystem.hpp"
#include "Accessibility.hpp"
#include "ECS.hpp"
#include "rtype/display/IDisplay.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System responsible for applying post-processing shaders to the
 * rendered scene and drawing the final result to the window.
 */
class ShaderRenderSystem : public ::rtype::engine::ASystem {
   public:
    /**
     * @brief Construct a new Shader Render System
     *
     * @param display The display interface
     */
    explicit ShaderRenderSystem(
        std::shared_ptr<::rtype::display::IDisplay> display);

    ~ShaderRenderSystem() override = default;

    /**
     * @brief Apply shader effects and draw the final scene to the window
     *
     * @param registry ECS registry containing AccessibilitySettings
     * @param deltaTime Delta time (unused for this system)
     */
    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    std::shared_ptr<::rtype::display::IDisplay> _display;

    /**
     * @brief Apply shader settings to the display
     *
     * @param settings Accessibility settings from registry
     */
    void _applyShaderForSettings(const AccessibilitySettings& settings);
};

}  // namespace rtype::games::rtype::client
