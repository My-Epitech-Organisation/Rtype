/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SFMLDisplay.cpp
*/

#include "SFMLDisplay.hpp"

namespace rtype::display {

SFMLDisplay::SFMLDisplay() : _window(), _renderTarget(nullptr) {}

SFMLDisplay::~SFMLDisplay() {
    if (this->_window && this->_window->isOpen()) {
        this->_window->close();
    }
}

std::string SFMLDisplay::getLibName() const {
    return "SFML";
}

void SFMLDisplay::open(unsigned int width, unsigned int height, const std::string& title, const bool setFullscreen) {
    this->_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({width, height}), title, setFullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    this->_windowSizeHeight = height;
    this->_windowSizeWidth = width;
    this->_windowTitleName = title;
    this->_renderTarget = this->_window.get();
    this->_view = this->_window->getDefaultView();
}

bool SFMLDisplay::isOpen() const {
    return this->_window && this->_window->isOpen();
}

void SFMLDisplay::close() {
    if (this->_window) {
        this->_window->close();
    }
}

bool SFMLDisplay::pollEvent(Event& event) {
    if (!this->_window) return false;

    while (const std::optional sfEvent = this->_window->pollEvent()) {
        if (sfEvent->is<sf::Event::Closed>()) {
            event.type = EventType::Closed;
            return true;
        } else if (const auto* keyPressed = sfEvent->getIf<sf::Event::KeyPressed>()) {
            event.type = EventType::KeyPressed;
            event.key.code = _translateKey(keyPressed->code);
            event.key.alt = keyPressed->alt;
            event.key.control = keyPressed->control;
            event.key.shift = keyPressed->shift;
            event.key.system = keyPressed->system;
            return true;
        } else if (const auto* keyReleased = sfEvent->getIf<sf::Event::KeyReleased>()) {
            event.type = EventType::KeyReleased;
            event.key.code = _translateKey(keyReleased->code);
            event.key.alt = keyReleased->alt;
            event.key.control = keyReleased->control;
            event.key.shift = keyReleased->shift;
            event.key.system = keyReleased->system;
            return true;
        } else if (const auto* mouseMoved = sfEvent->getIf<sf::Event::MouseMoved>()) {
            event.type = EventType::MouseMoved;
            event.mouseMove.x = mouseMoved->position.x;
            event.mouseMove.y = mouseMoved->position.y;
            return true;
        } else if (const auto* mousePressed = sfEvent->getIf<sf::Event::MouseButtonPressed>()) {
            event.type = EventType::MouseButtonPressed;
            event.mouseButton.button = _translateMouseButton(mousePressed->button);
            event.mouseButton.x = mousePressed->position.x;
            event.mouseButton.y = mousePressed->position.y;
            return true;
        } else if (const auto* mouseReleased = sfEvent->getIf<sf::Event::MouseButtonReleased>()) {
            event.type = EventType::MouseButtonReleased;
            event.mouseButton.button = _translateMouseButton(mouseReleased->button);
            event.mouseButton.x = mouseReleased->position.x;
            event.mouseButton.y = mouseReleased->position.y;
            return true;
        } else if (const auto* joyPressed = sfEvent->getIf<sf::Event::JoystickButtonPressed>()) {
            event.type = EventType::JoystickButtonPressed;
            event.joystickButton.joystickId = joyPressed->joystickId;
            event.joystickButton.button = joyPressed->button;
            return true;
        } else if (const auto* joyReleased = sfEvent->getIf<sf::Event::JoystickButtonReleased>()) {
            event.type = EventType::JoystickButtonReleased;
            event.joystickButton.joystickId = joyReleased->joystickId;
            event.joystickButton.button = joyReleased->button;
            return true;
        } else if (const auto* joyMoved = sfEvent->getIf<sf::Event::JoystickMoved>()) {
            event.type = EventType::JoystickMoved;
            event.joystickMove.joystickId = joyMoved->joystickId;
            event.joystickMove.axis = _translateJoystickAxis(joyMoved->axis);
            event.joystickMove.position = joyMoved->position;
            return true;
        } else if (sfEvent->is<sf::Event::FocusLost>()) {
            event.type = EventType::FocusLost;
            return true;
        } else if (sfEvent->is<sf::Event::FocusGained>()) {
            event.type = EventType::FocusGained;
            return true;
        } else if (const auto* textEntered = sfEvent->getIf<sf::Event::TextEntered>()) {
            event.type = EventType::TextEntered;
            event.text.unicode = textEntered->unicode;
            return true;
        }
    }
    return false;
}

void SFMLDisplay::clear(const Color& color) {
    if (this->_renderTarget) {
        this->_renderTarget->clear(sf::Color(color.r, color.g, color.b, color.a));
    }
}

void SFMLDisplay::display() {
    if (this->_window) {
        this->_window->display();
    }
}

void SFMLDisplay::setFramerateLimit(unsigned int limit) {
    if (this->_window) {
        this->_window->setFramerateLimit(limit);
    }
}

void SFMLDisplay::setFullscreen(bool setFullscreen) {
    this->_window.reset();
    this->close();
    this->open(this->_windowSizeWidth, this->_windowSizeHeight, this->_windowTitleName, setFullscreen);
    this->_windowIsFullscreen = setFullscreen;

}

bool SFMLDisplay::isFullscreen() const {
    return this->_windowIsFullscreen;
}

void SFMLDisplay::drawSprite(const std::string& textureName, const Vector2f& position, const IntRect& rect, const Vector2f& scale, const Color& color, float rotation) {
    if (!this->_renderTarget || _textures.find(textureName) == _textures.end()) return;

    sf::Sprite sprite(_textures[textureName]->getSFMLTexture());
    if (rect.width > 0 && rect.height > 0) {
        sprite.setTextureRect(sf::IntRect({rect.left, rect.top}, {rect.width, rect.height}));
    }
    bool hasRotationComponent = (rotation != -999.0f);
    sf::Vector2f adjustedPosition = {position.x, position.y};
    if (hasRotationComponent) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
    }

