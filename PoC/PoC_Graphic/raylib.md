# Summary of the raylib documentation

For this project, raylib is used with C. This means we must create our own constructors and destructors, which is not always convenient.

## Example of how to display an image

Below is an example showing how to display an image using raylib:

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