/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** main
*/
#include <iostream>
#include <thread>
#include <chrono>
#include "ACommand.hpp"
#include "Game.hpp"

int main()
{
    ACommand commandQueue;
    Game game;
    std::thread t1([&game, &commandQueue](){
        std::cout << "Thread Network started" << std::endl;
        for (int i = 0; i < 5; ++i) {
            commandQueue.addNewCommand("Thread 1 - Count: " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "Thread Network ended" << std::endl;
        game.setAppRunning(false);
    });
    t1.detach();
    while (!commandQueue.isEmpty() ||  game.isAppRunning()) {
        if (commandQueue.isEmpty()) {
            std::cout << "No executing commands..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        std::cout << "Graphic Thread executing commands..." << std::endl;
        commandQueue.execute(game);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}