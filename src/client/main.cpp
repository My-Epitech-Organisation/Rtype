/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** main.cpp
*/
#include <iostream>
#include "./ClientApp.hpp"

int main()
{
    RTypeClient::ClientApp client;
    std::cout << "Hello" << std::endl;
    client.run();
    std::cout << "Good bye" << std::endl;
    return 0;
}