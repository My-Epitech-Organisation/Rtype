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

#include "../../../../../include/rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Text input component for editable text fields.
 *
 * Allows users to type text into a field. Supports focus management,
 * placeholder text, and optional validation.
 */
struct TextInput {
    std::string fontName;
    std::string content;
    std::string placeholder;
    ::rtype::display::Color textColor;
    ::rtype::display::Color backgroundColor;
    ::rtype::display::Color focusedBorderColor;
    ::rtype::display::Color unfocusedBorderColor;
    unsigned int fontSize;
    std::size_t maxLength;
    bool isFocused;
    bool isNumericOnly;
    ::rtype::display::Vector2f size;
    std::function<void(const std::string&)> onChanged;
    std::function<void(const std::string&)> onSubmit;

    /**
     * @brief Construct a new TextInput component.
     * @param fontName Name of the font in the asset manager
     * @param width Width of the input field
     * @param height Height of the input field
     * @param placeholder Placeholder text when empty
     * @param initialValue Initial text content
     * @param maxLength Maximum number of characters (0 = unlimited)
     * @param isNumericOnly Only allow numeric input
     */
    TextInput(std::string fontName, float width, float height,
              std::string_view placeholder = "",
              std::string_view initialValue = "", std::size_t maxLength = 0,
              bool isNumericOnly = false)
        : fontName(std::move(fontName)),
          content(initialValue),
          placeholder(placeholder),
          textColor(::rtype::display::Color::White()),
          backgroundColor(::rtype::display::Color(50, 50, 50)),
          focusedBorderColor(::rtype::display::Color::Cyan()),
          unfocusedBorderColor(::rtype::display::Color::White()),
          fontSize(24),
          maxLength(maxLength),
          isFocused(false),
          isNumericOnly(isNumericOnly),
          size({width, height}) {}

    /**
     * @brief Update the displayed text
     */
    void updateDisplay() {
        // Logic moved to RenderSystem or handled by IDisplay
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
