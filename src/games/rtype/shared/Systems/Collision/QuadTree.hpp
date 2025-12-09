/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** QuadTree - Spatial partitioning data structure for collision optimization
*/

#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "Rect.hpp"

namespace rtype::games::rtype::shared::collision {

/**
 * @brief Generic object that can be stored in a QuadTree.
 *
 * @tparam T The type of data associated with the object (typically entity ID)
 */
template <typename T>
struct QuadTreeObject {
    Rect bounds;
    T data;

    QuadTreeObject(const Rect& bounds, const T& data)
        : bounds(bounds), data(data) {}
};

/**
 * @class QuadTree
 * @brief QuadTree node for spatial partitioning.
 *
 * The QuadTree recursively subdivides space into four quadrants.
 * Objects are stored at the deepest level where they fit entirely within a
 * node.
 *
 * Quadrant layout:
 * +-------+-------+
 * |       |       |
 * |  NW   |  NE   |
 * |       |       |
 * +-------+-------+
 * |       |       |
 * |  SW   |  SE   |
 * |       |       |
 * +-------+-------+
 *
 * Time Complexity:
 * - Insert: O(log n) average, O(n) worst case
 * - Query: O(log n + k) where k is the number of results
 *
 * @tparam T The type of data associated with stored objects
 */
template <typename T>
class QuadTree {
   public:
    static constexpr size_t DEFAULT_MAX_OBJECTS = 10;
    static constexpr size_t DEFAULT_MAX_DEPTH = 5;

    /**
     * @brief Constructs a QuadTree with specified parameters.
     *
     * @param bounds The boundary of this node
     * @param maxObjects Maximum objects before subdivision (default: 10)
     * @param maxDepth Maximum tree depth (default: 5)
     * @param depth Current depth of this node (default: 0)
     */
    explicit QuadTree(const Rect& bounds,
                      size_t maxObjects = DEFAULT_MAX_OBJECTS,
                      size_t maxDepth = DEFAULT_MAX_DEPTH, size_t depth = 0)
        : _bounds(bounds),
          _maxObjects(maxObjects),
          _maxDepth(maxDepth),
          _depth(depth),
          _divided(false) {}

    /**
     * @brief Inserts an object into the QuadTree.
     *
     * The object will be placed in the deepest node that can fully contain it.
     * If a node exceeds its capacity, it will subdivide.
     *
     * @param obj The object to insert
     * @return true if insertion was successful, false otherwise
     */
    bool insert(const QuadTreeObject<T>& obj) {
        if (!_bounds.contains(obj.bounds)) {
            return false;
        }

        if (_divided) {
            if (_northwest->insert(obj)) return true;
            if (_northeast->insert(obj)) return true;
            if (_southwest->insert(obj)) return true;
            if (_southeast->insert(obj)) return true;

            _objects.push_back(obj);
            return true;
        }

        _objects.push_back(obj);

        if (_objects.size() > _maxObjects && _depth < _maxDepth) {
            subdivide();
        }

        return true;
    }

    /**
     * @brief Queries objects within a range.
     *
     * Returns all objects whose bounding boxes intersect with the query range.
     * Uses a non-const reference output parameter to efficiently accumulate
     * results across recursive calls without repeated vector allocations.
     *
     * @param range The query range
     * @param found Vector to store found objects (output parameter)
     */
    void query(const Rect& range, std::vector<QuadTreeObject<T>>& found) const {
        if (!_bounds.intersects(range)) {
            return;
        }

        for (const auto& obj : _objects) {
            if (obj.bounds.intersects(range)) {
                found.push_back(obj);
            }
        }

        if (_divided) {
            _northwest->query(range, found);
            _northeast->query(range, found);
            _southwest->query(range, found);
            _southeast->query(range, found);
        }
    }

    /**
     * @brief Queries all objects in the QuadTree.
     *
     * Uses a non-const reference output parameter to efficiently accumulate
     * results across recursive calls without repeated vector allocations.
     *
     * @param found Vector to store all objects (output parameter)
     */
    void queryAll(std::vector<QuadTreeObject<T>>& found) const {
        found.insert(found.end(), _objects.begin(), _objects.end());

        if (_divided) {
            _northwest->queryAll(found);
            _northeast->queryAll(found);
            _southwest->queryAll(found);
            _southeast->queryAll(found);
        }
    }

