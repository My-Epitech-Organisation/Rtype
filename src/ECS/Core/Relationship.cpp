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

    bool RelationshipManager::setParent(Entity child, Entity parent) {
        if (child == parent) return false;

        std::unique_lock lock(_relationshipMutex);

        if (wouldCreateCycle(child, parent)) {
            return false;
        }

        auto old_parent_it = _parentMap.find(child.index());
        if (old_parent_it != _parentMap.end()) {
            auto& old_children = _childrenMap[old_parent_it->second.index()];
            old_children.erase(child.index());
            if (old_children.empty()) {
                _childrenMap.erase(old_parent_it->second.index());
            }
        }

        _parentMap[child.index()] = parent;
        _childrenMap[parent.index()][child.index()] = child;

        return true;
    }

    void RelationshipManager::removeParent(Entity child) {
        std::unique_lock lock(_relationshipMutex);

        auto it = _parentMap.find(child.index());
        if (it != _parentMap.end()) {
            auto parent = it->second;
            _parentMap.erase(it);

            auto& children = _childrenMap[parent.index()];
            children.erase(child.index());
            if (children.empty()) {
                _childrenMap.erase(parent.index());
            }
        }
    }

    std::optional<Entity> RelationshipManager::getParent(Entity child) const {
        std::shared_lock lock(_relationshipMutex);
        auto it = _parentMap.find(child.index());
        if (it != _parentMap.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool RelationshipManager::hasParent(Entity child) const {
        std::shared_lock lock(_relationshipMutex);
        return _parentMap.find(child.index()) != _parentMap.end();
    }

    std::vector<Entity> RelationshipManager::getChildren(Entity parent) const {
        std::shared_lock lock(_relationshipMutex);
        std::vector<Entity> result;

        auto it = _childrenMap.find(parent.index());
        if (it != _childrenMap.end()) {
            result.reserve(it->second.size());
            for (const auto& [child_idx, child_entity] : it->second) {
                result.push_back(child_entity);
            }
        }

        return result;
    }

    std::vector<Entity> RelationshipManager::getDescendants(Entity parent) const {
        std::shared_lock lock(_relationshipMutex);
        std::vector<Entity> result;
        getDescendantsRecursive(parent, &result);
        return result;
    }

    std::vector<Entity> RelationshipManager::getAncestors(Entity child) const {
        std::shared_lock lock(_relationshipMutex);
        std::vector<Entity> result;

        auto current = child;
        while (true) {
            auto it = _parentMap.find(current.index());
            if (it == _parentMap.end()) break;

            result.push_back(it->second);
            current = it->second;
        }

        return result;
    }

    Entity RelationshipManager::getRoot(Entity entity) const {
        std::shared_lock lock(_relationshipMutex);

        auto current = entity;
        while (true) {
            auto it = _parentMap.find(current.index());
            if (it == _parentMap.end()) break;
            current = it->second;
        }

        return current;
    }

    bool RelationshipManager::isAncestor(Entity potential_ancestor, Entity entity) const {
        std::shared_lock lock(_relationshipMutex);

        auto current = entity;
        while (true) {
            auto it = _parentMap.find(current.index());
            if (it == _parentMap.end()) return false;

            if (it->second == potential_ancestor) return true;
            current = it->second;
        }
    }

    void RelationshipManager::removeEntity(Entity entity) {
        std::unique_lock lock(_relationshipMutex);

        auto parent_it = _parentMap.find(entity.index());
        if (parent_it != _parentMap.end()) {
            auto parent = parent_it->second;
            _parentMap.erase(parent_it);

            auto& children = _childrenMap[parent.index()];
            children.erase(entity.index());
            if (children.empty()) {
                _childrenMap.erase(parent.index());
            }
        }

        auto children_it = _childrenMap.find(entity.index());
        if (children_it != _childrenMap.end()) {
            for (const auto& [child_idx, child_entity] : children_it->second) {
                _parentMap.erase(child_idx);
            }
            _childrenMap.erase(children_it);
        }
    }

    void RelationshipManager::clear() {
        std::unique_lock lock(_relationshipMutex);
        _parentMap.clear();
        _childrenMap.clear();
    }

    size_t RelationshipManager::childCount(Entity parent) const {
        std::shared_lock lock(_relationshipMutex);
        auto it = _childrenMap.find(parent.index());
        return (it != _childrenMap.end()) ? it->second.size() : 0;
    }

    size_t RelationshipManager::getDepth(Entity entity) const {
        std::shared_lock lock(_relationshipMutex);

        size_t depth = 0;
        auto current = entity;

        while (true) {
            auto it = _parentMap.find(current.index());
            if (it == _parentMap.end()) break;
            depth++;
            current = it->second;
        }

        return depth;
    }

    bool RelationshipManager::wouldCreateCycle(Entity child, Entity parent) const {
        auto current = parent;
        while (true) {
            if (current == child) return true;

            auto it = _parentMap.find(current.index());
            if (it == _parentMap.end()) break;
            current = it->second;
        }

        return false;
    }

    void RelationshipManager::getDescendantsRecursive(Entity parent, std::vector<Entity>* result) const {
        auto it = _childrenMap.find(parent.index());
        if (it == _childrenMap.end()) return;

        for (const auto& [child_idx, child_entity] : it->second) {
            result->push_back(child_entity);
            getDescendantsRecursive(child_entity, result);
        }
    }

} // namespace ECS