    sprite.setPosition(adjustedPosition);
    sprite.setScale({scale.x, scale.y});
    sprite.setColor(sf::Color(color.r, color.g, color.b, color.a));

    if (hasRotationComponent) {
        sprite.setRotation(sf::degrees(rotation));
    }

    this->_renderTarget->draw(sprite);
}

void SFMLDisplay::drawText(const std::string& text, const std::string& fontName, const Vector2f& position, unsigned int size, const Color& color) {
    if (!this->_renderTarget || _fonts.find(fontName) == _fonts.end()) return;

    sf::Text sfText(_fonts[fontName]->getSFMLFont());
    sfText.setString(text);
    sfText.setCharacterSize(size);
    sfText.setFillColor(sf::Color(color.r, color.g, color.b, color.a));

    sf::FloatRect bounds = sfText.getLocalBounds();
    sfText.setOrigin({bounds.position.x, bounds.position.y});
    sfText.setPosition({position.x, position.y});
    this->_renderTarget->draw(sfText);
}

void SFMLDisplay::drawRectangle(const Vector2f& position, const Vector2f& size, const Color& fillColor, const Color& outlineColor, float outlineThickness) {
    if (!this->_renderTarget) return;

    sf::RectangleShape rect({size.x, size.y});
    rect.setPosition({position.x, position.y});
    rect.setFillColor(sf::Color(fillColor.r, fillColor.g, fillColor.b, fillColor.a));
    rect.setOutlineColor(sf::Color(outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a));
    rect.setOutlineThickness(outlineThickness);
    this->_renderTarget->draw(rect);
}

Vector2f SFMLDisplay::getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) {
    if (_fonts.find(fontName) == _fonts.end()) return {0, 0};
    sf::Text sfText(_fonts[fontName]->getSFMLFont());
    sfText.setString(text);
    sfText.setCharacterSize(size);
    sf::FloatRect bounds = sfText.getLocalBounds();
    return {bounds.size.x, bounds.size.y};
}

Vector2f SFMLDisplay::getTextureSize(const std::string& textureName) {
    if (_textures.find(textureName) == _textures.end()) return {0, 0};
    sf::Vector2u size = _textures[textureName]->getSFMLTexture().getSize();
    return {static_cast<float>(size.x), static_cast<float>(size.y)};
}

void SFMLDisplay::setView(const Vector2f& center, const Vector2f& size) {
    if (_renderTarget) {
        _view.setCenter({center.x, center.y});
        _view.setSize({size.x, size.y});
        _renderTarget->setView(_view);
    }
}

Vector2f SFMLDisplay::getViewCenter() const {
    sf::Vector2f center = _view.getCenter();
    return {center.x, center.y};
}

Vector2f SFMLDisplay::getViewSize() const {
    sf::Vector2f size = _view.getSize();
    return {size.x, size.y};
}

void SFMLDisplay::resetView() {
    if (_renderTarget && _window) {
        _view = _window->getDefaultView();
        _renderTarget->setView(_view);
    }
}

Vector2f SFMLDisplay::mapPixelToCoords(const Vector2i& pixelPos) const {
    if (_window) {
        sf::Vector2f coords = _window->mapPixelToCoords({pixelPos.x, pixelPos.y}, _view);
        return {coords.x, coords.y};
    }
    return {static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y)};
}

