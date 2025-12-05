/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS - Main include header
*/

#ifndef SRC_ENGINE_ECS_ECS_HPP_
#define SRC_ENGINE_ECS_ECS_HPP_

#include "core/CommandBuffer.hpp"
#include "core/Entity.hpp"
#include "core/Prefab.hpp"
#include "core/Registry/Registry.hpp"
#include "core/Relationship.hpp"
#include "serialization/Serialization.hpp"
#include "signal/SignalDispatcher.hpp"
#include "storage/ISparseSet.hpp"
#include "storage/SparseSet.hpp"
#include "system/SystemScheduler.hpp"
#include "traits/ComponentTraits.hpp"
#include "view/ExcludeView.hpp"
#include "view/Group.hpp"
#include "view/ParallelView.hpp"
#include "view/View.hpp"

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

#endif  // SRC_ENGINE_ECS_ECS_HPP_
