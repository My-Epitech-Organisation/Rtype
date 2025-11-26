/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ObjectPool - Object Pool implementation for memory reuse
*/

#ifndef OBJECTPOOL_HPP
    #define OBJECTPOOL_HPP

#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <iostream>

namespace Memory {

/**
 * @brief Object Pool for efficient object reuse
 * 
 * This implementation provides zero-allocation object reuse by maintaining
 * a pool of pre-allocated objects. When an object is released, it's returned
 * to the pool for future reuse instead of being deallocated.
 * 
 * @tparam T Type of objects to pool
 */
template<typename T>
class ObjectPool {
public:
    /**
     * @brief Construct an object pool with initial capacity
     * @param initialCapacity Initial number of objects to pre-allocate
     */
    explicit ObjectPool(size_t initialCapacity = 100)
        : _growthFactor(2.0f)
    {
        reserve(initialCapacity);
    }

    /**
     * @brief Destructor - cleans up all pooled objects
     */
    ~ObjectPool() {
        clear();
    }

    // Disable copy
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // Enable move
    ObjectPool(ObjectPool&&) noexcept = default;
    ObjectPool& operator=(ObjectPool&&) noexcept = default;

    /**
     * @brief Acquire an object from the pool
     * @param args Constructor arguments for new objects
     * @return Pointer to the acquired object
     */
    template<typename... Args>
    T* acquire(Args&&... args) {
        if (_available.empty()) {
            grow();
        }

        T* obj = _available.back();
        _available.pop_back();
        _inUse++;

        // Reconstruct object with new arguments
        new (obj) T(std::forward<Args>(args)...);

        _stats.totalAcquisitions++;
        return obj;
    }

    /**
     * @brief Release an object back to the pool
     * @param obj Pointer to the object to release
     */
    void release(T* obj) {
        if (!obj) {
            return;
        }

        // Call destructor
        obj->~T();

        // Return to available pool
        _available.push_back(obj);
        _inUse--;

        _stats.totalReleases++;
    }

    /**
     * @brief Reserve space for a specific number of objects
     * @param capacity Number of objects to reserve space for
     */
    void reserve(size_t capacity) {
        while (_pool.size() < capacity) {
            allocateBlock();
        }
    }

    /**
     * @brief Clear all pooled objects
     */
    void clear() {
        // Destruct all objects still in use
        for (auto* block : _blocks) {
            delete[] reinterpret_cast<char*>(block);
        }
        _blocks.clear();
        _pool.clear();
        _available.clear();
        _inUse = 0;
    }

    /**
     * @brief Get the number of objects currently in use
     */
    size_t inUse() const { return _inUse; }

    /**
     * @brief Get the number of available objects in the pool
     */
    size_t available() const { return _available.size(); }

    /**
     * @brief Get the total capacity of the pool
     */
    size_t capacity() const { return _pool.size(); }

    /**
     * @brief Statistics structure
     */
    struct Statistics {
        size_t totalAcquisitions = 0;
        size_t totalReleases = 0;
        size_t totalAllocations = 0;
        size_t peakUsage = 0;

        void print() const {
            std::cout << "=== Object Pool Statistics ===" << std::endl;
            std::cout << "Total Acquisitions: " << totalAcquisitions << std::endl;
            std::cout << "Total Releases: " << totalReleases << std::endl;
            std::cout << "Total Allocations: " << totalAllocations << std::endl;
            std::cout << "Peak Usage: " << peakUsage << std::endl;
        }
    };

    /**
     * @brief Get pool statistics
     */
    const Statistics& getStatistics() const { return _stats; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics() { 
        _stats = Statistics(); 
    }

private:
    /**
     * @brief Allocate a new block of objects
     */
    void allocateBlock() {
        constexpr size_t blockSize = 32;
        
        // Allocate raw memory
        char* rawMemory = new char[sizeof(T) * blockSize];
        T* block = reinterpret_cast<T*>(rawMemory);
        
        _blocks.push_back(block);
        
        // Add objects to pool without constructing them
        for (size_t i = 0; i < blockSize; ++i) {
            T* obj = block + i;
            _pool.push_back(obj);
            _available.push_back(obj);
        }

        _stats.totalAllocations += blockSize;
    }

    /**
     * @brief Grow the pool when it's exhausted
     */
    void grow() {
        size_t oldCapacity = _pool.size();
        size_t newCapacity = oldCapacity == 0 ? 32 : 
                             static_cast<size_t>(oldCapacity * _growthFactor);
        
        size_t toAllocate = newCapacity - oldCapacity;
        size_t blocks = (toAllocate + 31) / 32; // Round up to block size
        
        for (size_t i = 0; i < blocks; ++i) {
            allocateBlock();
        }
    }

    std::vector<T*> _blocks;           ///< Memory blocks
    std::vector<T*> _pool;             ///< All pooled objects
    std::vector<T*> _available;        ///< Available objects
    size_t _inUse = 0;                 ///< Number of objects in use
    float _growthFactor;               ///< Growth factor when pool is exhausted
    Statistics _stats;                 ///< Pool statistics
};

} // namespace Memory

#endif // OBJECTPOOL_HPP
