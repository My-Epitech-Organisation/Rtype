/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextInputComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTINPUTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTINPUTCOMPONENT_HPP_

#include <cctype>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace rtype::games::rtype::client {

/**
 * @brief Text input component for editable text fields.
 *
 * Allows users to type text into a field. Supports focus management,
 * placeholder text, and optional validation.
 */
struct TextInput {
    std::shared_ptr<sf::Font> font;
    sf::Text text;
    sf::RectangleShape background;
    std::string content;
    std::string placeholder;
    sf::Color textColor;
    sf::Color backgroundColor;
    sf::Color focusedBorderColor;
    sf::Color unfocusedBorderColor;
    unsigned int fontSize;
    std::size_t maxLength;
    bool isFocused;
    bool isNumericOnly;
    std::function<void(const std::string&)> onChanged;
    std::function<void(const std::string&)> onSubmit;

    /**
     * @brief Construct a new TextInput component.
     * @param font Reference to the font
     * @param width Width of the input field
     * @param height Height of the input field
     * @param placeholder Placeholder text when empty
     * @param initialValue Initial text content
     * @param maxLength Maximum number of characters (0 = unlimited)
     * @param isNumericOnly Only allow numeric input
     */
    TextInput(std::shared_ptr<sf::Font> font, float width, float height,
              std::string_view placeholder = "",
              std::string_view initialValue = "", std::size_t maxLength = 0,
              bool isNumericOnly = false)
        : font(font),
          text(*font),
          background({width, height}),
          content(initialValue),
          placeholder(placeholder),
          textColor(sf::Color::White),
          backgroundColor(sf::Color(50, 50, 50)),
          focusedBorderColor(sf::Color::Cyan),
          unfocusedBorderColor(sf::Color(100, 100, 100)),
          fontSize(24),
          maxLength(maxLength),
          isFocused(false),
          isNumericOnly(isNumericOnly) {
        background.setFillColor(backgroundColor);
        background.setOutlineThickness(2.f);
        background.setOutlineColor(unfocusedBorderColor);
        text.setString(
            std::string(initialValue.empty() ? placeholder : initialValue));
        text.setCharacterSize(fontSize);
        text.setFillColor(content.empty() ? sf::Color(150, 150, 150)
                                          : textColor);
    }

    /**
     * @brief Update the displayed text
     */
    void updateDisplay() {
        if (content.empty() && !isFocused) {
            text.setString(placeholder);
            text.setFillColor(sf::Color(150, 150, 150));
        } else {
            text.setString(content + (isFocused ? "_" : ""));
            text.setFillColor(textColor);
        }
        background.setOutlineColor(isFocused ? focusedBorderColor
                                             : unfocusedBorderColor);
    }

    /**
     * @brief Handle text input character
     * @param character The character to add
     * @return true if character was accepted
     */
    bool handleTextInput(char character) {
        if (!isFocused) return false;
        if (maxLength > 0 && content.length() >= maxLength) return false;
        if (isNumericOnly && !std::isdigit(character)) return false;
        if (!std::isprint(character)) return false;

        content += character;
        updateDisplay();
        if (onChanged) onChanged(content);
        return true;
    }

    /**
     * @brief Handle backspace key
     */
    void handleBackspace() {
        if (!isFocused || content.empty()) return;
        content.pop_back();
        updateDisplay();
        if (onChanged) onChanged(content);
    }

    /**
     * @brief Set focus state
     */
    void setFocus(bool focused) {
        isFocused = focused;
        updateDisplay();
    }

    TextInput(const TextInput& other) = default;
    TextInput(TextInput&& other) noexcept = default;
    TextInput& operator=(const TextInput& other) = default;
    TextInput& operator=(TextInput&& other) noexcept = default;
    ~TextInput() = default;
};

/// Tag for text input entities
struct TextInputTag {};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTINPUTCOMPONENT_HPP_
