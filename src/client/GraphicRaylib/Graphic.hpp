/*
** EPITECH PROJECT, 2025
** Rtype [WSL: FedoraLinux-42]
** File description:
** graphic
*/

#include "raylib.h"

class Graphic
{
    private:
        Image _image;
        Texture2D _texture;

    public:
        void loop();
        Graphic();
        ~Graphic();
};