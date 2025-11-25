/*
** EPITECH PROJECT, 2025
** Rtype [WSL: FedoraLinux-42]
** File description:
** graphic
*/

#include "graphic.hpp"

Graphic::Graphic()
{
    InitWindow(800, 450, "RTYPE Raylib - POC");
    SetTargetFPS(60);
    this->_image = LoadImage("assets/r-typesheet42.gif");
    this->_texture = LoadTextureFromImage(this->_image);
}

void Graphic::loop()
{
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(_texture, 100, 50, WHITE);
        EndDrawing();
    }
}

Graphic::~Graphic() {
    UnloadTexture(this->_texture);
    UnloadImage(this->_image);
    CloseWindow();
}