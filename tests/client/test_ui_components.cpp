/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for client UI components (Rectangle, Text, Button)
*/

#include <gtest/gtest.h>

#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/TextComponent.hpp"
#include "games/rtype/client/Components/ButtonComponent.hpp"

using namespace rtype::games::rtype::client;

// =============================================================================
// RectangleComponent Tests
// =============================================================================

class RectangleComponentTest : public ::testing::Test {
   protected:
    sf::Color mainColor = sf::Color::Blue;
    sf::Color hoveredColor = sf::Color::Red;
    std::pair<float, float> size = {100.0f, 50.0f};
};

TEST_F(RectangleComponentTest, ConstructorInitializesSize) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_FLOAT_EQ(rect.size.first, 100.0f);
    EXPECT_FLOAT_EQ(rect.size.second, 50.0f);
}

TEST_F(RectangleComponentTest, ConstructorInitializesMainColor) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_EQ(rect.mainColor, mainColor);
}

TEST_F(RectangleComponentTest, ConstructorInitializesHoveredColor) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_EQ(rect.hoveredColor, hoveredColor);
}

TEST_F(RectangleComponentTest, CurrentColorDefaultsToMainColor) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_EQ(rect.currentColor, mainColor);
}

TEST_F(RectangleComponentTest, DefaultOutlineThickness) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_FLOAT_EQ(rect.outlineThickness, 0.0f);
}

TEST_F(RectangleComponentTest, DefaultOutlineColor) {
    Rectangle rect(size, mainColor, hoveredColor);
    EXPECT_EQ(rect.outlineColor, sf::Color::Black);
}

TEST_F(RectangleComponentTest, CopyConstructor) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.outlineThickness = 2.0f;
    rect.outlineColor = sf::Color::White;
    rect.currentColor = hoveredColor;

    Rectangle copy(rect);

    EXPECT_EQ(copy.size, rect.size);
    EXPECT_EQ(copy.mainColor, rect.mainColor);
    EXPECT_EQ(copy.hoveredColor, rect.hoveredColor);
    EXPECT_EQ(copy.currentColor, rect.currentColor);
    EXPECT_FLOAT_EQ(copy.outlineThickness, rect.outlineThickness);
    EXPECT_EQ(copy.outlineColor, rect.outlineColor);
}

TEST_F(RectangleComponentTest, MoveConstructor) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.outlineThickness = 3.0f;

    Rectangle moved(std::move(rect));

    EXPECT_FLOAT_EQ(moved.size.first, 100.0f);
    EXPECT_FLOAT_EQ(moved.size.second, 50.0f);
    EXPECT_EQ(moved.mainColor, mainColor);
    EXPECT_FLOAT_EQ(moved.outlineThickness, 3.0f);
}

TEST_F(RectangleComponentTest, CopyAssignmentOperator) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.outlineThickness = 5.0f;

    std::pair<float, float> otherSize = {200.0f, 100.0f};
    Rectangle other(otherSize, sf::Color::Green, sf::Color::Yellow);

    other = rect;

    EXPECT_EQ(other.size, rect.size);
    EXPECT_EQ(other.mainColor, rect.mainColor);
    EXPECT_EQ(other.hoveredColor, rect.hoveredColor);
    EXPECT_FLOAT_EQ(other.outlineThickness, 5.0f);
}

TEST_F(RectangleComponentTest, MoveAssignmentOperator) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.outlineThickness = 7.0f;

    std::pair<float, float> otherSize = {200.0f, 100.0f};
    Rectangle other(otherSize, sf::Color::Green, sf::Color::Yellow);

    other = std::move(rect);

    EXPECT_FLOAT_EQ(other.size.first, 100.0f);
    EXPECT_FLOAT_EQ(other.size.second, 50.0f);
    EXPECT_EQ(other.mainColor, mainColor);
    EXPECT_FLOAT_EQ(other.outlineThickness, 7.0f);
}

TEST_F(RectangleComponentTest, ModifyCurrentColor) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.currentColor = hoveredColor;
    EXPECT_EQ(rect.currentColor, hoveredColor);
}

TEST_F(RectangleComponentTest, ModifyOutlineProperties) {
    Rectangle rect(size, mainColor, hoveredColor);
    rect.outlineThickness = 4.5f;
    rect.outlineColor = sf::Color::Cyan;

    EXPECT_FLOAT_EQ(rect.outlineThickness, 4.5f);
    EXPECT_EQ(rect.outlineColor, sf::Color::Cyan);
}

// =============================================================================
// TextComponent Tests
// =============================================================================

class TextComponentTest : public ::testing::Test {
   protected:
    sf::Font font;
    sf::Color textColor = sf::Color::White;
};

TEST_F(TextComponentTest, ConstructorWithDefaultSize) {
    Text text(font, textColor);
    EXPECT_EQ(text.size, 30u);
}

TEST_F(TextComponentTest, ConstructorWithCustomSize) {
    Text text(font, textColor, 24);
    EXPECT_EQ(text.size, 24u);
}

TEST_F(TextComponentTest, ConstructorWithTextContent) {
    Text text(font, textColor, 30, "Hello World");
    EXPECT_EQ(text.textContent, "Hello World");
}

TEST_F(TextComponentTest, ConstructorInitializesColor) {
    Text text(font, textColor, 30, "Test");
    EXPECT_EQ(text.color, textColor);
}

