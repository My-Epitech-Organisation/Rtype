/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IEvent - Base interface for all events in the Event Manager system
*/

#pragma once

#include <cstdint>
#include <typeindex>

namespace rtype::engine {

/**
 * @brief Base interface for all events
 *
 * All custom events must inherit from this interface.
 * This allows type-safe event dispatching using std::type_index.
 *
 * Design principles:
 * - Polymorphic base for event type erasure
 * - Virtual destructor for proper cleanup
 * - Type identification through std::type_index
 *
 * Example:
 * @code
 * struct CollisionEvent : public IEvent {
 *     ECS::Entity entityA;
 *     ECS::Entity entityB;
 *     float impactForce;
 * };
 * @endcode
 */
class IEvent {
   public:
    virtual ~IEvent() = default;

    /**
     * @brief Get the type index of this event
     * @return Type index for runtime type identification
     */
    virtual std::type_index getType() const = 0;

   protected:
    IEvent() = default;
    IEvent(const IEvent&) = default;
    IEvent(IEvent&&) = default;
    IEvent& operator=(const IEvent&) = default;
    IEvent& operator=(IEvent&&) = default;
};

/**
 * @brief Helper macro to implement getType() for derived events
 *
 * Usage:
 * @code
 * struct MyEvent : public IEvent {
 *     IMPLEMENT_EVENT_TYPE()
 *     int data;
 * };
 * @endcode
 */
#define IMPLEMENT_EVENT_TYPE()                           \
    std::type_index getType() const override {           \
        return std::type_index(typeid(*this));           \
    }

}  // namespace rtype::engine
