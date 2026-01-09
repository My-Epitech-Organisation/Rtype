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
#include "IFont.hpp"
#include "IMusic.hpp"
#include "ISound.hpp"
#include "ISoundBuffer.hpp"
#include "ITexture.hpp"

namespace rtype::display {

    /**
     * @file IDisplay.hpp
     * @brief Interfaces and types for the display module (textures, fonts, sounds, events, etc.)
     *
     * This file defines:
     * - EventType: possible event types (close, keyboard, mouse, joystick, text, etc.)
     * - ITexture/IFont/ISoundBuffer/ISound/IMusic: abstract interfaces for multimedia resources
     * - Key/MouseButton/JoystickAxis: enumerations for user inputs
     * - Event: structure describing an event polled from the window
     * - IDisplay: main rendering and window management interface
     *
     * All interfaces are intended to be implemented by a display library
     * (for example SFML) behind an adapter conforming to IDisplay.
     *
     * The comments document the intended behavior of each method for implementers.
     */

    /**
     * @brief Abstract display interface used by the engine.
     *
     * Provides operations for window creation/closure, event management,
     * rendering (sprites, texts, shapes), and resource management
     * (textures, fonts, sounds, shaders, etc.).
     */
    class IDisplay {
    public:
        virtual ~IDisplay() = default;

        /**
         * @brief Returns the name of the display library (e.g., "SFML").
         * @return Name of the library
         */
        [[nodiscard]] virtual std::string getLibName(void) const = 0;

        /**
         * @brief Opens a display window.
         * @param width Width of the window in pixels
         * @param height Height of the window in pixels
         * @param title Title of the window
         * @param setFullscreen true to open in fullscreen mode
         */
        virtual void open(unsigned int width, unsigned int height, const std::string& title, const bool setFullscreen) = 0;

        /**
         * @brief Indicates if the window is currently open.
         * @return true if open
         */
        [[nodiscard]] virtual bool isOpen() const = 0;

        /**
         * @brief Closes the window and releases associated resources.
         */
        virtual void close() = 0;

        /**
         * @brief Retrieves the next event from the event queue.
         * @param event Reference where the retrieved event will be stored
         * @return true if an event was retrieved
         */
        virtual bool pollEvent(Event& event) = 0;

        /**
         * @brief Clears the render target with a given color.
         * @param color Fill color (default value: opaque black)
         */
        virtual void clear(const Color& color = {0, 0, 0, 255}) = 0;

        /**
         * @brief Presents the rendered content to the screen (swap buffers).
         */
        virtual void display() = 0;

        /**
         * @brief Limits the refresh rate (FPS).
         * @param limit Maximum number of frames per second
         */
        virtual void setFramerateLimit(unsigned int limit) = 0;

        /**
         * @brief Enables/disables fullscreen mode.
         * @param fullscreen true for fullscreen
         */
        virtual void setFullscreen(bool fullscreen) = 0;

        /**
         * @brief Indicates if fullscreen mode is active.
         * @return true if fullscreen
         */
        [[nodiscard]] virtual bool isFullscreen() const = 0;

        // Rendering methods
        /**
         * @brief Draws a sprite to the screen.
         * @param textureName Name of the loaded texture
         * @param position Sprite position
         * @param rect Source area of the texture to use
         * @param scale Scale applied to the sprite
         * @param color Modulation color (tint)
         */
        virtual void drawSprite(const std::string& textureName, const Vector2<float>& position, const Rect<int>& rect, const Vector2<float>& scale, const Color& color) = 0;

        /**
         * @brief Draws text to the screen.
         * @param text String to display
         * @param fontName Name of the loaded font
         * @param position Text position
         * @param size Font size in points
         * @param color Text color
         */
        virtual void drawText(const std::string& text, const std::string& fontName, const Vector2<float>& position, unsigned int size, const Color& color) = 0;

        /**
         * @brief Draws a filled rectangle, optionally with an outline.
         * @param position Rectangle position
         * @param size Rectangle size
         * @param fillColor Fill color
         * @param outlineColor Outline color
         * @param outlineThickness Thickness of the outline
         */
        virtual void drawRectangle(const Vector2<float>& position, const Vector2<float>& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) = 0;

        /**
         * @brief Calculates the dimensions (width, height) of a given text.
         * @param text Text string
         * @param fontName Name of the loaded font
         * @param size Font size in points
         * @return Text dimensions in pixels
         */
        virtual Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) = 0;

        /**
         * @brief Returns the size (in pixels) of a loaded texture.
         * @param textureName Name of the texture
         * @return Texture size
         */
        virtual Vector2<float> getTextureSize(const std::string& textureName) = 0;

        // View management
        /**
         * @brief Sets the view (center and size) used for rendering.
         * @param center Center of the view
         * @param size Size of the view
         */
        virtual void setView(const Vector2<float>& center, const Vector2<float>& size) = 0;

