/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SFMLWrap.hpp
*/

#ifndef R_TYPE_SFMLWRAP_HPP
#define R_TYPE_SFMLWRAP_HPP

#include "include/rtype/display/IDisplay.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include "SFMLTexture.hpp"
#include "SFMLFont.hpp"
#include "SFMLMusic.hpp"
#include "SFMLSoundBuffer.hpp"
#include "SFMLSound.hpp"
#include "lib/display/ADisplay.hpp"
#include "lib/common/src/Logger/Macros.hpp"

namespace rtype::display {
    class SFMLDisplay : public ::rtype::display::ADisplay {
    public:
        SFMLDisplay();
        ~SFMLDisplay() override;

        [[nodiscard]] std::string getLibName(void) const override;

        void open(unsigned int width, unsigned int height, const std::string& title, bool setFullscreen) override;
        [[nodiscard]] bool isOpen() const override;
        void close() override;
        bool pollEvent(Event& event) override;
        void clear(const Color& color = {0, 0, 0, 255}) override;
        void display() override;
        void setFramerateLimit(unsigned int limit) override;
        void setFullscreen(bool) override;
        [[nodiscard]] bool isFullscreen() const override;

        // Rendering methods
        void drawSprite(const std::string& textureName, const Vector2<float>& position, const Rect<int>& rect, const Vector2<float>& scale, const Color& color) override;
        void drawText(const std::string& text, const std::string& fontName, const Vector2<float>& position, unsigned int size, const Color& color) override;
        void drawRectangle(const Vector2<float>& position, const Vector2<float>& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) override;

        Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) override;
        Vector2<float> getTextureSize(const std::string& textureName) override;

        // View management
        void setView(const Vector2<float>& center, const Vector2<float>& size) override;
        [[nodiscard]] Vector2<float> getViewCenter() const override;
        [[nodiscard]] Vector2<float> getViewSize() const override;
        void resetView() override;

        [[nodiscard]] Vector2<int> getWindowSize() const override;

        // Asset management (might be better elsewhere, but for now...)
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

        // Joystick
        [[nodiscard]] bool isJoystickConnected(unsigned int joystickId) const override;
        [[nodiscard]] unsigned int getJoystickCount() const override;

    private:
        Key _translateKey(sf::Keyboard::Key key);
        MouseButton _translateMouseButton(sf::Mouse::Button button);
        JoystickAxis _translateJoystickAxis(sf::Joystick::Axis axis);

        std::unique_ptr<sf::RenderWindow> _window;
        sf::RenderTarget* _renderTarget;
        sf::View _view;
        std::unordered_map<std::string, std::shared_ptr<SFMLTexture>> _textures;
        std::unordered_map<std::string, std::shared_ptr<SFMLFont>> _fonts;
        std::unordered_map<std::string, std::shared_ptr<SFMLSoundBuffer>> _soundBuffers;
        std::unordered_map<std::string, std::shared_ptr<SFMLMusic>> _musics;
        std::unordered_map<std::string, std::unique_ptr<sf::Shader>> _shaders;
        std::unordered_map<std::string, std::unique_ptr<sf::RenderTexture>> _renderTextures;
    };
}


#endif //R_TYPE_SFMLWRAP_HPP