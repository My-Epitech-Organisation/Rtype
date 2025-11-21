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
     * Layout: 32-bit packed structure
     * - [19:0]  Index (20 bits)     - Entity slot position
     * - [31:20] Generation (12 bits) - Version counter
     *
     * Generational indices prevent ABA problems where entity IDs are recycled.
     * When an entity is destroyed, its generation increments, invalidating old handles.
     */
    struct Entity {
        std::uint32_t id = NullID;

        static constexpr std::uint32_t IndexBits = 20;
        static constexpr std::uint32_t IndexMask = (1 << IndexBits) - 1;
        static constexpr std::uint32_t GenerationBits = 12;
        static constexpr std::uint32_t GenerationMask = (1 << GenerationBits) - 1;
        static constexpr std::uint32_t MaxGeneration = GenerationMask;
        static constexpr std::uint32_t NullID = std::numeric_limits<std::uint32_t>::max();

        constexpr Entity() = default;
        constexpr explicit Entity(std::uint32_t raw) : id(raw) {}
        constexpr Entity(std::uint32_t index, std::uint32_t generation) {
            id = (index & IndexMask) | ((generation & GenerationMask) << IndexBits);
        }

        [[nodiscard]] constexpr std::uint32_t index() const noexcept { return id & IndexMask; }
        [[nodiscard]] constexpr std::uint32_t generation() const noexcept { return (id >> IndexBits) & GenerationMask; }
        [[nodiscard]] constexpr bool is_null() const noexcept { return id == NullID; }
        [[nodiscard]] constexpr bool is_tombstone() const noexcept { return generation() == MaxGeneration; }

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
