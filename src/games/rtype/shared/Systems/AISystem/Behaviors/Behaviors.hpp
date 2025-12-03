/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Behaviors - Aggregate header for all AI behaviors
*/

#pragma once

// Interface
#include "IAIBehavior.hpp"

// Registry
#include "BehaviorRegistry.hpp"

// Built-in behaviors
#include "Chase/ChaseBehavior.hpp"
#include "MoveLeft/MoveLeftBehavior.hpp"
#include "Patrol/PatrolBehavior.hpp"
#include "SineWave/SineWaveBehavior.hpp"
#include "Stationary/StationaryBehavior.hpp"

/**
 * @file Behaviors.hpp
 * @brief Include this file to get access to all AI behaviors
 *
 * To add a new behavior:
 * 1. Create YourBehavior.hpp and YourBehavior.cpp in this folder
 * 2. Implement the IAIBehavior interface
 * 3. Add the include here
 * 4. Register it in BehaviorRegistry.cpp's registerDefaultBehaviors()
 * 5. Add the enum value in AIComponent.hpp's AIBehavior enum
 */
