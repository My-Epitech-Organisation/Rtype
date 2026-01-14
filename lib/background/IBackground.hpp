/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IBackground.hpp
*/

#ifndef R_TYPE_IBACKGROUND_HPP
#define R_TYPE_IBACKGROUND_HPP
#include <vector>
#include <string>

class IBackground {
public:
    virtual void createEntitiesBackground() = 0;
    virtual void unloadEntitiesBackground() = 0;
    virtual std::string getBackgroundName() = 0;
    virtual ~IBackground() = default;
};

#endif //R_TYPE_IBACKGROUND_HPP