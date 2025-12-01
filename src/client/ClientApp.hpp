/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.hpp
*/

#ifndef R_TYPE_CLIENTAPP_HPP
#define R_TYPE_CLIENTAPP_HPP

#include "Graphic/Graphic.hpp"

class ClientApp {
private:
    Graphic _graphic;
public:

    explicit ClientApp(const std::shared_ptr<ECS::Registry>& registry);
    ~ClientApp() = default;
};

#endif
