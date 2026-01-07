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
     * @brief Interface abstraite d'affichage utilisée par le moteur.
     *
     * Fournit les opérations de création/fermeture de fenêtre, gestion
     * d'événements, rendu (sprites, textes, formes) et gestion des
     * ressources (textures, polices, sons, shaders, etc.).
     */
    class IDisplay {
    public:
        virtual ~IDisplay() = default;

        /**
         * @brief Retourne le nom de la bibliothèque d'affichage (par ex. "SFML").
         * @return Nom de la bibliothèque
         */
        [[nodiscard]] virtual std::string getLibName(void) const = 0;

        /**
         * @brief Ouvre une fenêtre d'affichage.
         * @param width Largeur de la fenêtre en pixels
         * @param height Hauteur de la fenêtre en pixels
         * @param title Titre de la fenêtre
         * @param setFullscreen true pour ouvrir en plein écran
         */
        virtual void open(unsigned int width, unsigned int height, const std::string& title, const bool setFullscreen) = 0;

        /**
         * @brief Indique si la fenêtre est actuellement ouverte.
         * @return true si ouverte
         */
        [[nodiscard]] virtual bool isOpen() const = 0;

        /**
         * @brief Ferme la fenêtre et libère les ressources associées.
         */
        virtual void close() = 0;

        /**
         * @brief Récupère l'événement suivant de la file d'événements.
         * @param event Référence où stocker l'événement récupéré
         * @return true si un événement a été récupéré
         */
        virtual bool pollEvent(Event& event) = 0;

        /**
         * @brief Efface la cible de rendu avec une couleur donnée.
         * @param color Couleur de remplissage (valeur par défaut : noir opaque)
         */
        virtual void clear(const Color& color = {0, 0, 0, 255}) = 0;

        /**
         * @brief Présente le contenu rendu à l'écran (swap buffers).
         */
        virtual void display() = 0;

        /**
         * @brief Limite la fréquence de rafraîchissement (FPS).
         * @param limit Nombre maximal d'images par seconde
         */
        virtual void setFramerateLimit(unsigned int limit) = 0;

        /**
         * @brief Active/désactive le mode plein écran.
         * @param fullscreen true pour plein écran
         */
        virtual void setFullscreen(bool) = 0;

        /**
         * @brief Indique si le mode plein écran est actif.
         * @return true si plein écran
         */
        [[nodiscard]] virtual bool isFullscreen() const = 0;

        // Rendering methods
        /**
         * @brief Dessine un sprite à l'écran.
         * @param textureName Nom de la texture chargée
         * @param position Position du sprite
         * @param rect Zone source de la texture à utiliser
         * @param scale Échelle appliquée au sprite
         * @param color Couleur modulatrice (tint)
         */
        virtual void drawSprite(const std::string& textureName, const Vector2<float>& position, const Rect<int>& rect, const Vector2<float>& scale, const Color& color) = 0;

        /**
         * @brief Dessine du texte à l'écran.
         * @param text Chaîne à afficher
         * @param fontName Nom de la police chargée
         * @param position Position du texte
         * @param size Taille du texte en points
         * @param color Couleur du texte
         */
        virtual void drawText(const std::string& text, const std::string& fontName, const Vector2<float>& position, unsigned int size, const Color& color) = 0;

        /**
         * @brief Dessine un rectangle rempli et optionnellement avec contour.
         */
        virtual void drawRectangle(const Vector2<float>& position, const Vector2<float>& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) = 0;

        /**
         * @brief Calcule les dimensions (largeur, hauteur) d'un texte donné.
         */
        virtual Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) = 0;

        /**
         * @brief Retourne la taille (en pixels) d'une texture chargée.
         */
        virtual Vector2<float> getTextureSize(const std::string& textureName) = 0;

        // View management
        /**
         * @brief Définit la vue (centre et taille) utilisée pour le rendu.
         */
        virtual void setView(const Vector2<float>& center, const Vector2<float>& size) = 0;
        [[nodiscard]] virtual Vector2<float> getViewCenter() const = 0;
        [[nodiscard]] virtual Vector2<float> getViewSize() const = 0;

        /**
         * @brief Réinitialise la vue sur la valeur par défaut (généralement la taille de la fenêtre).
         */
        virtual void resetView() = 0;

        /**
         * @brief Retourne la taille actuelle de la fenêtre en pixels.
         */
        [[nodiscard]] virtual Vector2<int> getWindowSize() const = 0;

        // Asset management (might be better elsewhere, but for now...)
        /**
         * @brief Charge une texture depuis un fichier et l'associe à un nom.
         */
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
        /**
         * @brief Définit une valeur uniforme float pour un shader donné.
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) = 0;

        /**
         * @brief Définit une matrice (vecteur de floats) comme uniforme pour un shader.
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) = 0;

        // Render to texture
        /**
         * @brief Commence le rendu vers une texture rendable identifiée par son nom.
         */
        virtual void beginRenderToTexture(const std::string& textureName) = 0;
        virtual void endRenderToTexture() = 0;
        virtual void drawRenderTexture(const std::string& textureName, const std::string& shaderName) = 0;

        // Joystick
        /**
         * @brief Indique si un joystick est connecté.
         */
        [[nodiscard]] virtual bool isJoystickConnected(unsigned int joystickId) const = 0;
        [[nodiscard]] virtual unsigned int getJoystickCount() const = 0;
    };

} // namespace rtype::display

typedef rtype::display::IDisplay* (*entryPoint)();

#endif // IDISPLAY_HPP_
