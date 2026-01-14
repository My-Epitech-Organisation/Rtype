/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SDL2Display.cpp - SDL2 implementation of IDisplay
*/

#include "SDL2Display.hpp"
#include "SDL2AudioEngine.hpp"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace rtype::display {

SDL2Display::SDL2Display() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL2 initialization failed: " << SDL_GetError() << std::endl;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL2_ttf initialization failed: " << TTF_GetError() << std::endl;
    }
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::cerr << "SDL2_image initialization failed: " << IMG_GetError() << std::endl;
    }
    initSDL2AudioEngine();
}

SDL2Display::~SDL2Display() {
    close();
    for (auto& kv : _renderTextures) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
        }
    }
    _renderTextures.clear();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

std::string SDL2Display::getLibName() const {
    return "SDL2";
}

void SDL2Display::open(unsigned int width, unsigned int height, const std::string& title, bool setFullscreen) {
    _windowSizeWidth = width;
    _windowSizeHeight = height;
    _windowTitleName = title;
    _windowIsFullscreen = setFullscreen;

    _viewSize = {static_cast<float>(width), static_cast<float>(height)};
    _viewCenter = {width / 2.0f, height / 2.0f};

    Uint32 flags = SDL_WINDOW_SHOWN;
    if (setFullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    _window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, flags);
    if (!_window) {
        std::cerr << "SDL2 window creation failed: " << SDL_GetError() << std::endl;
        return;
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        std::cerr << "SDL2 renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(_window);
        _window = nullptr;
        return;
    }

    SDL_RenderSetLogicalSize(_renderer, width, height);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_StartTextInput();

    _isOpen = true;
    _lastFrameTime = SDL_GetTicks();
}

bool SDL2Display::isOpen() const {
    return _isOpen;
}

void SDL2Display::close() {
    if (_debugVisuals) {
        std::cerr << "[SDL2Display] Closing display, destroying " << _renderTextures.size() << " render textures and " << _textures.size() << " textures" << std::endl;
    }
    for (auto &kv : _renderTextures) {
        if (kv.second) {
            SDL_DestroyTexture(kv.second);
            kv.second = nullptr;
        }
    }
    _renderTextures.clear();

    for (auto &kv : _textures) {
        if (kv.second) {
            SDL_Texture* ptr = kv.second->getSDL2Texture();
            if (ptr) {
                SDL_DestroyTexture(ptr);
                kv.second->setSDL2Texture(nullptr);
            }
        }
    }

    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    _isOpen = false;
}

bool SDL2Display::pollEvent(Event& event) {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT:
                event.type = EventType::Closed;
                return true;
            case SDL_KEYDOWN:
                event.type = EventType::KeyPressed;
                event.key.code = translateKey(sdlEvent.key.keysym.sym);
                return true;
            case SDL_KEYUP:
                event.type = EventType::KeyReleased;
                event.key.code = translateKey(sdlEvent.key.keysym.sym);
                return true;
            case SDL_MOUSEBUTTONDOWN:
                event.type = EventType::MouseButtonPressed;
                event.mouseButton.button = translateMouseButton(sdlEvent.button.button);
                event.mouseButton.x = sdlEvent.button.x;
                event.mouseButton.y = sdlEvent.button.y;
                return true;
            case SDL_MOUSEBUTTONUP:
                event.type = EventType::MouseButtonReleased;
                event.mouseButton.button = translateMouseButton(sdlEvent.button.button);
                event.mouseButton.x = sdlEvent.button.x;
                event.mouseButton.y = sdlEvent.button.y;
                return true;
            case SDL_MOUSEMOTION:
                event.type = EventType::MouseMoved;
                event.mouseMove.x = sdlEvent.motion.x;
                event.mouseMove.y = sdlEvent.motion.y;
                return true;
            case SDL_TEXTINPUT:
                event.type = EventType::TextEntered;
                if (sdlEvent.text.text[0] != '\0') {
                    event.text.unicode = static_cast<uint32_t>(static_cast<unsigned char>(sdlEvent.text.text[0]));
                } else {
                    event.text.unicode = 0;
                }
                return true;
            default:
                break;
        }
    }
    return false;
}

void SDL2Display::clear(const Color& color) {
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(_renderer);

    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
}

void SDL2Display::display() {
    SDL_RenderPresent(_renderer);

    if (_framerateLimit > 0) {
        Uint32 frameTime = SDL_GetTicks() - _lastFrameTime;
        Uint32 targetFrameTime = 1000 / _framerateLimit;
        if (frameTime < targetFrameTime) {
            SDL_Delay(targetFrameTime - frameTime);
        }
        _lastFrameTime = SDL_GetTicks();
    }
}

