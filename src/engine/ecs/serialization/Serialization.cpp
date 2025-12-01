/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Serialization implementation
*/

#include "Serialization.hpp"

#include "../core/Registry/Registry.hpp"

namespace ECS {

auto Serializer::saveToFile(const std::string& filename) -> bool {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string data = serialize();
    file << data;
    file.close();
    return true;
}

auto Serializer::loadFromFile(const std::string& filename, bool clearExisting)
    -> bool {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return deserialize(buffer.str(), clearExisting);
}

auto Serializer::serialize() -> std::string {
    std::ostringstream oss;

    // TODO(SamTess): To implement serialization logic:
    // 1. Iterate through all entities
    // 2. For each entity, get its components
    // 3. Serialize each component using registered _serializers
    // 4. Write to the output stream
    (void)registry;  // Will be used when fully implemented

    oss << "ECS_SAVE_V1\n";
    oss << "# Format: ENTITY <index> <generation>\n";
    oss << "# Format: COMPONENT <type> <data>\n";

    // This is a basic implementation
    // For full implementation, you would need Registry to expose entity
    // iteration
    oss << "END\n";

    return oss.str();
}

auto Serializer::deserialize(const std::string& data, bool clearExisting)
    -> bool {
    std::istringstream iss(data);
    std::string line;
    (void)registry;       // Will be used when fully implemented
    (void)clearExisting;  // To avoid unused parameter warning

    if (!std::getline(iss, line) || line != "ECS_SAVE_V1") {
        return false;
    }
    // TODO(SamTess): Implement deserialization logic
    while (std::getline(iss, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "END") {
            break;
        }
    }

    return true;
}

}  // namespace ECS