Vector2i SFMLDisplay::getWindowSize() const {
    if (_window) {
        sf::Vector2u size = _window->getSize();
        return {static_cast<int>(size.x), static_cast<int>(size.y)};
    }
    return {0, 0};
}

void SFMLDisplay::loadTexture(const std::string& name, const std::string& path) {
    auto texture = std::make_shared<SFMLTexture>();
    if (texture->loadFromFile(path)) {
        texture->setRepeated(true);
        _textures[name] = std::move(texture);
    }
}

std::shared_ptr<ITexture> SFMLDisplay::getTexture(const std::string& name) {
    if (_textures.find(name) != _textures.end()) {
        return _textures[name];
    }
    return nullptr;
}


void SFMLDisplay::loadFont(const std::string& name, const std::string& path) {
    auto font = std::make_shared<SFMLFont>();
    if (font->openFromFile(path)) {
        _fonts[name] = std::move(font);
    }
}

std::shared_ptr<IFont> SFMLDisplay::getFont(const std::string& name) {
    if (_fonts.find(name) != _fonts.end()) {
        return _fonts[name];
    }
    return nullptr;
}

void SFMLDisplay::loadSoundBuffer(const std::string& name, const std::string& path) {
    auto buffer = std::make_shared<SFMLSoundBuffer>();
    if (buffer->loadFromFile(path)) {
        _soundBuffers[name] = std::move(buffer);
    }
}

std::shared_ptr<ISoundBuffer> SFMLDisplay::getSoundBuffer(const std::string& name) {
    if (_soundBuffers.find(name) != _soundBuffers.end()) {
        return _soundBuffers[name];
    }
    return nullptr;
}

void SFMLDisplay::loadMusic(const std::string& name, const std::string& path) {
    auto music = std::make_shared<SFMLMusic>();
    if (music->openFromFile(path)) {
        _musics[name] = std::move(music);
    }
}

std::shared_ptr<IMusic> SFMLDisplay::getMusic(const std::string& name) {
    if (_musics.find(name) != _musics.end()) {
        return _musics[name];
    }
    return nullptr;
}

std::shared_ptr<ISound> SFMLDisplay::createSound(std::shared_ptr<ISoundBuffer> buffer) {
    auto sfmlBuffer = std::dynamic_pointer_cast<SFMLSoundBuffer>(buffer);
    if (sfmlBuffer) {
        return std::make_shared<SFMLSound>(sfmlBuffer);
    }
    return nullptr;
}

void SFMLDisplay::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto shader = std::make_unique<sf::Shader>();
    bool loaded = false;
    if (vertexPath.empty() && !fragmentPath.empty()) {
        loaded = shader->loadFromFile(fragmentPath, sf::Shader::Type::Fragment);
    } else {
        loaded = shader->loadFromFile(vertexPath, fragmentPath);
    }

    if (loaded) {
        _shaders[name] = std::move(shader);
    }
}

void SFMLDisplay::setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) {
    if (_shaders.find(shaderName) != _shaders.end()) {
        _shaders[shaderName]->setUniform(uniformName, value);
    }
}

void SFMLDisplay::setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) {
    if (_shaders.find(shaderName) != _shaders.end() && matrix.size() == 9) {
        _shaders[shaderName]->setUniform(uniformName, sf::Glsl::Mat3(matrix.data()));
    }
}

void SFMLDisplay::beginRenderToTexture(const std::string& textureName) {
    if (_renderTextures.find(textureName) == _renderTextures.end()) {
        if (!_window) return;
        auto rt = std::make_unique<sf::RenderTexture>();
        if (!rt->resize(_window->getSize())) return;
        _renderTextures[textureName] = std::move(rt);
    }
    _renderTarget = _renderTextures[textureName].get();
    _renderTarget->setView(_view);
}

void SFMLDisplay::endRenderToTexture() {
    if (auto* rt = dynamic_cast<sf::RenderTexture*>(_renderTarget)) {
        rt->display();
    }
    _renderTarget = _window.get();
    _renderTarget->setView(_view);
}

void SFMLDisplay::drawRenderTexture(const std::string& textureName, const std::string& shaderName) {
    if (!_window || _renderTextures.find(textureName) == _renderTextures.end()) return;

    sf::Sprite sprite(_renderTextures[textureName]->getTexture());
    if (shaderName != "" && _shaders.find(shaderName) != _shaders.end()) {
        _window->draw(sprite, _shaders[shaderName].get());
    } else {
        _window->draw(sprite);
    }
}

