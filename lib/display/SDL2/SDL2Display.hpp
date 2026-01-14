/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Display.hpp - SDL2 implementation of IDisplay
*/

#ifndef R_TYPE_SDL2DISPLAY_HPP
#define R_TYPE_SDL2DISPLAY_HPP

#include "include/rtype/display/IDisplay.hpp"
#include "lib/display/ADisplay.hpp"
#include "SDL2Font.hpp"
#include "SDL2Texture.hpp"
#include "SDL2SoundBuffer.hpp"
#include "SDL2Music.hpp"
#include "SDL2Sound.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace rtype::display {

class SDL2Display : public ADisplay {
public:
    SDL2Display();
    ~SDL2Display() override;

    [[nodiscard]] std::string getLibName() const override;

    void open(unsigned int width, unsigned int height, const std::string& title, bool setFullscreen) override;
    [[nodiscard]] bool isOpen() const override;
    void close() override;
    bool pollEvent(Event& event) override;
    void clear(const Color& color = {0, 0, 0, 255}) override;
    void display() override;
    void setFramerateLimit(unsigned int limit) override;
    void setFullscreen(bool fullscreen) override;
    [[nodiscard]] bool isFullscreen() const override;

    // Rendering methods
    void drawSprite(const std::string& textureName, const Vector2<float>& position,
                   const Rect<int>& rect, const Vector2<float>& scale, const Color& color, float rotation = 0.0f) override;
    void drawText(const std::string& text, const std::string& fontName,
                 const Vector2<float>& position, unsigned int size, const Color& color) override;
    void drawRectangle(const Vector2<float>& position, const Vector2<float>& size,
                      const Color& fillColor, const Color& outlineColor, float outlineThickness) override;

    Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) override;
    Vector2<float> getTextureSize(const std::string& textureName) override;

    // View management
    void setView(const Vector2<float>& center, const Vector2<float>& size) override;
    [[nodiscard]] Vector2<float> getViewCenter() const override;
    [[nodiscard]] Vector2<float> getViewSize() const override;
    void resetView() override;
    [[nodiscard]] Vector2<float> mapPixelToCoords(const Vector2<int>& pixelPos) const override;

    [[nodiscard]] Vector2<int> getWindowSize() const override;

    // Asset management
    void loadTexture(const std::string& name, const std::string& path) override;
    void loadFont(const std::string& name, const std::string& path) override;
    void loadSoundBuffer(const std::string& name, const std::string& path) override;
    void loadMusic(const std::string& name, const std::string& path) override;
    [[nodiscard]] std::shared_ptr<ITexture> getTexture(const std::string& name) override;
    [[nodiscard]] std::shared_ptr<IFont> getFont(const std::string& name) override;
    [[nodiscard]] std::shared_ptr<ISoundBuffer> getSoundBuffer(const std::string& name) override;
    [[nodiscard]] std::shared_ptr<IMusic> getMusic(const std::string& name) override;
    [[nodiscard]] std::shared_ptr<ISound> createSound(std::shared_ptr<ISoundBuffer> buffer) override;
    void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) override;

    // Shader uniforms
    void setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) override;
    void setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) override;

    // Render to texture
    void beginRenderToTexture(const std::string& textureName) override;
    void endRenderToTexture() override;
    void drawRenderTexture(const std::string& textureName, const std::string& shaderName) override;

    // Clipboard
    void setClipboardText(const std::string& text) override;
    [[nodiscard]] std::string getClipboardText() const override;

    // Joystick
    [[nodiscard]] bool isJoystickConnected(unsigned int joystickId) const override;
    [[nodiscard]] unsigned int getJoystickCount() const override;

private:
    Key translateKey(SDL_Keycode key);
    MouseButton translateMouseButton(Uint8 button);

    // View helpers
    Vector2<int> worldToScreenPosition(const Vector2<float>& pos) const;
    Vector2<int> worldToScreenSize(const Vector2<float>& size) const;

    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    bool _isOpen = false;
    unsigned int _framerateLimit = 60;
    Uint32 _lastFrameTime = 0;

    Vector2<float> _viewCenter;
    Vector2<float> _viewSize;

    // Asset storage
    std::unordered_map<std::string, std::shared_ptr<SDL2Texture>> _textures;
    std::unordered_map<std::string, std::shared_ptr<SDL2Font>> _fonts;
    std::unordered_map<std::string, std::shared_ptr<SDL2SoundBuffer>> _soundBuffers;
    std::unordered_map<std::string, std::shared_ptr<SDL2Music>> _musicFiles;

    bool _debugVisuals = false;
    int _debugGridSpacing = 64;

    // Render textures
    std::unordered_map<std::string, SDL_Texture*> _renderTextures;
    std::string _activeRenderTarget;
};

}  // namespace rtype::display

#endif  // R_TYPE_SDL2DISPLAY_HPP
