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

auto RelationshipManager::setParent(Entity child, Entity parent) -> bool {
    if (child == parent) {
        return false;
    }

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

    auto iter = _parentMap.find(child.index());
    if (iter != _parentMap.end()) {
        auto parent = iter->second;
        _parentMap.erase(iter);

        auto& children = _childrenMap[parent.index()];
        children.erase(child.index());
        if (children.empty()) {
            _childrenMap.erase(parent.index());
        }
    }
}

auto RelationshipManager::getParent(Entity child) const
    -> std::optional<Entity> {
    std::shared_lock lock(_relationshipMutex);
    auto iter = _parentMap.find(child.index());
    if (iter != _parentMap.end()) {
        return iter->second;
    }
    return std::nullopt;
}

auto RelationshipManager::hasParent(Entity child) const -> bool {
    std::shared_lock lock(_relationshipMutex);
    return _parentMap.find(child.index()) != _parentMap.end();
}

auto RelationshipManager::getChildren(Entity parent) const
    -> std::vector<Entity> {
    std::shared_lock lock(_relationshipMutex);
    std::vector<Entity> result;

    auto iter = _childrenMap.find(parent.index());
    if (iter != _childrenMap.end()) {
        result.reserve(iter->second.size());
        for (const auto& [child_idx, child_entity] : iter->second) {
            result.push_back(child_entity);
        }
    }

    return result;
}

auto RelationshipManager::getDescendants(Entity parent) const
    -> std::vector<Entity> {
    std::shared_lock lock(_relationshipMutex);
    std::vector<Entity> result;
    getDescendantsRecursive(parent, &result);
    return result;
}

auto RelationshipManager::getAncestors(Entity child) const
    -> std::vector<Entity> {
    std::shared_lock lock(_relationshipMutex);
    std::vector<Entity> result;

    auto current = child;
    while (true) {
        auto iter = _parentMap.find(current.index());
        if (iter == _parentMap.end()) {
            break;
        }

        result.push_back(iter->second);
        current = iter->second;
    }

    return result;
}

auto RelationshipManager::getRoot(Entity entity) const -> Entity {
    std::shared_lock lock(_relationshipMutex);

    auto current = entity;
    while (true) {
        auto iter = _parentMap.find(current.index());
        if (iter == _parentMap.end()) {
            break;
        }
        current = iter->second;
    }

    return current;
}

auto RelationshipManager::isAncestor(Entity potential_ancestor,
                                     Entity entity) const -> bool {
    std::shared_lock lock(_relationshipMutex);

    auto current = entity;
    while (true) {
        auto iter = _parentMap.find(current.index());
        if (iter == _parentMap.end()) {
            return false;
        }

        if (iter->second == potential_ancestor) {
            return true;
        }
        current = iter->second;
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

auto RelationshipManager::childCount(Entity parent) const -> size_t {
    std::shared_lock lock(_relationshipMutex);
    auto iter = _childrenMap.find(parent.index());
    return (iter != _childrenMap.end()) ? iter->second.size() : 0;
}

auto RelationshipManager::getDepth(Entity entity) const -> size_t {
    std::shared_lock lock(_relationshipMutex);

    size_t depth = 0;
    auto current = entity;

    while (true) {
        auto iter = _parentMap.find(current.index());
        if (iter == _parentMap.end()) {
            break;
        }
        depth++;
        current = iter->second;
    }

    return depth;
}

auto RelationshipManager::wouldCreateCycle(Entity child, Entity parent) const
    -> bool {
    auto current = parent;
    while (true) {
        if (current == child) {
            return true;
        }

        auto iter = _parentMap.find(current.index());
        if (iter == _parentMap.end()) {
            break;
        }
        current = iter->second;
    }

    return false;
}

void RelationshipManager::getDescendantsRecursive(
    Entity parent, std::vector<Entity>* result) const {
    auto iter = _childrenMap.find(parent.index());
    if (iter == _childrenMap.end()) {
        return;
    }

    for (const auto& [child_idx, child_entity] : iter->second) {
        result->push_back(child_entity);
        getDescendantsRecursive(child_entity, result);
    }
}

}  // namespace ECS
