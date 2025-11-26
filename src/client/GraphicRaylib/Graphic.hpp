/*
** EPITECH PROJECT, 2025
** Rtype [WSL: FedoraLinux-42]
** File description:
** graphic
*/

#ifndef SRC_CLIENT_GRAPHICRAYLIB_GRAPHIC_HPP_
#define SRC_CLIENT_GRAPHICRAYLIB_GRAPHIC_HPP_

#include "raylib.h"

class Graphic {
   private:
    Image _image;
    Texture2D _texture;

   public:
    void loop();
    Graphic();
    ~Graphic();
};

#endif  // SRC_CLIENT_GRAPHICRAYLIB_GRAPHIC_HPP_
