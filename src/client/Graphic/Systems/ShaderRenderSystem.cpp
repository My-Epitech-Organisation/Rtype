/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ShaderRenderSystem.cpp
*/

#include "ShaderRenderSystem.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <array>

namespace rtype::games::rtype::client {

ShaderRenderSystem::ShaderRenderSystem(
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<sf::RenderTexture> sceneTexture,
    std::shared_ptr<sf::Shader> colorShader)
    : ::rtype::engine::ASystem("ShaderRenderSystem"),
      _window(std::move(window)),
      _sceneTexture(std::move(sceneTexture)),
      _colorShader(std::move(colorShader)) {}

sf::Shader* ShaderRenderSystem::_getShaderForSettings(
    const AccessibilitySettings& acc) {
    if (!_colorShader || acc.colorMode == ColorBlindMode::None) {
        return nullptr;
    }

    auto makeMat3 = [](std::initializer_list<float> values) {
        std::array<float, 9> data{};
        std::copy(values.begin(), values.end(), data.begin());
        return sf::Glsl::Mat3(data.data());
    };

    auto mat = makeMat3({1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f});
    float contrast = 1.0f;
    float intensity = std::clamp(acc.intensity, 0.0f, 1.5f);

    switch (acc.colorMode) {
        case ColorBlindMode::Protanopia:
            mat = makeMat3({0.566f, 0.433f, 0.0f, 0.558f, 0.442f, 0.0f, 0.0f,
                            0.242f, 0.758f});
            break;
        case ColorBlindMode::Deuteranopia:
            mat = makeMat3(
                {0.625f, 0.375f, 0.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.3f, 0.7f});
            break;
        case ColorBlindMode::Tritanopia:
            mat = makeMat3({0.95f, 0.05f, 0.0f, 0.0f, 0.433f, 0.567f, 0.0f,
                            0.475f, 0.525f});
            break;
        case ColorBlindMode::Achromatopsia:
            mat = makeMat3({0.2126f, 0.2126f, 0.2126f, 0.7152f, 0.7152f,
                            0.7152f, 0.0722f, 0.0722f, 0.0722f});
            contrast = 1.3f;
            break;
        case ColorBlindMode::HighContrast:
            mat = makeMat3({0.299f, 0.299f, 0.299f, 0.587f, 0.587f, 0.587f,
                            0.114f, 0.114f, 0.114f});
            contrast = 1.6f;
            break;
        case ColorBlindMode::None:
        default:
            return nullptr;
    }

    _colorShader->setUniform("texture", sf::Shader::CurrentTexture);
    _colorShader->setUniform("colorMatrix", mat);
    _colorShader->setUniform("contrast", contrast);
    _colorShader->setUniform("intensity", intensity);

    return _colorShader.get();
}

void ShaderRenderSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    _window->clear();

    sf::Sprite composed(_sceneTexture->getTexture());
    sf::Shader* shader = nullptr;

    if (registry.hasSingleton<AccessibilitySettings>()) {
        const auto& acc = registry.getSingleton<AccessibilitySettings>();
        shader = _getShaderForSettings(acc);
    }

    _window->draw(composed, shader);
}

}  // namespace rtype::games::rtype::client
