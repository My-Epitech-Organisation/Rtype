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
         * @param position Position du rectangle
         * @param size Taille du rectangle
         * @param fillColor Couleur de remplissage
         * @param outlineColor Couleur du contour
         * @param outlineThickness Épaisseur du contour
         */
        virtual void drawRectangle(const Vector2<float>& position, const Vector2<float>& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) = 0;

        /**
         * @brief Calcule les dimensions (largeur, hauteur) d'un texte donné.
         * @param text Chaîne de texte
         * @param fontName Nom de la police chargée
         * @param size Taille du texte en points
         * @return Dimensions du texte en pixels
         */
        virtual Vector2<float> getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) = 0;

        /**
         * @brief Retourne la taille (en pixels) d'une texture chargée.
         * @param textureName Nom de la texture
         * @return Taille de la texture
         */
        virtual Vector2<float> getTextureSize(const std::string& textureName) = 0;

        // View management
        /**
         * @brief Définit la vue (centre et taille) utilisée pour le rendu.
         * @param center Centre de la vue
         * @param size Taille de la vue
         */
        virtual void setView(const Vector2<float>& center, const Vector2<float>& size) = 0;
        /**
         * @brief Retourne le centre actuel de la vue.
         * @return Centre de la vue
         */
        [[nodiscard]] virtual Vector2<float> getViewCenter() const = 0;
        /**
         * @brief Retourne la taille actuelle de la vue.
         * @return Taille de la vue
         */
        [[nodiscard]] virtual Vector2<float> getViewSize() const = 0;

        /**
         * @brief Réinitialise la vue sur la valeur par défaut (généralement la taille de la fenêtre).
         */
        virtual void resetView() = 0;

        /**
         * @brief Retourne la taille actuelle de la fenêtre en pixels.
         * @return Taille de la fenêtre
         */
        [[nodiscard]] virtual Vector2<int> getWindowSize() const = 0;

        // Asset management (might be better elsewhere, but for now...)
        /**
        * @brief Charge une texture depuis un fichier et l'associe à un nom.
        * @param name nom de la texture
        * @param path chemin vers le fichier de texture
        */
        virtual void loadTexture(const std::string& name, const std::string& path) = 0;
        /**
        * @brief Charge une police depuis un fichier et l'associe à un nom.
        * @param name nom de la police
        * @param path chemin vers le fichier de police
        */
        virtual void loadFont(const std::string& name, const std::string& path) = 0;
        /**
        * @brief Charge un sfx depuis un fichier et l'associe à un nom.
        * @param name nom du sfx
        * @param path chemin vers le fichier de son
        */
        virtual void loadSoundBuffer(const std::string& name, const std::string& path) = 0;
        /**
        * @brief Charge une musique depuis un fichier et l'associe à un nom.
        * @param name nom de la musique
        * @param path chemin vers le fichier de musique
        */
        virtual void loadMusic(const std::string& name, const std::string& path) = 0;
        /**
        * @brief Recupère une ressource chargée par son nom.
        * @param name nom de la ressource précédemment chargée
        * @return un shared_ptr vers la ressource
        */
        virtual std::shared_ptr<ITexture> getTexture(const std::string& name) = 0;

        /**
         * @brief Récupère une ressource chargée par son nom.
         * @param name nom de la ressource précédemment chargée
         * @return un shared_ptr vers la ressource
         */
        virtual std::shared_ptr<IFont> getFont(const std::string& name) = 0;
        /**
         * @brief Récupère une ressource chargée par son nom.
         * @param name nom de la ressource précédemment chargée
         * @return un shared_ptr vers la ressource
         */
        virtual std::shared_ptr<ISoundBuffer> getSoundBuffer(const std::string& name) = 0;
        /**
         * @brief Récupère une ressource chargée par son nom.
         * @param name nom de la ressource précédemment chargée
         * @return un shared_ptr vers la ressource
         */
        virtual std::shared_ptr<IMusic> getMusic(const std::string& name) = 0;
        /**
         * @brief Crée un objet son à partir d'un buffer sonore.
         * @param buffer shared_ptr vers le buffer sonore
         * @return shared_ptr vers l'objet son créé
         */
        virtual std::shared_ptr<ISound> createSound(std::shared_ptr<ISoundBuffer> buffer) = 0;
        /**
         * @brief Charge un shader depuis des fichiers de vertex et fragment, et l'associe à un nom.
         * @param name nom du shader
         * @param vertexPath chemin vers le fichier de shader vertex
         * @param fragmentPath chemin vers le fichier de shader fragment
         */
        virtual void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) = 0;

        // Shader uniforms
        /**
         * @brief Définit une valeur uniforme float pour un shader donné.
         * @param shaderName Nom du shader
         * @param uniformName Nom de l'uniforme dans le shader
         * @param value Valeur float à assigner
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) = 0;

        /**
         * @brief Définit une matrice (vecteur de floats) comme uniforme pour un shader.
         * @param shaderName Nom du shader
         * @param uniformName Nom de l'uniforme dans le shader
         * @param matrix Vecteur de floats représentant la matrice
         */
        virtual void setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) = 0;

        // Render to texture
        /**
         * @brief Commence le rendu vers une texture rendable identifiée par son nom.
         * @param textureName Nom de la texture rendable
         */
        virtual void beginRenderToTexture(const std::string& textureName) = 0;
        virtual void endRenderToTexture() = 0;
        virtual void drawRenderTexture(const std::string& textureName, const std::string& shaderName) = 0;

        // Joystick
        /**
         * @brief Indique si un joystick est connecté.
         * @param joystickId Identifiant du joystick
         * @return true si connecté
         */
        [[nodiscard]] virtual bool isJoystickConnected(unsigned int joystickId) const = 0;
        /**
         * @brief Retourne le nombre de joysticks connectés.
         * @return Nombre de joysticks
         */
        [[nodiscard]] virtual unsigned int getJoystickCount() const = 0;
    };

} // namespace rtype::display

typedef rtype::display::IDisplay* (*entryPoint)();

#endif // IDISPLAY_HPP_
