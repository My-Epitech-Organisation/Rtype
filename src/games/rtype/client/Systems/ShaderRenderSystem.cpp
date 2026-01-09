/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ShaderRenderSystem.cpp
*/

#include "ShaderRenderSystem.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

namespace rtype::games::rtype::client {

ShaderRenderSystem::ShaderRenderSystem(
    std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("ShaderRenderSystem"),
      _display(std::move(display)) {}

void ShaderRenderSystem::_applyShaderForSettings(
    const AccessibilitySettings& acc) {
    if (acc.colorMode == ColorBlindMode::None) {
        return;
    }

    std::vector<float> mat = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f};
    float contrast = 1.0f;
    float intensity = std::clamp(acc.intensity, 0.0f, 1.5f);

    switch (acc.colorMode) {
        case ColorBlindMode::Protanopia:
            mat = {0.566f, 0.433f, 0.0f,   0.558f, 0.442f,
                   0.0f,   0.0f,   0.242f, 0.758f};
            break;
        case ColorBlindMode::Deuteranopia:
            mat = {0.625f, 0.375f, 0.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.3f, 0.7f};
            break;
        case ColorBlindMode::Tritanopia:
            mat = {0.95f,  0.05f, 0.0f,   0.0f,  0.433f,
                   0.567f, 0.0f,  0.475f, 0.525f};
            break;
        case ColorBlindMode::Achromatopsia:
            mat = {0.2126f, 0.2126f, 0.2126f, 0.7152f, 0.7152f,
                   0.7152f, 0.0722f, 0.0722f, 0.0722f};
            contrast = 1.3f;
            break;
        case ColorBlindMode::HighContrast:
            mat = {0.299f, 0.299f, 0.299f, 0.587f, 0.587f,
                   0.587f, 0.114f, 0.114f, 0.114f};
            contrast = 1.6f;
            break;
        case ColorBlindMode::None:
        default:
            return;
    }

    _display->setShaderUniform("colorShader", "colorMatrix", mat);
    _display->setShaderUniform("colorShader", "contrast", contrast);
    _display->setShaderUniform("colorShader", "intensity", intensity);
}

void ShaderRenderSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    _display->clear();

    std::string shaderName = "";
    if (registry.hasSingleton<AccessibilitySettings>()) {
        const auto& acc = registry.getSingleton<AccessibilitySettings>();
        if (acc.colorMode != ColorBlindMode::None) {
            _applyShaderForSettings(acc);
            shaderName = "colorShader";
        }
    }

    _display->drawRenderTexture("scene", shaderName);
}

}  // namespace rtype::games::rtype::client
