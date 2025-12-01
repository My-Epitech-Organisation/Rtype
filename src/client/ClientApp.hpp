/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.hpp
*/

#ifndef SRC_CLIENT_CLIENTAPP_HPP_
#define SRC_CLIENT_CLIENTAPP_HPP_

#include "Graphic/Graphic.hpp"

class ClientApp {
   private:
    Graphic _graphic;

   public:
    explicit ClientApp(const std::shared_ptr<ECS::Registry>& registry);
    ~ClientApp() = default;

    ClientApp(const ClientApp&) = delete;
    ClientApp& operator=(const ClientApp&) = delete;
    ClientApp(ClientApp&&) = delete;
    ClientApp& operator=(ClientApp&&) = delete;
};

#endif  // SRC_CLIENT_CLIENTAPP_HPP_
