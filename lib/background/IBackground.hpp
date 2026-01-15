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
    /**
     * @brief Creates the entities that make up the background.
     * 
     * This function is responsible for initializing and spawning all ECS entities
     * required for the background (e.g., stars, planets, nebulas).
     */
    virtual void createEntitiesBackground() = 0;

    /**
     * @brief Unloads and destroys the background entities.
     * 
     * This function should remove all entities created by createEntitiesBackground()
     * from the ECS registry to clean up the level.
     */
    virtual void unloadEntitiesBackground() = 0;

    /**
     * @brief Retrieves the name of the background.
     * 
     * @return The name of the background as a string.
     */
    virtual std::string getBackgroundName() = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~IBackground() = default;
};

#endif //R_TYPE_IBACKGROUND_HPP