/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SDL2 implementation of Snake renderer - Implementation
*/

#include "SDL2SnakeRenderer.hpp"

#include <iostream>

namespace rtype::games::snake::client::sdl2 {

SDL2SnakeRenderer::SDL2SnakeRenderer(int width, int height, int cellSize)
    : width_(width), height_(height), cellSize_(cellSize) {}

SDL2SnakeRenderer::~SDL2SnakeRenderer() {
    if (pixels_) {
        delete[] pixels_;
    }
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool SDL2SnakeRenderer::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError()
                  << std::endl;
        return false;
    }

    window_ = SDL_CreateWindow("Snake Game - SDL2", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, width_, height_,
                               SDL_WINDOW_SHOWN);
    if (!window_) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "Renderer creation failed: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    pixels_ = new uint32_t[width_ * height_];
    std::fill(pixels_, pixels_ + (width_ * height_), 0xFF000000);

    return true;
}

bool SDL2SnakeRenderer::processInput(Direction& inputDirection) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                shouldClose_ = true;
                return false;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        inputDirection = Direction::UP;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        inputDirection = Direction::DOWN;
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        inputDirection = Direction::LEFT;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        inputDirection = Direction::RIGHT;
                        break;
                    case SDLK_ESCAPE:
                        shouldClose_ = true;
                        return false;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
    return true;
}

void SDL2SnakeRenderer::beginFrame() {
    std::fill(pixels_, pixels_ + (width_ * height_), 0xFF000000);
}

void SDL2SnakeRenderer::endFrame() {
    SDL_UpdateTexture(texture_, nullptr, pixels_, width_ * sizeof(uint32_t));

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

void SDL2SnakeRenderer::drawCell(int gridX, int gridY, uint32_t color) {
    int pixelX = gridX * cellSize_;
    int pixelY = gridY * cellSize_;

    uint32_t sdlColor = rgbToSDL(color);

    for (int dy = 0; dy < cellSize_; dy++) {
        for (int dx = 0; dx < cellSize_; dx++) {
            int px = pixelX + dx;
            int py = pixelY + dy;
            if (px >= 0 && px < width_ && py >= 0 && py < height_) {
                pixels_[py * width_ + px] = sdlColor;
            }
        }
    }
}

uint32_t SDL2SnakeRenderer::rgbToSDL(uint32_t rgb) const {
    return 0xFF000000 | rgb;
}

void SDL2SnakeRenderer::renderSnake(const std::vector<SnakeSegment>& segments,
                                    uint32_t headColor, uint32_t bodyColor) {
    if (segments.empty()) {
        return;
    }

    drawCell(segments[0].gridX, segments[0].gridY, headColor);

    for (size_t i = 1; i < segments.size(); i++) {
        drawCell(segments[i].gridX, segments[i].gridY, bodyColor);
    }
}

void SDL2SnakeRenderer::renderFood(int gridX, int gridY, uint32_t color) {
    drawCell(gridX, gridY, color);
}

void SDL2SnakeRenderer::renderScore(int score, int x, int y) {
    std::string scoreStr = "Score: " + std::to_string(score);
    renderText(scoreStr, x, y, 0xFFFFFF);
}

void SDL2SnakeRenderer::renderGameOver(int finalScore) {
    std::cout << "GAME OVER! Final Score: " << finalScore << std::endl;
}

void SDL2SnakeRenderer::drawGrid(int gridWidth, int gridHeight,
                                 uint32_t gridColor) {
    uint32_t sdlColor = rgbToSDL(gridColor);

    for (int x = 0; x <= gridWidth; x++) {
        int pixelX = x * cellSize_;
        if (pixelX >= width_) break;
        for (int dy = 0; dy < height_; dy++) {
            if (pixelX < width_) {
                pixels_[dy * width_ + pixelX] = sdlColor;
            }
        }
    }

    for (int y = 0; y <= gridHeight; y++) {
        int pixelY = y * cellSize_;
        if (pixelY >= height_) break;
        for (int dx = 0; dx < width_; dx++) {
            pixels_[pixelY * width_ + dx] = sdlColor;
        }
    }
}

void SDL2SnakeRenderer::renderText(const std::string& text, int x, int y,
                                   uint32_t color) {
    uint32_t sdlColor = rgbToSDL(color);
    int charX = x;
    const int char_width = 25;
    const int digit_height = 20;

    auto drawRect = [this, sdlColor](int px, int py, int w, int h) {
        for (int dy = 0; dy < h; dy++) {
            for (int dx = 0; dx < w; dx++) {
                int screenX = px + dx;
                int screenY = py + dy;
                if (screenX >= 0 && screenX < width_ && screenY >= 0 &&
                    screenY < height_) {
                    pixels_[screenY * width_ + screenX] = sdlColor;
                }
            }
        }
    };

    for (char c : text) {
        if (c >= '0' && c <= '9') {
            int digit = c - '0';
            int seg_w = 8;
            int seg_h = 3;
            int vert_w = 3;
            int vert_h = 5;
            int gap = 2;

            int cx = charX;
            int cy = y;

            if (digit != 1 && digit != 4) {
                drawRect(cx, cy, seg_w, seg_h);
            }

            if (digit != 5 && digit != 6) {
                drawRect(cx + seg_w, cy + gap, vert_w, vert_h);
            }

            if (digit != 2) {
                drawRect(cx + seg_w, cy + gap + vert_h + gap, vert_w, vert_h);
            }

            if (digit != 1 && digit != 4 && digit != 7) {
                drawRect(cx, cy + gap + vert_h + gap + vert_h, seg_w, seg_h);
            }

            if (digit == 0 || digit == 2 || digit == 6 || digit == 8 ||
                digit == 9) {
                drawRect(cx - vert_w, cy + gap + vert_h + gap, vert_w, vert_h);
            }

            if (digit == 0 || digit == 4 || digit == 5 || digit == 6 ||
                digit == 8 || digit == 9) {
                drawRect(cx - vert_w, cy + gap, vert_w, vert_h);
            }

            if (digit == 2 || digit == 3 || digit == 4 || digit == 5 ||
                digit == 6 || digit == 8 || digit == 9) {
                drawRect(cx, cy + gap + vert_h, seg_w, seg_h);
            }
        } else if (c == ':' || c == ' ') {
        } else {
            switch (c) {
                case 'S': {
                    drawRect(charX, y, 6, 3);
                }
                case 'c': {
                    drawRect(charX + 2, y, 4, 12);
                }
                case 'o': {
                    drawRect(charX + 1, y, 4, 12);
                }
                case 'r': {
                    drawRect(charX, y, 3, 12);
                }
                case 'e': {
                    drawRect(charX + 1, y, 4, 12);
                }
            }
        }
        charX += char_width;
    }
}

}  // namespace rtype::games::snake::client::sdl2
