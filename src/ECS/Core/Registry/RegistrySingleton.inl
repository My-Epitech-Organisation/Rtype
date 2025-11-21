/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Singleton Management Template Implementations
*/

#ifndef ECS_CORE_REGISTRY_SINGLETON_INL
    #define ECS_CORE_REGISTRY_SINGLETON_INL

// This file is included inside the ECS namespace in Registry.hpp
// Do not add namespace ECS here!

// ========================================================================
// SINGLETON OPERATIONS
// ========================================================================

    template<typename T, typename... Args>
    T& Registry::set_singleton(Args&&... args) {
        std::type_index type = std::type_index(typeid(T));
        singletons[type] = std::make_any<T>(std::forward<Args>(args)...);
        return std::any_cast<T&>(singletons[type]);
    }

    template<typename T>
    T& Registry::get_singleton() {
        return std::any_cast<T&>(singletons.at(std::type_index(typeid(T))));
    }

    template<typename T>
    bool Registry::has_singleton() const noexcept {
        return singletons.find(std::type_index(typeid(T))) != singletons.end();
    }

    template<typename T>
    void Registry::remove_singleton() noexcept {
        singletons.erase(std::type_index(typeid(T)));
    }

#endif // ECS_CORE_REGISTRY_SINGLETON_INL
