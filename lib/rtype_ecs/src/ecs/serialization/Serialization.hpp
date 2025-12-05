/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Serialization - Save/load ECS state
*/

#ifndef SRC_ENGINE_ECS_SERIALIZATION_SERIALIZATION_HPP_
#define SRC_ENGINE_ECS_SERIALIZATION_SERIALIZATION_HPP_

#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../core/Entity.hpp"

namespace ECS {

class Registry;

/**
 * @brief Component serialization interface.
 *
 * Users must implement _serializers for each component type they want to
 * save/load.
 */
class IComponentSerializer {
   public:
    IComponentSerializer() = default;
    virtual ~IComponentSerializer() = default;
    IComponentSerializer(const IComponentSerializer&) = default;
    IComponentSerializer(IComponentSerializer&&) = default;
    auto operator=(const IComponentSerializer&)
        -> IComponentSerializer& = default;
    auto operator=(IComponentSerializer&&) -> IComponentSerializer& = default;

    /**
     * @brief Serializes a component to string format.
     * @param entity Entity owning the component
     * @param registry Reference to registry
     * @return Serialized component data
     */
    [[nodiscard]] virtual auto serialize(
        Entity entity, std::reference_wrapper<Registry> registry) const
        -> std::string = 0;

    /**
     * @brief Deserializes and attaches component to entity.
     * @param entity Target entity
     * @param data Serialized component data
     * @param registry Reference to registry
     */
    virtual void deserialize(
        Entity entity, const std::string& data,
        std::reference_wrapper<Registry> registry) const = 0;
};

/**
 * @brief Handles saving and loading of ECS world state.
 *
 * Features:
 * - Entity persistence with generation tracking
 * - Component serialization via registered handlers
 * - Simple text-based format
 *
 * Example:
 *   Serializer serializer(registry);
 *   serializer.registerSerializer<Position>(pos_serializer);
 *   serializer.saveToFile("save.txt");
 *   serializer.loadFromFile("save.txt");
 */
class Serializer {
   public:
    explicit Serializer(std::reference_wrapper<Registry> reg) : registry(reg) {}

    /**
     * @brief Registers a component serializer.
     */
    template <typename T>
    void registerSerializer(std::shared_ptr<IComponentSerializer> serializer) {
        _serializers[std::type_index(typeid(T))] = std::move(serializer);
        _typeNames[std::type_index(typeid(T))] = typeid(T).name();
    }

    /**
     * @brief Saves current ECS state to file.
     * @param filename Output file path
     * @return true if successful
     */
    auto saveToFile(const std::string& filename) -> bool;

    /**
     * @brief Loads ECS state from file.
     * @param filename Input file path
     * @param clearExisting If true, clears registry before loading
     * @return true if successful
     */
    auto loadFromFile(const std::string& filename, bool clearExisting = true)
        -> bool;

    /**
     * @brief Serializes ECS state to string.
     */
    [[nodiscard]] auto serialize() -> std::string;

    /**
     * @brief Deserializes ECS state from string.
     */
    auto deserialize(const std::string& data, bool clearExisting = true)
        -> bool;

   private:
    std::reference_wrapper<Registry> registry;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentSerializer>>
        _serializers;
    std::unordered_map<std::type_index, std::string> _typeNames;
    std::unordered_map<std::string, std::type_index> _nameToType;
};

/**
 * @brief Helper template for simple component _serializers.
 */
template <typename T>
class ComponentSerializer : public IComponentSerializer {
   public:
    using SerializeFunc = std::function<std::string(const T&)>;
    using DeserializeFunc = std::function<T(const std::string&)>;

    ComponentSerializer(SerializeFunc ser, DeserializeFunc deser)
        : serialize_func(std::move(ser)), deserialize_func(std::move(deser)) {}

    [[nodiscard]] auto serialize(
        Entity entity, std::reference_wrapper<Registry> registry) const
        -> std::string override;
    void deserialize(Entity entity, const std::string& data,
                     std::reference_wrapper<Registry> registry) const override;

   private:
    SerializeFunc serialize_func;
    DeserializeFunc deserialize_func;
};

}  // namespace ECS

// Template implementations (after Registry is fully defined)
#include "../core/Registry/Registry.hpp"

namespace ECS {
template <typename T>
auto ComponentSerializer<T>::serialize(
    Entity entity, std::reference_wrapper<Registry> registry) const
    -> std::string {
    const T& component = registry.get().template getComponent<T>(entity);
    return serialize_func(component);
}

template <typename T>
void ComponentSerializer<T>::deserialize(
    Entity entity, const std::string& data,
    std::reference_wrapper<Registry> registry) const {
    T component = deserialize_func(data);
    registry.get().template emplaceComponent<T>(entity, std::move(component));
}
}  // namespace ECS

#endif  // SRC_ENGINE_ECS_SERIALIZATION_SERIALIZATION_HPP_
