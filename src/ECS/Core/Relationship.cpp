/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Relationship
*/

#include "Relationship.hpp"
#include <algorithm>
#include <mutex>

namespace ECS {

    bool RelationshipManager::set_parent(Entity child, Entity parent) {
        if (child == parent) return false;

        std::unique_lock lock(relationship_mutex);

        if (would_create_cycle(child, parent)) {
            return false;
        }

        auto old_parent_it = parent_map.find(child.index());
        if (old_parent_it != parent_map.end()) {
            auto& old_children = children_map[old_parent_it->second.index()];
            old_children.erase(child.index());
            if (old_children.empty()) {
                children_map.erase(old_parent_it->second.index());
            }
        }

        parent_map[child.index()] = parent;
        children_map[parent.index()][child.index()] = child;

        return true;
    }

    void RelationshipManager::remove_parent(Entity child) {
        std::unique_lock lock(relationship_mutex);

        auto it = parent_map.find(child.index());
        if (it != parent_map.end()) {
            auto parent = it->second;
            parent_map.erase(it);

            auto& children = children_map[parent.index()];
            children.erase(child.index());
            if (children.empty()) {
                children_map.erase(parent.index());
            }
        }
    }

    std::optional<Entity> RelationshipManager::get_parent(Entity child) const {
        std::shared_lock lock(relationship_mutex);
        auto it = parent_map.find(child.index());
        if (it != parent_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool RelationshipManager::has_parent(Entity child) const {
        std::shared_lock lock(relationship_mutex);
        return parent_map.find(child.index()) != parent_map.end();
    }

    std::vector<Entity> RelationshipManager::get_children(Entity parent) const {
        std::shared_lock lock(relationship_mutex);
        std::vector<Entity> result;

        auto it = children_map.find(parent.index());
        if (it != children_map.end()) {
            result.reserve(it->second.size());
            for (const auto& [child_idx, child_entity] : it->second) {
                result.push_back(child_entity);
            }
        }

        return result;
    }

    std::vector<Entity> RelationshipManager::get_descendants(Entity parent) const {
        std::shared_lock lock(relationship_mutex);
        std::vector<Entity> result;
        get_descendants_recursive(parent, &result);
        return result;
    }

    std::vector<Entity> RelationshipManager::get_ancestors(Entity child) const {
        std::shared_lock lock(relationship_mutex);
        std::vector<Entity> result;

        auto current = child;
        while (true) {
            auto it = parent_map.find(current.index());
            if (it == parent_map.end()) break;

            result.push_back(it->second);
            current = it->second;
        }

        return result;
    }

    Entity RelationshipManager::get_root(Entity entity) const {
        std::shared_lock lock(relationship_mutex);

        auto current = entity;
        while (true) {
            auto it = parent_map.find(current.index());
            if (it == parent_map.end()) break;
            current = it->second;
        }

        return current;
    }

    bool RelationshipManager::is_ancestor(Entity potential_ancestor, Entity entity) const {
        std::shared_lock lock(relationship_mutex);

        auto current = entity;
        while (true) {
            auto it = parent_map.find(current.index());
            if (it == parent_map.end()) return false;

            if (it->second == potential_ancestor) return true;
            current = it->second;
        }
    }

    void RelationshipManager::remove_entity(Entity entity) {
        std::unique_lock lock(relationship_mutex);

        auto parent_it = parent_map.find(entity.index());
        if (parent_it != parent_map.end()) {
            auto parent = parent_it->second;
            parent_map.erase(parent_it);

            auto& children = children_map[parent.index()];
            children.erase(entity.index());
            if (children.empty()) {
                children_map.erase(parent.index());
            }
        }

        auto children_it = children_map.find(entity.index());
        if (children_it != children_map.end()) {
            for (const auto& [child_idx, child_entity] : children_it->second) {
                parent_map.erase(child_idx);
            }
            children_map.erase(children_it);
        }
    }

    void RelationshipManager::clear() {
        std::unique_lock lock(relationship_mutex);
        parent_map.clear();
        children_map.clear();
    }

    size_t RelationshipManager::child_count(Entity parent) const {
        std::shared_lock lock(relationship_mutex);
        auto it = children_map.find(parent.index());
        return (it != children_map.end()) ? it->second.size() : 0;
    }

    size_t RelationshipManager::get_depth(Entity entity) const {
        std::shared_lock lock(relationship_mutex);

        size_t depth = 0;
        auto current = entity;

        while (true) {
            auto it = parent_map.find(current.index());
            if (it == parent_map.end()) break;
            depth++;
            current = it->second;
        }

        return depth;
    }

    bool RelationshipManager::would_create_cycle(Entity child, Entity parent) const {
        auto current = parent;
        while (true) {
            if (current == child) return true;

            auto it = parent_map.find(current.index());
            if (it == parent_map.end()) break;
            current = it->second;
        }

        return false;
    }

    void RelationshipManager::get_descendants_recursive(Entity parent, std::vector<Entity>* result) const {
        auto it = children_map.find(parent.index());
        if (it == children_map.end()) return;

        for (const auto& [child_idx, child_entity] : it->second) {
            result->push_back(child_entity);
            get_descendants_recursive(child_entity, result);
        }
    }

} // namespace ECS