TEST_F(TextComponentTest, DefaultTextContentIsEmpty) {
    Text text(font, textColor);
    EXPECT_TRUE(text.textContent.empty());
}

TEST_F(TextComponentTest, SfTextIsSetCorrectly) {
    Text text(font, textColor, 30, "Test String");
    EXPECT_EQ(text.text.getString(), "Test String");
}

TEST_F(TextComponentTest, CopyConstructor) {
    Text text(font, sf::Color::Red, 20, "Original");
    Text copy(text);

    EXPECT_EQ(copy.textContent, text.textContent);
    EXPECT_EQ(copy.color, text.color);
    EXPECT_EQ(copy.size, text.size);
}

TEST_F(TextComponentTest, MoveConstructor) {
    Text text(font, sf::Color::Green, 18, "Moved Text");
    Text moved(std::move(text));

    EXPECT_EQ(moved.textContent, "Moved Text");
    EXPECT_EQ(moved.color, sf::Color::Green);
    EXPECT_EQ(moved.size, 18u);
}

TEST_F(TextComponentTest, CopyAssignmentOperator) {
    Text text(font, sf::Color::Blue, 16, "Source");
    Text other(font, sf::Color::White, 30, "Target");

    other = text;

    EXPECT_EQ(other.textContent, "Source");
    EXPECT_EQ(other.color, sf::Color::Blue);
    EXPECT_EQ(other.size, 16u);
}

TEST_F(TextComponentTest, MoveAssignmentOperator) {
    Text text(font, sf::Color::Yellow, 14, "MovedSource");
    Text other(font, sf::Color::White, 30, "Target");

    other = std::move(text);

    EXPECT_EQ(other.textContent, "MovedSource");
    EXPECT_EQ(other.color, sf::Color::Yellow);
    EXPECT_EQ(other.size, 14u);
}

TEST_F(TextComponentTest, ModifyTextContent) {
    Text text(font, textColor, 30, "Initial");
    text.textContent = "Modified";
    EXPECT_EQ(text.textContent, "Modified");
}

TEST_F(TextComponentTest, ModifyColor) {
    Text text(font, textColor, 30, "Test");
    text.color = sf::Color::Magenta;
    EXPECT_EQ(text.color, sf::Color::Magenta);
}

TEST_F(TextComponentTest, ModifySize) {
    Text text(font, textColor, 30, "Test");
    text.size = 48;
    EXPECT_EQ(text.size, 48u);
}

TEST_F(TextComponentTest, EmptyStringContent) {
    Text text(font, textColor, 30, "");
    EXPECT_TRUE(text.textContent.empty());
    EXPECT_EQ(text.text.getString(), "");
}

TEST_F(TextComponentTest, SpecialCharactersInContent) {
    Text text(font, textColor, 30, "Hello\nWorld\t!");
    EXPECT_EQ(text.textContent, "Hello\nWorld\t!");
}

// =============================================================================
// ButtonComponent Tests
// =============================================================================

TEST(ButtonComponentTest, ConstructorWithVoidCallback) {
    bool called = false;
    Button<> button([&called]() { called = true; });

    button.callback();

    EXPECT_TRUE(called);
}

TEST(ButtonComponentTest, ConstructorWithIntCallback) {
    int receivedValue = 0;
    Button<int> button([&receivedValue](int val) { receivedValue = val; });

    button.callback(42);

    EXPECT_EQ(receivedValue, 42);
}

TEST(ButtonComponentTest, ConstructorWithMultipleArgsCallback) {
    int sum = 0;
    Button<int, int> button([&sum](int a, int b) { sum = a + b; });

    button.callback(10, 20);

    EXPECT_EQ(sum, 30);
}

TEST(ButtonComponentTest, ConstructorWithStringCallback) {
    std::string receivedText;
    Button<const std::string&> button(
        [&receivedText](const std::string& text) { receivedText = text; });

    button.callback("Hello Button");

    EXPECT_EQ(receivedText, "Hello Button");
}

TEST(ButtonComponentTest, CallbackCanBeReassigned) {
    int counter = 0;
    Button<> button([&counter]() { counter += 1; });

    button.callback();
    EXPECT_EQ(counter, 1);

    button.callback = [&counter]() { counter += 10; };
    button.callback();
    EXPECT_EQ(counter, 11);
}

TEST(ButtonComponentTest, CallbackWithReturnValueIgnored) {
    Button<> button([]() {
        // Return value is ignored since callback is std::function<void(Args...)>
    });
    button.callback();
    SUCCEED();
}

TEST(ButtonComponentTest, MultipleCallbackInvocations) {
    int callCount = 0;
    Button<> button([&callCount]() { callCount++; });

    button.callback();
    button.callback();
    button.callback();

    EXPECT_EQ(callCount, 3);
}

TEST(ButtonComponentTest, CallbackWithComplexTypes) {
    std::vector<int> receivedVec;
    Button<const std::vector<int>&> button(
        [&receivedVec](const std::vector<int>& vec) { receivedVec = vec; });

    std::vector<int> input = {1, 2, 3, 4, 5};
    button.callback(input);

    EXPECT_EQ(receivedVec, input);
}

TEST(ButtonComponentTest, CallbackModifiesExternalState) {
    struct State {
        int value = 0;
        bool active = false;
    } state;

    Button<int, bool> button([&state](int v, bool a) {
        state.value = v;
        state.active = a;
    });

    button.callback(100, true);

    EXPECT_EQ(state.value, 100);
    EXPECT_TRUE(state.active);
}
