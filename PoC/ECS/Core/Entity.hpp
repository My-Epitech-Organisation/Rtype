/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Entity
*/

#ifndef ECS_CORE_ENTITY_HPP
    #define ECS_CORE_ENTITY_HPP
    #include <cstdint>
    #include <limits>
    #include <compare>
    #include <functional>

namespace ECS {

    /**
     * @brief Type-safe entity identifier using generational indices.
     *
     * Layout: 32-bit _packed structure
     * - [19:0]  Index (20 bits)     - Entity slot position
     * - [31:20] Generation (12 bits) - Version counter
     *
     * Generational indices prevent ABA problems where entity IDs are recycled.
     * When an entity is destroyed, its generation increments, invalidating old handles.
     */
    struct Entity {
        std::uint32_t id = _NullID;

        static constexpr std::uint32_t _IndexBits = 20;
        static constexpr std::uint32_t _IndexMask = (1 << _IndexBits) - 1;
        static constexpr std::uint32_t _GenerationBits = 12;
        static constexpr std::uint32_t _GenerationMask = (1 << _GenerationBits) - 1;
        static constexpr std::uint32_t _MaxGeneration = _GenerationMask;
        static constexpr std::uint32_t _NullID = std::numeric_limits<std::uint32_t>::max();

        constexpr Entity() = default;
        constexpr explicit Entity(std::uint32_t raw) : id(raw) {}
        constexpr Entity(std::uint32_t index, std::uint32_t generation) {
            id = (index & _IndexMask) | ((generation & _GenerationMask) << _IndexBits);
        }

        [[nodiscard]] constexpr std::uint32_t index() const noexcept { return id & _IndexMask; }
        [[nodiscard]] constexpr std::uint32_t generation() const noexcept { return (id >> _IndexBits) & _GenerationMask; }
        [[nodiscard]] constexpr bool isNull() const noexcept { return id == _NullID; }
        [[nodiscard]] constexpr bool isTombstone() const noexcept { return generation() == _MaxGeneration; }

        auto operator<=>(const Entity&) const noexcept = default;
    };

} // namespace ECS

namespace std {
    template<>
    struct hash<ECS::Entity> {
        std::size_t operator()(const ECS::Entity& entity) const noexcept {
            return hash<uint32_t>{}(entity.id);
        }
    };
}

#endif // ECS_CORE_ENTITY_HPP
