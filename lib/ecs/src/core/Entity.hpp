/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Entity
*/

#ifndef SRC_ENGINE_ECS_CORE_ENTITY_HPP_
#define SRC_ENGINE_ECS_CORE_ENTITY_HPP_

#define NOMINMAX

#include <compare>
#include <cstdint>
#include <functional>
#include <limits>

namespace ECS {

/**
 * @brief Type-safe entity identifier using generational indices.
 *
 * Layout: 32-bit Packed structure
 * - [19:0]  Index (20 bits)     - Entity slot position
 * - [31:20] Generation (12 bits) - Version counter
 *
 * Generational indices prevent ABA problems where entity IDs are recycled.
 * When an entity is destroyed, its generation increments, invalidating old
 * handles.
 */
struct Entity {
    std::uint32_t id = _NullID;

    static constexpr std::uint32_t _IndexBits = 20;
    static constexpr std::uint32_t _IndexMask = (1 << _IndexBits) - 1;
    static constexpr std::uint32_t _GenerationBits = 12;
    static constexpr std::uint32_t _GenerationMask = (1 << _GenerationBits) - 1;
    static constexpr std::uint32_t _MaxGeneration = _GenerationMask;
    static constexpr std::uint32_t _NullID =
        std::numeric_limits<std::uint32_t>::max();

    constexpr Entity() = default;
    constexpr explicit Entity(std::uint32_t raw) : id(raw) {}
    constexpr Entity(std::uint32_t index, std::uint32_t generation)
        : id((index & _IndexMask) |
             ((generation & _GenerationMask) << _IndexBits)) {}

    [[nodiscard]] constexpr auto index() const noexcept -> std::uint32_t {
        return id & _IndexMask;
    }
    [[nodiscard]] constexpr auto generation() const noexcept -> std::uint32_t {
        return (id >> _IndexBits) & _GenerationMask;
    }
    [[nodiscard]] constexpr auto isNull() const noexcept -> bool {
        return id == _NullID;
    }
    [[nodiscard]] constexpr auto isTombstone() const noexcept -> bool {
        return generation() == _MaxGeneration;
    }

    auto operator<=>(const Entity&) const noexcept = default;
};

}  // namespace ECS

namespace std {
template <>
struct hash<ECS::Entity> {
    auto operator()(const ECS::Entity& entity) const noexcept -> std::size_t {
        return hash<std::uint32_t>{}(entity.id);
    }
};
}  // namespace std

#endif  // SRC_ENGINE_ECS_CORE_ENTITY_HPP_
