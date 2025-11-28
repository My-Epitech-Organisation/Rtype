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
    auto Registry::setSingleton(Args&&... args) -> T& {
        auto type = std::type_index(typeid(T));
        _singletons[type] = std::make_any<T>(std::forward<Args>(args)...);
        return std::any_cast<T&>(_singletons[type]);
    }

    template<typename T>
    auto Registry::getSingleton() -> T& {
        return std::any_cast<T&>(_singletons.at(std::type_index(typeid(T))));
    }

    template<typename T>
    auto Registry::hasSingleton() const noexcept -> bool {
        return _singletons.find(std::type_index(typeid(T))) != _singletons.end();
    }

    template<typename T>
    void Registry::removeSingleton() noexcept {
        _singletons.erase(std::type_index(typeid(T)));
    }

#endif // ECS_CORE_REGISTRY_SINGLETON_INL
