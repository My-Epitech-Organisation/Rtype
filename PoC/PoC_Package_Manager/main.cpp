/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** main
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <asio.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Package Manager Test");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    asio::io_context io_context;
    asio::steady_timer timer(io_context, std::chrono::seconds(1));

    std::cout << "SFML and Asio integration test successful!" << std::endl;
    std::cout << "SFML version: " << SFML_VERSION_MAJOR << "." << SFML_VERSION_MINOR << std::endl;
    std::cout << "Asio library loaded successfully" << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    while (window.isOpen() && std::chrono::steady_clock::now() - start_time < std::chrono::seconds(2)) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