Key SFMLDisplay::_translateKey(sf::Keyboard::Key key) {
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z)
        return static_cast<Key>(static_cast<int>(Key::A) + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::A)));
    if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9)
        return static_cast<Key>(static_cast<int>(Key::Num0) + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num0)));

    switch (key) {
        case sf::Keyboard::Key::Escape: return Key::Escape;
        case sf::Keyboard::Key::LControl: return Key::LControl;
        case sf::Keyboard::Key::LShift: return Key::LShift;
        case sf::Keyboard::Key::LAlt: return Key::LAlt;
        case sf::Keyboard::Key::LSystem: return Key::LSystem;
        case sf::Keyboard::Key::RControl: return Key::RControl;
        case sf::Keyboard::Key::RShift: return Key::RShift;
        case sf::Keyboard::Key::RAlt: return Key::RAlt;
        case sf::Keyboard::Key::RSystem: return Key::RSystem;
        case sf::Keyboard::Key::Menu: return Key::Menu;
        case sf::Keyboard::Key::LBracket: return Key::LBracket;
        case sf::Keyboard::Key::RBracket: return Key::RBracket;
        case sf::Keyboard::Key::Semicolon: return Key::SemiColon;
        case sf::Keyboard::Key::Comma: return Key::Comma;
        case sf::Keyboard::Key::Period: return Key::Period;
        case sf::Keyboard::Key::Apostrophe: return Key::Quote;
        case sf::Keyboard::Key::Slash: return Key::Slash;
        case sf::Keyboard::Key::Backslash: return Key::BackSlash;
        case sf::Keyboard::Key::Grave: return Key::Tilde;
        case sf::Keyboard::Key::Equal: return Key::Equal;
        case sf::Keyboard::Key::Hyphen: return Key::Dash;
        case sf::Keyboard::Key::Space: return Key::Space;
        case sf::Keyboard::Key::Enter: return Key::Return;
        case sf::Keyboard::Key::Backspace: return Key::BackSpace;
        case sf::Keyboard::Key::Tab: return Key::Tab;
        case sf::Keyboard::Key::PageUp: return Key::PageUp;
        case sf::Keyboard::Key::PageDown: return Key::PageDown;
        case sf::Keyboard::Key::End: return Key::End;
        case sf::Keyboard::Key::Home: return Key::Home;
        case sf::Keyboard::Key::Insert: return Key::Insert;
        case sf::Keyboard::Key::Delete: return Key::Delete;
        case sf::Keyboard::Key::Add: return Key::Add;
        case sf::Keyboard::Key::Subtract: return Key::Subtract;
        case sf::Keyboard::Key::Multiply: return Key::Multiply;
        case sf::Keyboard::Key::Divide: return Key::Divide;
        case sf::Keyboard::Key::Left: return Key::Left;
        case sf::Keyboard::Key::Right: return Key::Right;
        case sf::Keyboard::Key::Up: return Key::Up;
        case sf::Keyboard::Key::Down: return Key::Down;
        default: return Key::Unknown;
    }
}

MouseButton SFMLDisplay::_translateMouseButton(sf::Mouse::Button button) {
    switch (button) {
        case sf::Mouse::Button::Left: return MouseButton::Left;
        case sf::Mouse::Button::Right: return MouseButton::Right;
        case sf::Mouse::Button::Middle: return MouseButton::Middle;
        case sf::Mouse::Button::Extra1: return MouseButton::XButton1;
        case sf::Mouse::Button::Extra2: return MouseButton::XButton2;
        default: return MouseButton::ButtonCount;
    }
}

JoystickAxis SFMLDisplay::_translateJoystickAxis(sf::Joystick::Axis axis) {
    switch (axis) {
        case sf::Joystick::Axis::X: return JoystickAxis::X;
        case sf::Joystick::Axis::Y: return JoystickAxis::Y;
        case sf::Joystick::Axis::Z: return JoystickAxis::Z;
        case sf::Joystick::Axis::R: return JoystickAxis::R;
        case sf::Joystick::Axis::U: return JoystickAxis::U;
        case sf::Joystick::Axis::V: return JoystickAxis::V;
        case sf::Joystick::Axis::PovX: return JoystickAxis::PovX;
        case sf::Joystick::Axis::PovY: return JoystickAxis::PovY;
        default: return JoystickAxis::X;
    }
}

bool SFMLDisplay::isJoystickConnected(unsigned int joystickId) const {
    return sf::Joystick::isConnected(joystickId);
}

unsigned int SFMLDisplay::getJoystickCount() const {
    return sf::Joystick::Count;
}

extern "C" IDisplay* entryPoint() {
    return new SFMLDisplay();
}

} // namespace rtype::display
