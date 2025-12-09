/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** QuadTreeSystem - Spatial partitioning system implementation
*/

#include "QuadTreeSystem.hpp"

#include <unordered_set>

namespace rtype::games::rtype::shared {

QuadTreeSystem::QuadTreeSystem(const collision::Rect& worldBounds,
                               size_t maxObjects, size_t maxDepth)
    : ASystem("QuadTreeSystem"),
      _worldBounds(worldBounds),
      _maxObjects(maxObjects),
      _maxDepth(maxDepth),
      _quadTree(nullptr) {}

void QuadTreeSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    _quadTree = std::make_unique<collision::QuadTree<uint32_t>>(
        _worldBounds, _maxObjects, _maxDepth);
    auto view = registry.view<TransformComponent, BoundingBoxComponent>();

    view.each([this](ECS::Entity entity, const TransformComponent& transform,
                     const BoundingBoxComponent& bbox) {
        collision::Rect bounds = createRectFromComponents(transform, bbox);
        collision::QuadTreeObject<uint32_t> obj(bounds, entity.id);
        _quadTree->insert(obj);
    });
}

std::vector<CollisionPair> QuadTreeSystem::queryCollisionPairs(
    ECS::Registry& registry) const {
    std::vector<CollisionPair> pairs;

    if (!_quadTree) {
        return pairs;
    }
    std::unordered_set<uint64_t> checkedPairs;

    auto view = registry.view<TransformComponent, BoundingBoxComponent>();

    view.each([this, &registry, &pairs, &checkedPairs](
                  ECS::Entity entity, const TransformComponent& transform,
                  const BoundingBoxComponent& bbox) {
        collision::Rect bounds = createRectFromComponents(transform, bbox);

        std::vector<collision::QuadTreeObject<uint32_t>> nearby;
        _quadTree->query(bounds, nearby);

        for (const auto& other : nearby) {
            if (other.data == entity.id) {
                continue;
            }
            uint32_t minId = std::min(entity.id, other.data);
            uint32_t maxId = std::max(entity.id, other.data);
            uint64_t pairKey = (static_cast<uint64_t>(minId) << 32) | maxId;

            if (checkedPairs.find(pairKey) != checkedPairs.end()) {
                continue;
            }
            checkedPairs.insert(pairKey);
            ECS::Entity otherEntity{other.data, 0};
            pairs.emplace_back(entity, ECS::Entity{other.data, 0});
        }
    });

    return pairs;
}

std::vector<ECS::Entity> QuadTreeSystem::queryNearby(
    const collision::Rect& area) const {
    std::vector<ECS::Entity> result;

    if (!_quadTree) {
        return result;
    }

    std::vector<collision::QuadTreeObject<uint32_t>> found;
    _quadTree->query(area, found);

    result.reserve(found.size());
    for (const auto& obj : found) {
        result.push_back(ECS::Entity{obj.data, 0});
    }

    return result;
}

std::vector<ECS::Entity> QuadTreeSystem::queryNearby(float x, float y,
                                                     float radius) const {
    collision::Rect area(x - radius, y - radius, radius * 2.0F, radius * 2.0F);
    return queryNearby(area);
}

collision::Rect QuadTreeSystem::createRectFromComponents(
    const TransformComponent& transform, const BoundingBoxComponent& bbox) {
    const float halfWidth = bbox.width * 0.5F;
    const float halfHeight = bbox.height * 0.5F;

    return collision::Rect(transform.x - halfWidth, transform.y - halfHeight,
                           bbox.width, bbox.height);
}

}  // namespace rtype::games::rtype::shared

