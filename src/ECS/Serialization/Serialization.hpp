/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Serialization - Save/load ECS state
*/

#ifndef ECS_SERIALIZATION_SERIALIZATION_HPP
    #define ECS_SERIALIZATION_SERIALIZATION_HPP
    #include "../Core/Entity.hpp"
    #include <string>
    #include <vector>
    #include <unordered_map>
    #include <typeindex>
    #include <functional>
    #include <fstream>
    #include <sstream>
    #include <memory>
    #include <utility>

namespace ECS {

    class Registry;

    /**
     * @brief Component serialization interface.
     *
     * Users must implement _serializers for each component type they want to save/load.
     */
    class IComponentSerializer {
    public:
        virtual ~IComponentSerializer() = default;

        /**}
         * @brief Serializes a component to string format.
         * @param entity Entity owning the component
         * @param registry Reference to registry
         * @return Serialized component data
         */
        virtual std::string serialize(Entity entity, Registry* registry) const = 0;

        /**
         * @brief Deserializes and attaches component to entity.
         * @param entity Target entity
         * @param data Serialized component data
         * @param registry Reference to registry
         */
        virtual void deserialize(Entity entity, const std::string& data, Registry* registry) const = 0;
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
        explicit Serializer(Registry* reg) : registry(reg) {}

        /**
         * @brief Registers a component serializer.
         */
        template<typename T>
        void registerSerializer(std::shared_ptr<IComponentSerializer> serializer) {
            _serializers[std::type_index(typeid(T))] = std::move(serializer);
            _typeNames[std::type_index(typeid(T))] = typeid(T).name();
        }

        /**
         * @brief Saves current ECS state to file.
         * @param filename Output file path
         * @return true if successful
         */
        bool saveToFile(const std::string& filename);

        /**
         * @brief Loads ECS state from file.
         * @param filename Input file path
         * @param clear_existing If true, clears registry before loading
         * @return true if successful
         */
        bool loadFromFile(const std::string& filename, bool clear_existing = true);

        /**
         * @brief Serializes ECS state to string.
         */
        std::string serialize();

        /**
         * @brief Deserializes ECS state from string.
         */
        bool deserialize(const std::string& data, bool clear_existing = true);

    private:
        Registry* registry;
        std::unordered_map<std::type_index, std::shared_ptr<IComponentSerializer>> _serializers;
        std::unordered_map<std::type_index, std::string> _typeNames;
        std::unordered_map<std::string, std::type_index> _nameToType;
    };

    /**
     * @brief Helper template for simple component _serializers.
     */
    template<typename T>
    class ComponentSerializer : public IComponentSerializer {
    public:
        using SerializeFunc = std::function<std::string(const T&)>;
        using DeserializeFunc = std::function<T(const std::string&)>;

        ComponentSerializer(SerializeFunc ser, DeserializeFunc deser)
            : serialize_func(std::move(ser)), deserialize_func(std::move(deser)) {}

        std::string serialize(Entity entity, Registry* registry) const override;
        void deserialize(Entity entity, const std::string& data, Registry* registry) const override;

    private:
        SerializeFunc serialize_func;
        DeserializeFunc deserialize_func;
    };

} // namespace ECS

// Template implementations (after Registry is fully defined)
#include "../Core/Registry.hpp"

namespace ECS {
    template<typename T>
    std::string ComponentSerializer<T>::serialize(Entity entity, Registry* registry) const {
        const T& component = registry->template getComponent<T>(entity);
        return serialize_func(component);
    }

    template<typename T>
    void ComponentSerializer<T>::deserialize(Entity entity, const std::string& data, Registry* registry) const {
        T component = deserialize_func(data);
        registry->template emplaceComponent<T>(entity, std::move(component));
    }
}

#endif // ECS_SERIALIZATION_SERIALIZATION_HPP