    /**
     * @brief Clears all objects from the QuadTree.
     *
     * Removes all stored objects and collapses subdivisions.
     */
    void clear() {
        _objects.clear();
        if (_divided) {
            _northwest.reset();
            _northeast.reset();
            _southwest.reset();
            _southeast.reset();
            _divided = false;
        }
    }

    /**
     * @brief Gets the number of objects in this node (not including
     * subdivisions).
     * @return Number of objects
     */
    [[nodiscard]] size_t size() const noexcept { return _objects.size(); }

    /**
     * @brief Gets the total number of objects in the tree (including all
     * subdivisions).
     * @return Total number of objects
     */
    [[nodiscard]] size_t totalSize() const noexcept {
        size_t total = _objects.size();
        if (_divided) {
            total += _northwest->totalSize();
            total += _northeast->totalSize();
            total += _southwest->totalSize();
            total += _southeast->totalSize();
        }
        return total;
    }

    /**
     * @brief Gets the bounds of this node.
     * @return The bounding rectangle
     */
    [[nodiscard]] const Rect& getBounds() const noexcept { return _bounds; }

    /**
     * @brief Checks if this node has been subdivided.
     * @return true if subdivided, false otherwise
     */
    [[nodiscard]] bool isDivided() const noexcept { return _divided; }

    /**
     * @brief Gets the depth of this node.
     * @return The depth level
     */
    [[nodiscard]] size_t getDepth() const noexcept { return _depth; }

    /**
     * @brief Gets the number of nodes in the tree (including subdivisions).
     * @return Total number of nodes
     */
    [[nodiscard]] size_t getNodeCount() const noexcept {
        size_t count = 1;
        if (_divided) {
            count += _northwest->getNodeCount();
            count += _northeast->getNodeCount();
            count += _southwest->getNodeCount();
            count += _southeast->getNodeCount();
        }
        return count;
    }

   private:
    /**
     * @brief Subdivides this node into four quadrants.
     *
     * Attempts to redistribute existing objects into the new quadrants.
     * Objects that span multiple quadrants remain in this node.
     */
    void subdivide() {
        if (_divided) return;

        float halfW = _bounds.w * 0.5F;
        float halfH = _bounds.h * 0.5F;
        float x = _bounds.x;
        float y = _bounds.y;

        _northwest = std::make_unique<QuadTree>(
            Rect{x, y, halfW, halfH}, _maxObjects, _maxDepth, _depth + 1);
        _northeast =
            std::make_unique<QuadTree>(Rect{x + halfW, y, halfW, halfH},
                                       _maxObjects, _maxDepth, _depth + 1);
        _southwest =
            std::make_unique<QuadTree>(Rect{x, y + halfH, halfW, halfH},
                                       _maxObjects, _maxDepth, _depth + 1);
        _southeast =
            std::make_unique<QuadTree>(Rect{x + halfW, y + halfH, halfW, halfH},
                                       _maxObjects, _maxDepth, _depth + 1);

        _divided = true;

        std::vector<QuadTreeObject<T>> remaining;
        for (const auto& obj : _objects) {
            bool inserted = _northwest->insert(obj) ||
                            _northeast->insert(obj) ||
                            _southwest->insert(obj) || _southeast->insert(obj);

            if (!inserted) {
                remaining.push_back(obj);
            }
        }

        _objects = std::move(remaining);
    }

    Rect _bounds;
    size_t _maxObjects;
    size_t _maxDepth;
    size_t _depth;
    bool _divided;
    std::vector<QuadTreeObject<T>> _objects;

    std::unique_ptr<QuadTree> _northwest;
    std::unique_ptr<QuadTree> _northeast;
    std::unique_ptr<QuadTree> _southwest;
    std::unique_ptr<QuadTree> _southeast;
};

}  // namespace rtype::games::rtype::shared::collision
