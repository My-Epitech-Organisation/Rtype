/*
** EPITECH PROJECT, 2025
** Rtype [WSL: FedoraLinux-42]
** File description:
** graphic
*/

#include "graphic.hpp"

Graphic::Graphic()
{
    InitWindow(800, 450, "Raylib [core] example - basic window");
    SetTargetFPS(60);
    _texture = LoadTexture("assets/character.png");
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
    UnloadTexture(_textures);
    CloseWindow();
}