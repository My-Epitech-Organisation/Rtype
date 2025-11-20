/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CircularBuffer
*/

#include "rtype/network/CircularBuffer.hpp"
#include <algorithm>
#include <vector>
#include <cstdint>

namespace rtype::network {

CircularBuffer::CircularBuffer(size_t capacity)
    : _buffer(capacity, 0), _head(0), _tail(0), _size(0), _capacity(capacity) {
}

bool CircularBuffer::write(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        if (_size == _capacity) {
            _tail = nextIndex(_tail);
        } else {
            ++_size;
        }
        _buffer[_head] = byte;
        _head = nextIndex(_head);
    }
    return true;
}

std::vector<uint8_t> CircularBuffer::read(size_t bytes) {
    size_t toRead = std::min(bytes, _size);
    std::vector<uint8_t> result;
    result.reserve(toRead);
    for (size_t i = 0; i < toRead; ++i) {
        result.push_back(_buffer[_tail]);
        _tail = nextIndex(_tail);
        --_size;
    }
    return result;
}

size_t CircularBuffer::size() const {
    return _size;
}

size_t CircularBuffer::capacity() const {
    return _capacity;
}

bool CircularBuffer::empty() const {
    return _size == 0;
}

bool CircularBuffer::full() const {
    return _size == _capacity;
}

void CircularBuffer::clear() {
    _head = 0;
    _tail = 0;
    _size = 0;
}

size_t CircularBuffer::nextIndex(size_t index) const {
    return (index + 1) % _capacity;
}

}  // namespace rtype::network
