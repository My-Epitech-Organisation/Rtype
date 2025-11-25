/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS - Main include header
*/

#ifndef ECS_HPP
    #define ECS_HPP

#include "Core/Entity.hpp"
#include "Storage/ISparseSet.hpp"
#include "Storage/SparseSet.hpp"
#include "Storage/TagSparseSet.hpp"
#include "Traits/ComponentTraits.hpp"
#include "Signal/SignalDispatcher.hpp"
#include "View/View.hpp"
#include "View/ParallelView.hpp"
#include "View/Group.hpp"
#include "View/ExcludeView.hpp"
#include "Core/Relationship.hpp"
#include "Core/Prefab.hpp"
#include "Core/Registry.hpp"
#include "Core/CommandBuffer.hpp"
#include "System/SystemScheduler.hpp"
#include "Utils/Benchmark.hpp"
#include "Serialization/Serialization.hpp"

/**
 * @namespace ECS
 * @brief High-performance Entity Component System implementation.
 *
 * Architecture Overview:
 * - Entities: Lightweight IDs with generational indices
 * - Components: Plain data structures stored in _sparse sets
 * - Systems: Functions that operate on component views
 * - Registry: Central coordinator for all ECS operations
 *
 * Key Features:
 * - Cache-friendly _sparse set storage
 * - Zero-cost tag components
 * - Parallel iteration support
 * - Signal/observer pattern
 * - Singleton resources
 * - Cached entity groups
 *
 * Performance Characteristics:
 * - Entity creation: O(1)
 * - Component add/remove: O(1)
 * - Component lookup: O(1)
 * - View iteration: O(n) with optimal cache usage
 * - Parallel iteration: Near-linear speedup
 *
 * Usage:
 *   ECS::Registry registry;
 *   auto entity = registry.spawnEntity();
 *   registry.emplaceComponent<Position>(entity, 0.0f, 0.0f);
 *
 *   registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
 *       p.x += v.dx;
 *   });
 */

#endif // ECS_HPP