        /**
         * @brief Returns the current center of the view.
         * @return View center
         */
        [[nodiscard]] virtual Vector2<float> getViewCenter() const = 0;

        /**
         * @brief Returns the current size of the view.
         * @return View size
         */
        [[nodiscard]] virtual Vector2<float> getViewSize() const = 0;

        /**
         * @brief Resets the view to the default value (usually the window size).
         */
        virtual void resetView() = 0;

        /**
         * @brief Maps pixel coordinates to world coordinates based on the current view.
         * @param pixelPos Pixel position in the window
         * @return World coordinates
         */
        [[nodiscard]] virtual Vector2<float> mapPixelToCoords(const Vector2<int>& pixelPos) const = 0;

        /**
         * @brief Returns the current window size in pixels.
         * @return Window size
         */
        [[nodiscard]] virtual Vector2<int> getWindowSize() const = 0;

        // Asset management
        /**
        * @brief Loads a texture from a file and associates it with a name.
        * @param name Name of the texture
        * @param path Path to the texture file
        */
        virtual void loadTexture(const std::string& name, const std::string& path) = 0;

        /**
        * @brief Loads a font from a file and associates it with a name.
        * @param name Name of the font
        * @param path Path to the font file
        */
        virtual void loadFont(const std::string& name, const std::string& path) = 0;

        /**
        * @brief Loads a sound buffer from a file and associates it with a name.
        * @param name Name of the sound buffer (sfx)
        * @param path Path to the sound file
        */
        virtual void loadSoundBuffer(const std::string& name, const std::string& path) = 0;

        /**
        * @brief Loads music from a file and associates it with a name.
        * @param name Name of the music
        * @param path Path to the music file
        */
        virtual void loadMusic(const std::string& name, const std::string& path) = 0;

        /**
        * @brief Retrieves a loaded resource by its name.
        * @param name Name of the previously loaded resource
        * @return A shared_ptr to the resource
        */
        virtual std::shared_ptr<ITexture> getTexture(const std::string& name) = 0;

        /**
         * @brief Retrieves a loaded resource by its name.
         * @param name Name of the previously loaded resource
         * @return A shared_ptr to the resource
         */
        virtual std::shared_ptr<IFont> getFont(const std::string& name) = 0;

        /**
         * @brief Retrieves a loaded resource by its name.
         * @param name Name of the previously loaded resource
         * @return A shared_ptr to the resource
         */
        virtual std::shared_ptr<ISoundBuffer> getSoundBuffer(const std::string& name) = 0;

        /**
         * @brief Retrieves a loaded resource by its name.
         * @param name Name of the previously loaded resource
         * @return A shared_ptr to the resource
         */
        virtual std::shared_ptr<IMusic> getMusic(const std::string& name) = 0;

        /**
         * @brief Creates a sound object from a sound buffer.
         * @param buffer shared_ptr to the sound buffer
         * @return shared_ptr to the created sound object
         */
        virtual std::shared_ptr<ISound> createSound(std::shared_ptr<ISoundBuffer> buffer) = 0;

        /**
         * @brief Loads a shader from vertex and fragment files, and associates it with a name.
         * @param name Name of the shader
         * @param vertexPath Path to the vertex shader file
         * @param fragmentPath Path to the fragment shader file
         */
        virtual void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) = 0;

        // Shader uniforms
        /**
         * @brief Sets a float uniform value for a given shader.
         * @param shaderName Name of the shader
         * @param uniformName Name of the uniform in the shader
         * @param value Float value to assign
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) = 0;

        /**
         * @brief Sets a matrix (vector of floats) as a uniform for a shader.
         * @param shaderName Name of the shader
         * @param uniformName Name of the uniform in the shader
         * @param matrix Vector of floats representing the matrix
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) = 0;

        // Render to texture
        /**
         * @brief Starts rendering to a renderable texture identified by its name.
         * @param textureName Name of the renderable texture
         */
        virtual void beginRenderToTexture(const std::string& textureName) = 0;
        virtual void endRenderToTexture() = 0;
        virtual void drawRenderTexture(const std::string& textureName, const std::string& shaderName) = 0;

        // Joystick
        /**
         * @brief Indicates if a joystick is connected.
         * @param joystickId ID of the joystick
         * @return true if connected
         */
        [[nodiscard]] virtual bool isJoystickConnected(unsigned int joystickId) const = 0;

        /**
         * @brief Returns the number of connected joysticks.
         * @return Number of joysticks
         */
        [[nodiscard]] virtual unsigned int getJoystickCount() const = 0;
    };

} // namespace rtype::display

typedef rtype::display::IDisplay* (*entryPoint)();

#endif // IDISPLAY_HPP_
