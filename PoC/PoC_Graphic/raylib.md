# Récap de la documentation sur raylib

La raylib, dans le cadre de ce projet, est orientée C. Cela signifie qu'il est nécessaire de créer nos propres constructeurs et destructeurs, ce qui n'est pas toujours pratique. 

## Exemple de fonctionnement pour afficher une image

Voici un exemple de code pour afficher une image en utilisant raylib :

```C
void main()
{
    InitWindow(800, 450, "RTYPE Raylib - POC");
    SetTargetFPS(60);
    this->_image = LoadImage("assets/r-typesheet42.gif");
    this->_texture = LoadTextureFromImage(this->_image);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(_texture, 100, 50, WHITE);
        EndDrawing();
    }

    UnloadTexture(this->_texture);
    UnloadImage(this->_image);
    CloseWindow();
    return 0;
}
```