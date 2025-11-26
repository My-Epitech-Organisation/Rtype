/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MemoryPool - Linear allocator for efficient memory management
*/

#ifndef MEMORYPOOL_HPP
    #define MEMORYPOOL_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>

namespace Memory {

/**
 * @brief Linear Memory Pool Allocator
 * 
 * This allocator pre-allocates a large block of memory and assigns pointers
 * within it. It's extremely fast for allocation but doesn't support individual
 * deallocation (all or nothing).
 * 
 * Use cases:
 * - Frame-based allocations (cleared every frame)
 * - Temporary calculations
 * - Level loading
 */
class MemoryPool {
public:
    /**
     * @brief Construct a memory pool with specified size
     * @param size Size in bytes of the memory pool
     */
    explicit MemoryPool(size_t size)
        : _size(size)
        , _used(0)
        , _peakUsage(0)
        , _allocationCount(0)
    {
        _memory = static_cast<uint8_t*>(::operator new(size));
        if (!_memory) {
            throw std::bad_alloc();
        }
        std::memset(_memory, 0, size);
    }

    /**
     * @brief Destructor - frees the memory pool
     */
    ~MemoryPool() {
        if (_memory) {
            ::operator delete(_memory);
        }
    }

    // Disable copy
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Enable move
    MemoryPool(MemoryPool&& other) noexcept
        : _memory(other._memory)
        , _size(other._size)
        , _used(other._used)
        , _peakUsage(other._peakUsage)
        , _allocationCount(other._allocationCount)
    {
        other._memory = nullptr;
        other._size = 0;
        other._used = 0;
    }

    MemoryPool& operator=(MemoryPool&& other) noexcept {
        if (this != &other) {
            if (_memory) {
                ::operator delete(_memory);
            }
            _memory = other._memory;
            _size = other._size;
            _used = other._used;
            _peakUsage = other._peakUsage;
            _allocationCount = other._allocationCount;
            
            other._memory = nullptr;
            other._size = 0;
            other._used = 0;
        }
        return *this;
    }

    /**
     * @brief Allocate memory from the pool
     * @param size Size in bytes to allocate
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to allocated memory
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        if (size == 0) {
            return nullptr;
        }

        // Calculate aligned offset
        size_t alignedOffset = alignUp(_used, alignment);
        
        // Check if we have enough space
        if (alignedOffset + size > _size) {
            throw std::bad_alloc();
        }

        void* ptr = _memory + alignedOffset;
        _used = alignedOffset + size;
        _allocationCount++;

        if (_used > _peakUsage) {
            _peakUsage = _used;
        }

        return ptr;
    }

    /**
     * @brief Allocate and construct an object
     * @tparam T Type of object to allocate
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Pointer to constructed object
     */
    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Allocate an array of objects
     * @tparam T Type of array elements
     * @param count Number of elements
     * @return Pointer to allocated array
     */
    template<typename T>
    T* allocateArray(size_t count) {
        if (count == 0) {
            return nullptr;
        }
        
        void* ptr = allocate(sizeof(T) * count, alignof(T));
        
        // Default construct elements
        T* array = static_cast<T*>(ptr);
        for (size_t i = 0; i < count; ++i) {
            new (array + i) T();
        }
        
        return array;
    }

    /**
     * @brief Reset the pool (clear all allocations)
     */
    void reset() {
        _used = 0;
        // Optionally clear memory for debugging
        #ifdef DEBUG_MEMORY
        std::memset(_memory, 0, _size);
        #endif
    }

    /**
     * @brief Get current memory usage
     */
    size_t used() const { return _used; }

    /**
     * @brief Get total pool size
     */
    size_t size() const { return _size; }

    /**
     * @brief Get remaining available memory
     */
    size_t available() const { return _size - _used; }

    /**
     * @brief Get peak usage
     */
    size_t peakUsage() const { return _peakUsage; }

    /**
     * @brief Get number of allocations made
     */
    size_t allocationCount() const { return _allocationCount; }

    /**
     * @brief Get usage percentage
     */
    float usagePercentage() const {
        return _size > 0 ? (static_cast<float>(_used) / static_cast<float>(_size)) * 100.0f : 0.0f;
    }

    /**
     * @brief Statistics structure
     */
    struct Statistics {
        size_t totalSize = 0;
        size_t used = 0;
        size_t available = 0;
        size_t peakUsage = 0;
        size_t allocationCount = 0;
        float usagePercentage = 0.0f;

        void print() const {
            std::cout << "=== Memory Pool Statistics ===" << std::endl;
            std::cout << "Total Size:        " << formatBytes(totalSize) << std::endl;
            std::cout << "Used:              " << formatBytes(used) << std::endl;
            std::cout << "Available:         " << formatBytes(available) << std::endl;
            std::cout << "Peak Usage:        " << formatBytes(peakUsage) << std::endl;
            std::cout << "Allocation Count:  " << allocationCount << std::endl;
            std::cout << "Usage:             " << usagePercentage << "%" << std::endl;
        }

        static std::string formatBytes(size_t bytes) {
            const char* units[] = {"B", "KB", "MB", "GB"};
            int unit = 0;
            double size = static_cast<double>(bytes);
            
            while (size >= 1024.0 && unit < 3) {
                size /= 1024.0;
                unit++;
            }
            
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit]);
            return std::string(buffer);
        }
    };

    /**
     * @brief Get pool statistics
     */
    Statistics getStatistics() const {
        Statistics stats;
        stats.totalSize = _size;
        stats.used = _used;
        stats.available = available();
        stats.peakUsage = _peakUsage;
        stats.allocationCount = _allocationCount;
        stats.usagePercentage = usagePercentage();
        return stats;
    }

    /**
     * @brief Print statistics
     */
    void printStatistics() const {
        getStatistics().print();
    }

private:
    /**
     * @brief Align a value up to the specified alignment
     */
    static size_t alignUp(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    uint8_t* _memory = nullptr;    ///< Pointer to the memory block
    size_t _size = 0;              ///< Total size of the pool
    size_t _used = 0;              ///< Currently used bytes
    size_t _peakUsage = 0;         ///< Peak usage
    size_t _allocationCount = 0;   ///< Number of allocations
};

/**
 * @brief Stack-based allocator for temporary allocations
 * 
 * Similar to MemoryPool but supports stack-like deallocation
 * by tracking allocation markers.
 */
class StackAllocator {
public:
    struct Marker {
        size_t offset;
    };

    explicit StackAllocator(size_t size)
        : _pool(size) {}

    /**
     * @brief Get current marker position
     */
    Marker getMarker() const {
        return Marker{_pool.used()};
    }

    /**
     * @brief Rewind to a previous marker
     */
    void rewindToMarker(const Marker& marker) {
        // This is a simplified version - would need more sophisticated
        // implementation for production use
        if (marker.offset <= _pool.used()) {
            // In a real implementation, we'd track allocations
            // and call destructors as needed
        }
    }

    /**
     * @brief Allocate from the stack
     */
    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        return _pool.allocate<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief Get statistics
     */
    auto getStatistics() const { return _pool.getStatistics(); }

    /**
     * @brief Reset the allocator
     */
    void reset() { _pool.reset(); }

private:
    MemoryPool _pool;
};

} // namespace Memory

#endif // MEMORYPOOL_HPP
