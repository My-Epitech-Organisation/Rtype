/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ShaderRenderSystem.hpp
*/

#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include "Accessibility.hpp"
#include "ASystem.hpp"
#include "ECS.hpp"

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
     * @param window The main window to draw to
     * @param sceneTexture The texture containing the rendered scene
     * @param colorShader The shader used for color blind mode effects
     */
    ShaderRenderSystem(std::shared_ptr<sf::RenderWindow> window,
                       std::shared_ptr<sf::RenderTexture> sceneTexture,
                       std::shared_ptr<sf::Shader> colorShader);

    ~ShaderRenderSystem() override = default;

    /**
     * @brief Apply shader effects and draw the final scene to the window
     *
     * @param registry ECS registry containing AccessibilitySettings
     * @param deltaTime Delta time (unused for this system)
     */
    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    std::shared_ptr<sf::RenderWindow> _window;
    std::shared_ptr<sf::RenderTexture> _sceneTexture;
    std::shared_ptr<sf::Shader> _colorShader;

    /**
     * @brief Get the shader to apply based on accessibility settings
     *
     * @param settings Accessibility settings from registry
     * @return sf::Shader* Shader to use, or nullptr if no shader should be
     * applied
     */
    sf::Shader* _getShaderForSettings(const AccessibilitySettings& settings);
};

}  // namespace rtype::games::rtype::client
