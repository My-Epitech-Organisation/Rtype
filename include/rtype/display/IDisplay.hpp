/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** IDisplay.hpp
*/

#ifndef IDISPLAY_HPP_
#define IDISPLAY_HPP_

#include <string>
#include <memory>
#include <vector>
#include "DisplayTypes.hpp"

namespace rtype::display {

    enum class EventType {
        Closed,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        JoystickButtonPressed,
        JoystickButtonReleased,
        JoystickMoved,
        FocusLost,
        FocusGained,
        TextEntered,
        Unknown
    };

    class ITexture {
    public:
        virtual ~ITexture() = default;

        virtual bool loadFromFile(const std::string& path) = 0;
        virtual void setRepeated(bool repeated) = 0;
        virtual void setSmooth(bool smooth) = 0;
        virtual Vector2u getSize() const = 0;
    };

    class IFont {
    public:
        virtual ~IFont() = default;

        virtual bool openFromFile(const std::string& path) = 0;
    };

    class ISoundBuffer {
    public:
        virtual ~ISoundBuffer() = default;

        virtual bool loadFromFile(const std::string& path) = 0;
    };

    class ISound {
    public:
        virtual ~ISound() = default;

        enum class Status {
            Stopped,
            Paused,
            Playing
        };
        virtual void setVolume(float volume) = 0;
        virtual void play() = 0;
        virtual Status getStatus() const = 0;
    };
    class IMusic {
    public:
        virtual ~IMusic() = default;

        virtual bool openFromFile(const std::string& path) = 0;
        virtual void setLooping(bool loop) = 0;
        virtual void setVolume(float volume) = 0;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
    };

    enum class Key {
        Unknown = -1,
        A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        Escape, LControl, LShift, LAlt, LSystem, RControl, RShift, RAlt, RSystem,
        Menu, LBracket, RBracket, SemiColon, Comma, Period, Quote, Slash, BackSlash,
        Tilde, Equal, Dash, Space, Return, BackSpace, Tab, PageUp, PageDown, End, Home,
        Insert, Delete, Add, Subtract, Multiply, Divide, Left, Right, Up, Down,
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
        Pause
    };

    enum class MouseButton {
        Left,
        Right,
        Middle,
        XButton1,
        XButton2,
        ButtonCount
    };

    enum class JoystickAxis {
        X, Y, Z, R, U, V, PovX, PovY
    };

    struct Event {
        EventType type;
        union {
            struct {
                Key code;
                bool alt;
                bool control;
                bool shift;
                bool system;
            } key;
            struct {
                MouseButton button;
                int x;
                int y;
            } mouseButton;
            struct {
                int x;
                int y;
            } mouseMove;
            struct {
                unsigned int joystickId;
                unsigned int button;
            } joystickButton;
            struct {
                unsigned int joystickId;
                JoystickAxis axis;
                float position;
            } joystickMove;
            struct {
                uint32_t unicode;
            } text;
        };
    };

    class IDisplay {
    public:
        virtual ~IDisplay() = default;

        virtual void open(unsigned int width, unsigned int height, const std::string& title) = 0;
        [[nodiscard]] virtual bool isOpen() const = 0;
        virtual void close() = 0;
        virtual bool pollEvent(Event& event) = 0;
        virtual void clear(const Color& color = {0, 0, 0, 255}) = 0;
        virtual void display() = 0;
        virtual void setFramerateLimit(unsigned int limit) = 0;

        // Rendering methods
        virtual void drawSprite(const std::string& textureName, const Vector2<float>& position, const Rect<int>& rect, const Vector2<float>& scale, const Color& color) = 0;
        virtual void drawText(const std::string& text, const std::string& fontName, const Vector2<float>& position, unsigned int size, const Color& color) = 0;
        virtual void drawRectangle(const Vector2<float>& position, const Vector2<float>& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) = 0;

        virtual Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) = 0;
        virtual Vector2<float> getTextureSize(const std::string& textureName) = 0;

        // View management
        virtual void setView(const Vector2<float>& center, const Vector2<float>& size) = 0;
        [[nodiscard]] virtual Vector2<float> getViewCenter() const = 0;
        [[nodiscard]] virtual Vector2<float> getViewSize() const = 0;
        virtual void resetView() = 0;

        [[nodiscard]] virtual Vector2<int> getWindowSize() const = 0;

        // Asset management (might be better elsewhere, but for now...)
        virtual void loadTexture(const std::string& name, const std::string& path) = 0;
        virtual void loadFont(const std::string& name, const std::string& path) = 0;
        virtual void loadSoundBuffer(const std::string& name, const std::string& path) = 0;
        virtual void loadMusic(const std::string& name, const std::string& path) = 0;
        virtual std::shared_ptr<ITexture> getTexture(const std::string& name) = 0;
        virtual std::shared_ptr<IFont> getFont(const std::string& name) = 0;
        virtual std::shared_ptr<ISoundBuffer> getSoundBuffer(const std::string& name) = 0;
        virtual std::shared_ptr<IMusic> getMusic(const std::string& name) = 0;
        virtual std::shared_ptr<ISound> createSound(std::shared_ptr<ISoundBuffer> buffer) = 0;
        virtual void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) = 0;

        // Shader uniforms
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) = 0;
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) = 0;

        // Render to texture
        virtual void beginRenderToTexture(const std::string& textureName) = 0;
        virtual void endRenderToTexture() = 0;
        virtual void drawRenderTexture(const std::string& textureName, const std::string& shaderName) = 0;

        // Joystick
        [[nodiscard]] virtual bool isJoystickConnected(unsigned int joystickId) const = 0;
        [[nodiscard]] virtual unsigned int getJoystickCount() const = 0;
    };

} // namespace rtype::display

typedef rtype::display::IDisplay* (*entryPoint)();

#endif // IDISPLAY_HPP_
