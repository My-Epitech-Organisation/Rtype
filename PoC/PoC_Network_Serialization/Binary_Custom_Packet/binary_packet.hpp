// Custom binary packet serialization
#ifndef BINARY_PACKET_HPP
#define BINARY_PACKET_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <type_traits>

// Binary serializer for custom packets
class BinarySerializer {
private:
    std::vector<uint8_t> buffer_;

public:
    BinarySerializer() { buffer_.reserve(1024); }

    // Write primitive types
    template<typename T>
    void write(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        for (size_t i = 0; i < sizeof(T); ++i) {
            buffer_.push_back(bytes[i]);
        }
    }

    // Write array
    template<typename T>
    void write_array(const T* data, size_t count) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        buffer_.insert(buffer_.end(), bytes, bytes + (sizeof(T) * count));
    }

    const std::vector<uint8_t>& data() const { return buffer_; }
    size_t size() const { return buffer_.size(); }
    void clear() { buffer_.clear(); }
};

// Binary deserializer
class BinaryDeserializer {
private:
    const uint8_t* data_;
    size_t size_;
    size_t offset_;

public:
    BinaryDeserializer(const uint8_t* data, size_t size)
        : data_(data), size_(size), offset_(0) {}

    BinaryDeserializer(const std::vector<uint8_t>& buffer)
        : data_(buffer.data()), size_(buffer.size()), offset_(0) {}

    // Read primitive types
    template<typename T>
    T read() {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        if (offset_ + sizeof(T) > size_) {
            throw std::runtime_error("Buffer overflow");
        }
        T value;
        std::memcpy(&value, data_ + offset_, sizeof(T));
        offset_ += sizeof(T);
        return value;
    }

    // Read array
    template<typename T>
    void read_array(T* data, size_t count) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        size_t bytes = sizeof(T) * count;
        if (offset_ + bytes > size_) {
            throw std::runtime_error("Buffer overflow");
        }
        std::memcpy(data, data_ + offset_, bytes);
        offset_ += bytes;
    }

    size_t remaining() const { return size_ - offset_; }
};

// Compact position structure (8 bytes)
struct Position {
    float x;       // 4 bytes
    float y;       // 4 bytes
    // Total: 8 bytes

    void serialize(BinarySerializer& s) const {
        s.write(x);
        s.write(y);
    }

    static Position deserialize(BinaryDeserializer& d) {
        return Position{d.read<float>(), d.read<float>()};
    }
};

// Position with rotation (12 bytes)
struct PositionRot {
    float x;        // 4 bytes
    float y;        // 4 bytes
    float rotation; // 4 bytes
    // Total: 12 bytes

    void serialize(BinarySerializer& s) const {
        s.write(x);
        s.write(y);
        s.write(rotation);
    }

    static PositionRot deserialize(BinaryDeserializer& d) {
        return PositionRot{d.read<float>(), d.read<float>(), d.read<float>()};
    }
};

// Compact entity state (20 bytes)
struct EntityState {
    uint32_t id;         // 4 bytes
    float x;             // 4 bytes
    float y;             // 4 bytes
    float vel_x;         // 4 bytes
    float vel_y;         // 4 bytes
    // Total: 20 bytes (rotation, health, team omitted or packed)

    void serialize(BinarySerializer& s) const {
        s.write(id);
        s.write(x);
        s.write(y);
        s.write(vel_x);
        s.write(vel_y);
    }

    static EntityState deserialize(BinaryDeserializer& d) {
        return EntityState{
            d.read<uint32_t>(),
            d.read<float>(),
            d.read<float>(),
            d.read<float>(),
            d.read<float>()
        };
    }
};

// Full entity state (24 bytes)
struct EntityStateFull {
    uint32_t id;         // 4 bytes
    float x;             // 4 bytes
    float y;             // 4 bytes
    float rotation;      // 4 bytes
    float vel_x;         // 4 bytes
    float vel_y;         // 4 bytes
    uint8_t health;      // 1 byte
    uint8_t team;        // 1 byte
    uint8_t padding[2];  // 2 bytes (alignment)
    // Total: 28 bytes

    void serialize(BinarySerializer& s) const {
        s.write(id);
        s.write(x);
        s.write(y);
        s.write(rotation);
        s.write(vel_x);
        s.write(vel_y);
        s.write(health);
        s.write(team);
    }

    static EntityStateFull deserialize(BinaryDeserializer& d) {
        return EntityStateFull{
            d.read<uint32_t>(),
            d.read<float>(),
            d.read<float>(),
            d.read<float>(),
            d.read<float>(),
            d.read<float>(),
            d.read<uint8_t>(),
            d.read<uint8_t>(),
            {0, 0}
        };
    }
};

// Game state packet
struct GameStatePacket {
    uint32_t timestamp;                // 4 bytes
    uint8_t entity_count;              // 1 byte
    std::vector<EntityState> entities; // Variable size
    // Header: 5 bytes + (entity_count * 20 bytes)

    void serialize(BinarySerializer& s) const {
        s.write(timestamp);
        s.write(entity_count);
        for (const auto& entity : entities) {
            entity.serialize(s);
        }
    }

    static GameStatePacket deserialize(BinaryDeserializer& d) {
        GameStatePacket packet;
        packet.timestamp = d.read<uint32_t>();
        packet.entity_count = d.read<uint8_t>();

        packet.entities.reserve(packet.entity_count);
        for (uint8_t i = 0; i < packet.entity_count; ++i) {
            packet.entities.push_back(EntityState::deserialize(d));
        }
        return packet;
    }
};

#endif // BINARY_PACKET_HPP
