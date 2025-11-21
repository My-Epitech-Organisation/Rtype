/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CircularBuffer
*/

#pragma once

#include <vector>
#include <cstdint>

namespace rtype::network {

class CircularBuffer {
    public:
        explicit CircularBuffer(size_t capacity = 4096);
        ~CircularBuffer() = default;
        bool write(const std::vector<uint8_t>& data);
        std::vector<uint8_t> read(size_t bytes);
        size_t size() const;
        size_t capacity() const;
        bool empty() const;
        bool full() const;
        void clear();

    private:
        size_t nextIndex(size_t index) const;

        std::vector<uint8_t> _buffer;
        size_t _head;
        size_t _tail;
        size_t _size;
        size_t _capacity;
};

}  // namespace rtype::network