void SDL2Display::setFramerateLimit(unsigned int limit) {
    _framerateLimit = limit;
}

void SDL2Display::setFullscreen(bool fullscreen) {
    if (_window) {
        SDL_SetWindowFullscreen(_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
        _windowIsFullscreen = fullscreen;
    }
}

bool SDL2Display::isFullscreen() const {
    return _windowIsFullscreen;
}

void SDL2Display::drawSprite(const std::string& textureName, const Vector2<float>& position,
                             const Rect<int>& rect, const Vector2<float>& scale, const Color& color, float rotation) {
    auto it = _textures.find(textureName);
    if (it == _textures.end()) return;

    SDL_Texture* texture = it->second->getSDL2Texture();
    if (!texture) return;
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    const int srcW = rect.width > 0 ? rect.width : texW;
    const int srcH = rect.height > 0 ? rect.height : texH;
    SDL_Rect srcRect = {rect.left, rect.top, srcW, srcH};

    Vector2<int> screenPos = worldToScreenPosition(position);
    Vector2<int> screenSize = worldToScreenSize({static_cast<float>(srcW) * scale.x, static_cast<float>(srcH) * scale.y});

    SDL_Rect dstRect = {screenPos.x, screenPos.y, screenSize.x, screenSize.y};

    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);

    if (srcRect.x + srcRect.w <= texW) {
        if (rotation != 0.0f) {
            SDL_RenderCopyEx(_renderer, texture, &srcRect, &dstRect, rotation, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(_renderer, texture, &srcRect, &dstRect);
        }
    } else {
        int part1Width = texW - srcRect.x;
        int part2Width = srcRect.w - part1Width;

        int screenTotalW = dstRect.w;
        int screenPart1W = static_cast<int>(std::round((static_cast<float>(part1Width) / static_cast<float>(srcW)) * screenTotalW));
        int screenPart2W = screenTotalW - screenPart1W;

        SDL_Rect src1 = {srcRect.x, srcRect.y, part1Width, srcRect.h};
        SDL_Rect dst1 = {dstRect.x, dstRect.y, screenPart1W, dstRect.h};
        if (rotation != 0.0f) {
            SDL_RenderCopyEx(_renderer, texture, &src1, &dst1, rotation, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(_renderer, texture, &src1, &dst1);
        }

        SDL_Rect src2 = {0, srcRect.y, part2Width, srcRect.h};
        SDL_Rect dst2 = {dstRect.x + screenPart1W, dstRect.y, screenPart2W, dstRect.h};
        if (rotation != 0.0f) {
            SDL_RenderCopyEx(_renderer, texture, &src2, &dst2, rotation, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(_renderer, texture, &src2, &dst2);
        }
    }
}

void SDL2Display::drawText(const std::string& text, const std::string& fontName,
                           const Vector2<float>& position, unsigned int size, const Color& color) {
    if (text.empty()) return;

    TTF_Font* font = nullptr;
    auto fontIt = _fonts.find(fontName);
    if (fontIt != _fonts.end()) {
        font = fontIt->second->getFont(size);
        if (!font) {
            std::cerr << "Failed to get font " << fontName << " at size " << size << std::endl;
        }
    } else {
        std::cerr << "Font " << fontName << " not loaded" << std::endl;
    }

    if (!font) {
        Vector2<int> p = worldToScreenPosition(position);
        int charWidth = static_cast<int>((size / 2.0f) * (_windowSizeWidth / _viewSize.x));
        int charHeight = static_cast<int>(size * (_windowSizeHeight / _viewSize.y));

        SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
        int x = p.x;
        for (char c : text) {
            if (c != ' ') {
                SDL_Rect rect = {x, p.y, charWidth - 2, charHeight};
                SDL_RenderFillRect(_renderer, &rect);
            }
            x += charWidth;
        }
        return;
    }

    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) {
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }

    Vector2<int> screenPos = worldToScreenPosition(position);
    Vector2<int> screenSize = worldToScreenSize({static_cast<float>(surface->w), static_cast<float>(surface->h)});

    SDL_Rect dstRect = {screenPos.x, screenPos.y, screenSize.x, screenSize.y};

    SDL_RenderCopy(_renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}

void SDL2Display::drawRectangle(const Vector2<float>& position, const Vector2<float>& size,
                                const Color& fillColor, const Color& outlineColor, float outlineThickness) {
    Vector2<int> p = worldToScreenPosition(position);
    Vector2<int> s = worldToScreenSize(size);

    SDL_Rect rect = {p.x, p.y, s.x, s.y};

    SDL_SetRenderDrawColor(_renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(_renderer, &rect);

    if (outlineThickness > 0) {
        SDL_SetRenderDrawColor(_renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        for (int i = 0; i < static_cast<int>(outlineThickness); i++) {
            SDL_Rect outlineRect = {rect.x - i, rect.y - i, rect.w + 2*i, rect.h + 2*i};
            SDL_RenderDrawRect(_renderer, &outlineRect);
        }
    }
}

Vector2<float> SDL2Display::getTextBounds(const std::string& text, const std::string& fontName, unsigned int size) {
    auto fontIt = _fonts.find(fontName);
    if (fontIt == _fonts.end()) {
        return {static_cast<float>(text.length() * size / 2), static_cast<float>(size)};
    }

    TTF_Font* font = fontIt->second->getFont(size);
    if (!font) {
        return {static_cast<float>(text.length() * size / 2), static_cast<float>(size)};
    }

    int w, h;
    TTF_SizeText(font, text.c_str(), &w, &h);
    TTF_CloseFont(font);
    return {static_cast<float>(w), static_cast<float>(h)};
}

Vector2<float> SDL2Display::getTextureSize(const std::string& textureName) {
    auto it = _textures.find(textureName);
    if (it == _textures.end()) return {0, 0};
    auto sz = it->second->getSize();
    return {static_cast<float>(sz.x), static_cast<float>(sz.y)};
}

void SDL2Display::setView(const Vector2<float>& center, const Vector2<float>& size) {
    _viewCenter = center;
    _viewSize = size;
}

Vector2<float> SDL2Display::getViewCenter() const {
    return _viewCenter;
}

Vector2<float> SDL2Display::getViewSize() const {
    return _viewSize;
}

void SDL2Display::resetView() {
    _viewSize = {static_cast<float>(_windowSizeWidth), static_cast<float>(_windowSizeHeight)};
    _viewCenter = {_windowSizeWidth / 2.0f, _windowSizeHeight / 2.0f};
}

Vector2<float> SDL2Display::mapPixelToCoords(const Vector2<int>& pixelPos) const {
    float left = _viewCenter.x - (_viewSize.x / 2.0f);
    float top = _viewCenter.y - (_viewSize.y / 2.0f);
    float worldX = left + (pixelPos.x * _viewSize.x / static_cast<float>(_windowSizeWidth));
    float worldY = top + (pixelPos.y * _viewSize.y / static_cast<float>(_windowSizeHeight));
    return {worldX, worldY};
}

Vector2<int> SDL2Display::worldToScreenPosition(const Vector2<float>& pos) const {
    float left = _viewCenter.x - (_viewSize.x / 2.0f);
    float top = _viewCenter.y - (_viewSize.y / 2.0f);
    float fx = (pos.x - left) * (static_cast<float>(_windowSizeWidth) / _viewSize.x);
    float fy = (pos.y - top) * (static_cast<float>(_windowSizeHeight) / _viewSize.y);
    return {static_cast<int>(fx), static_cast<int>(fy)};
}

Vector2<int> SDL2Display::worldToScreenSize(const Vector2<float>& size) const {
    float sx = size.x * (static_cast<float>(_windowSizeWidth) / _viewSize.x);
    float sy = size.y * (static_cast<float>(_windowSizeHeight) / _viewSize.y);
    return {static_cast<int>(sx), static_cast<int>(sy)};
}

Vector2<int> SDL2Display::getWindowSize() const {
    return {static_cast<int>(_windowSizeWidth), static_cast<int>(_windowSizeHeight)};
}

void SDL2Display::loadTexture(const std::string& name, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << path << " - " << IMG_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        auto sdl2Texture = std::make_shared<SDL2Texture>();
        sdl2Texture->setSDL2Texture(texture);
        _textures[name] = sdl2Texture;
    }
}

std::shared_ptr<ITexture> SDL2Display::getTexture(const std::string& name) {
    auto it = _textures.find(name);
    if (it != _textures.end()) {
        return it->second;
    }
    return nullptr;
}

void SDL2Display::loadFont(const std::string& name, const std::string& path) {
    auto font = std::make_shared<SDL2Font>();
    if (!font->openFromFile(path)) {
        return;
    }

    _fonts[name] = font;
}

std::shared_ptr<IFont> SDL2Display::getFont(const std::string& name) {
    auto it = _fonts.find(name);
    if (it != _fonts.end()) {
        return it->second;
    }
    return nullptr;
}

void SDL2Display::loadSoundBuffer(const std::string& name, const std::string& path) {
    auto buffer = std::make_shared<SDL2SoundBuffer>();
    if (buffer->loadFromFile(path)) {
        _soundBuffers[name] = buffer;
    }
}

std::shared_ptr<ISoundBuffer> SDL2Display::getSoundBuffer(const std::string& name) {
    auto it = _soundBuffers.find(name);
    if (it != _soundBuffers.end()) return it->second;
    return nullptr;
}

void SDL2Display::loadMusic(const std::string& name, const std::string& path) {
    auto music = std::make_shared<SDL2Music>();
    if (music->openFromFile(path)) {
        _musicFiles[name] = music;
    }
}

std::shared_ptr<IMusic> SDL2Display::getMusic(const std::string& name) {
    auto it = _musicFiles.find(name);
    if (it != _musicFiles.end()) return it->second;
    return nullptr;
}

std::shared_ptr<ISound> SDL2Display::createSound(std::shared_ptr<ISoundBuffer> buffer) {
    auto sdlBuf = std::dynamic_pointer_cast<SDL2SoundBuffer>(buffer);
    if (!sdlBuf) return nullptr;
    return std::make_shared<SDL2Sound>(sdlBuf);
}

void SDL2Display::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    // Shaders not supported in SDL2 without OpenGL
}

void SDL2Display::setShaderUniform(const std::string& shaderName, const std::string& uniformName, float value) {
    // Not supported
}

void SDL2Display::setShaderUniform(const std::string& shaderName, const std::string& uniformName, const std::vector<float>& matrix) {
    // Not supported
}

void SDL2Display::beginRenderToTexture(const std::string& textureName) {
    if (!_renderer) return;

    SDL_Texture* target = nullptr;
    auto it = _renderTextures.find(textureName);
    if (it != _renderTextures.end()) {
        target = it->second;
    } else {
        target = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_TARGET,
                                   static_cast<int>(_windowSizeWidth),
                                   static_cast<int>(_windowSizeHeight));
        if (!target) {
            std::cerr << "Failed to create render texture: " << SDL_GetError() << std::endl;
            return;
        }
        SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
        _renderTextures[textureName] = target;
        if (_debugVisuals) {
            std::cerr << "[SDL2Display] Created render texture '" << textureName << "' (" << _windowSizeWidth << "x" << _windowSizeHeight << ")" << std::endl;
        }

        if (_renderer) {
            if (SDL_SetRenderTarget(_renderer, target) == 0) {
                SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
                SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
                SDL_RenderClear(_renderer);
                SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderTarget(_renderer, nullptr);
            }
        }


    }

    if (SDL_SetRenderTarget(_renderer, target) != 0) {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        return;
    }

    _activeRenderTarget = textureName;
}

void SDL2Display::endRenderToTexture() {
    if (!_renderer) return;

    SDL_SetRenderTarget(_renderer, nullptr);

    _activeRenderTarget.clear();
}

void SDL2Display::drawRenderTexture(const std::string& textureName, const std::string& shaderName) {
    (void)shaderName;
    if (!_renderer) return;
    auto it = _renderTextures.find(textureName);
    if (it == _renderTextures.end() || !it->second) return;

    SDL_Texture* tex = it->second;
    SDL_Rect dst{0, 0, static_cast<int>(_windowSizeWidth), static_cast<int>(_windowSizeHeight)};

    SDL_RenderCopy(_renderer, tex, nullptr, &dst);
}

bool SDL2Display::isJoystickConnected(unsigned int joystickId) const {
    return SDL_JoystickGetAttached(SDL_JoystickOpen(joystickId)) == SDL_TRUE;
}

unsigned int SDL2Display::getJoystickCount() const {
    return SDL_NumJoysticks();
}

Key SDL2Display::translateKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_a: return Key::A;
        case SDLK_b: return Key::B;
        case SDLK_c: return Key::C;
        case SDLK_d: return Key::D;
        case SDLK_e: return Key::E;
        case SDLK_f: return Key::F;
        case SDLK_g: return Key::G;
        case SDLK_h: return Key::H;
        case SDLK_i: return Key::I;
        case SDLK_j: return Key::J;
        case SDLK_k: return Key::K;
        case SDLK_l: return Key::L;
        case SDLK_m: return Key::M;
        case SDLK_n: return Key::N;
        case SDLK_o: return Key::O;
        case SDLK_p: return Key::P;
        case SDLK_q: return Key::Q;
        case SDLK_r: return Key::R;
        case SDLK_s: return Key::S;
        case SDLK_t: return Key::T;
        case SDLK_u: return Key::U;
        case SDLK_v: return Key::V;
        case SDLK_w: return Key::W;
        case SDLK_x: return Key::X;
        case SDLK_y: return Key::Y;
        case SDLK_z: return Key::Z;
        case SDLK_0: return Key::Num0;
        case SDLK_1: return Key::Num1;
        case SDLK_2: return Key::Num2;
        case SDLK_3: return Key::Num3;
        case SDLK_4: return Key::Num4;
        case SDLK_5: return Key::Num5;
        case SDLK_6: return Key::Num6;
        case SDLK_7: return Key::Num7;
        case SDLK_8: return Key::Num8;
        case SDLK_9: return Key::Num9;
        case SDLK_ESCAPE: return Key::Escape;
        case SDLK_SPACE: return Key::Space;
        case SDLK_RETURN: return Key::Return;
        case SDLK_UP: return Key::Up;
        case SDLK_DOWN: return Key::Down;
        case SDLK_LEFT: return Key::Left;
        case SDLK_RIGHT: return Key::Right;
        case SDLK_BACKSPACE: return Key::BackSpace;
        case SDLK_DELETE: return Key::Delete;
        case SDLK_TAB: return Key::Tab;
        case SDLK_PAGEUP: return Key::PageUp;
        case SDLK_PAGEDOWN: return Key::PageDown;
        case SDLK_END: return Key::End;
        case SDLK_HOME: return Key::Home;
        case SDLK_INSERT: return Key::Insert;
        case SDLK_LCTRL: return Key::LControl;
        case SDLK_RCTRL: return Key::RControl;
        case SDLK_LSHIFT: return Key::LShift;
        case SDLK_RSHIFT: return Key::RShift;
        case SDLK_LALT: return Key::LAlt;
        case SDLK_RALT: return Key::RAlt;
        case SDLK_LGUI: return Key::LSystem;
        case SDLK_RGUI: return Key::RSystem;
        case SDLK_MENU: return Key::Menu;
        case SDLK_LEFTBRACKET: return Key::LBracket;
        case SDLK_RIGHTBRACKET: return Key::RBracket;
        case SDLK_SEMICOLON: return Key::SemiColon;
        case SDLK_COMMA: return Key::Comma;
        case SDLK_PERIOD: return Key::Period;
        case SDLK_QUOTE: return Key::Quote;
        case SDLK_SLASH: return Key::Slash;
        case SDLK_BACKSLASH: return Key::BackSlash;
        case SDLK_BACKQUOTE: return Key::Tilde;
        case SDLK_EQUALS: return Key::Equal;
        case SDLK_MINUS: return Key::Dash;
        case SDLK_KP_PLUS: return Key::Add;
        case SDLK_KP_MINUS: return Key::Subtract;
        case SDLK_KP_MULTIPLY: return Key::Multiply;
        case SDLK_KP_DIVIDE: return Key::Divide;
        case SDLK_KP_0: return Key::Numpad0;
        case SDLK_KP_1: return Key::Numpad1;
        case SDLK_KP_2: return Key::Numpad2;
        case SDLK_KP_3: return Key::Numpad3;
        case SDLK_KP_4: return Key::Numpad4;
        case SDLK_KP_5: return Key::Numpad5;
        case SDLK_KP_6: return Key::Numpad6;
        case SDLK_KP_7: return Key::Numpad7;
        case SDLK_KP_8: return Key::Numpad8;
        case SDLK_KP_9: return Key::Numpad9;
        case SDLK_F1: return Key::F1;
        case SDLK_F2: return Key::F2;
        case SDLK_F3: return Key::F3;
        case SDLK_F4: return Key::F4;
        case SDLK_F5: return Key::F5;
        case SDLK_F6: return Key::F6;
        case SDLK_F7: return Key::F7;
        case SDLK_F8: return Key::F8;
        case SDLK_F9: return Key::F9;
        case SDLK_F10: return Key::F10;
        case SDLK_F11: return Key::F11;
        case SDLK_F12: return Key::F12;
        case SDLK_F13: return Key::F13;
        case SDLK_F14: return Key::F14;
        case SDLK_F15: return Key::F15;
        case SDLK_PAUSE: return Key::Pause;
        default: return Key::Unknown;
    }
}

MouseButton SDL2Display::translateMouseButton(Uint8 button) {
    switch (button) {
        case SDL_BUTTON_LEFT: return MouseButton::Left;
        case SDL_BUTTON_RIGHT: return MouseButton::Right;
        case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
        default: return MouseButton::Left;
    }
}

void SDL2Display::setClipboardText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

std::string SDL2Display::getClipboardText() const {
    char* text = SDL_GetClipboardText();
    std::string result;
    if (text) {
        result = text;
        SDL_free(text);
    }
    return result;
}

}  // namespace rtype::display
