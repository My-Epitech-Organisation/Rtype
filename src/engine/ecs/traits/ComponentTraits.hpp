/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ComponentTraits
*/

#ifndef ECS_TRAITS_COMPONENT_TRAITS_HPP
    #define ECS_TRAITS_COMPONENT_TRAITS_HPP
    #include <type_traits>

namespace ECS {

    /**
     * @brief Compile-time component type analysis for storage optimization.
     *
     * Provides traits for efficient component management:
     * - Empty components (tags) skip storage allocation
     * - Trivially copyable components use fast memory operations
     * - Trivially destructible components skip destructor calls
     */
    template<typename T>
    struct ComponentTraits {
        static constexpr bool isEmpty = std::is_empty_v<T>;
        static constexpr bool isTrivial = std::is_trivially_copyable_v<T>;
        static constexpr bool isTrivialDestructible = std::is_trivially_destructible_v<T>;
    };

    /**
     * @brief Validates that a type meets minimum component requirements.
     *
     * Components must be move-constructible for efficient storage operations.
     */
    template<typename T>
    concept Component = std::is_move_constructible_v<T>;

} // namespace ECS

#endif // ECS_TRAITS_COMPONENT_TRAITS_HPP
