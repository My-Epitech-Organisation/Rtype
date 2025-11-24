/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Binary vs JSON Storage PoC
*/

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ============================================================================
// Test Data Structures (Simulating ECS Components)
// ============================================================================

/**
 * @brief Position component - Simple 2D position
 */
struct Position {
    float x;
    float y;
    
    Position() : x(0.0f), y(0.0f) {}
    Position(float _x, float _y) : x(_x), y(_y) {}
};

/**
 * @brief Velocity component - Movement speed
 */
struct Velocity {
    float dx;
    float dy;
    
    Velocity() : dx(0.0f), dy(0.0f) {}
    Velocity(float _dx, float _dy) : dx(_dx), dy(_dy) {}
};

/**
 * @brief Health component - Entity health state
 */
struct Health {
    int current;
    int maximum;
    
    Health() : current(100), maximum(100) {}
    Health(int cur, int max) : current(cur), maximum(max) {}
};

/**
 * @brief Sprite component - Rendering information
 */
struct Sprite {
    char texture_path[64];
    int layer;
    float scale;
    
    Sprite() : layer(0), scale(1.0f) {
        std::memset(texture_path, 0, sizeof(texture_path));
    }
    
    Sprite(const char* path, int l, float s) : layer(l), scale(s) {
        std::strncpy(texture_path, path, sizeof(texture_path) - 1);
        texture_path[sizeof(texture_path) - 1] = '\0';
    }
};

/**
 * @brief Entity data - Complete entity with all components
 */
struct EntityData {
    uint32_t entity_id;
    Position position;
    Velocity velocity;
    Health health;
    Sprite sprite;
    
    EntityData() : entity_id(0) {}
};

// ============================================================================
// Binary Storage Implementation
// ============================================================================

class BinaryStorage {
public:
    /**
     * @brief Write entities to binary file (packed format)
     */
    static bool write(const std::string& filename, const std::vector<EntityData>& entities) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Write header: number of entities
        uint32_t count = static_cast<uint32_t>(entities.size());
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        // Write all entities in one go (packed binary)
        file.write(reinterpret_cast<const char*>(entities.data()), 
                   entities.size() * sizeof(EntityData));
        
        file.close();
        return true;
    }
    
    /**
     * @brief Read entities from binary file
     */
    static bool read(const std::string& filename, std::vector<EntityData>& entities) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read header
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        // Resize and read all entities
        entities.resize(count);
        file.read(reinterpret_cast<char*>(entities.data()), 
                  count * sizeof(EntityData));
        
        file.close();
        return true;
    }
};

// ============================================================================
// JSON Storage Implementation
// ============================================================================

class JSONStorage {
public:
    /**
     * @brief Write entities to JSON file
     */
    static bool write(const std::string& filename, const std::vector<EntityData>& entities) {
        json j;
        j["entities"] = json::array();
        
        for (const auto& entity : entities) {
            json entity_json;
            entity_json["entity_id"] = entity.entity_id;
            
            entity_json["position"]["x"] = entity.position.x;
            entity_json["position"]["y"] = entity.position.y;
            
            entity_json["velocity"]["dx"] = entity.velocity.dx;
            entity_json["velocity"]["dy"] = entity.velocity.dy;
            
            entity_json["health"]["current"] = entity.health.current;
            entity_json["health"]["maximum"] = entity.health.maximum;
            
            entity_json["sprite"]["texture_path"] = entity.sprite.texture_path;
            entity_json["sprite"]["layer"] = entity.sprite.layer;
            entity_json["sprite"]["scale"] = entity.sprite.scale;
            
            j["entities"].push_back(entity_json);
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4); // Pretty print with 4-space indent
        file.close();
        return true;
    }
    
    /**
     * @brief Read entities from JSON file
     */
    static bool read(const std::string& filename, std::vector<EntityData>& entities) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        entities.clear();
        for (const auto& entity_json : j["entities"]) {
            EntityData entity;
            entity.entity_id = entity_json["entity_id"];
            
            entity.position.x = entity_json["position"]["x"];
            entity.position.y = entity_json["position"]["y"];
            
            entity.velocity.dx = entity_json["velocity"]["dx"];
            entity.velocity.dy = entity_json["velocity"]["dy"];
            
            entity.health.current = entity_json["health"]["current"];
            entity.health.maximum = entity_json["health"]["maximum"];
            
            std::string texture = entity_json["sprite"]["texture_path"];
            std::strncpy(entity.sprite.texture_path, texture.c_str(), 
                        sizeof(entity.sprite.texture_path) - 1);
            entity.sprite.layer = entity_json["sprite"]["layer"];
            entity.sprite.scale = entity_json["sprite"]["scale"];
            
            entities.push_back(entity);
        }
        
        return true;
    }
};

// ============================================================================
// Benchmark Utilities
// ============================================================================

class Timer {
public:
    void start() {
        _start = std::chrono::high_resolution_clock::now();
    }
    
    double stop_ms() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - _start).count();
    }
    
    double stop_us() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(end - _start).count();
    }
    
private:
    std::chrono::high_resolution_clock::time_point _start;
};

/**
 * @brief Generate random test data
 */
std::vector<EntityData> generate_test_data(size_t count) {
    std::vector<EntityData> entities;
    entities.reserve(count);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist(-1000.0f, 1000.0f);
    std::uniform_real_distribution<float> vel_dist(-10.0f, 10.0f);
    std::uniform_int_distribution<int> health_dist(0, 200);
    std::uniform_int_distribution<int> layer_dist(0, 10);
    std::uniform_real_distribution<float> scale_dist(0.5f, 2.0f);
    
    const char* textures[] = {
        "assets/player.png",
        "assets/enemy.png",
        "assets/bullet.png",
        "assets/powerup.png",
        "assets/background.png"
    };
    
    for (size_t i = 0; i < count; ++i) {
        EntityData entity;
        entity.entity_id = static_cast<uint32_t>(i);
        entity.position = Position(pos_dist(gen), pos_dist(gen));
        entity.velocity = Velocity(vel_dist(gen), vel_dist(gen));
        entity.health = Health(health_dist(gen), 100);
        entity.sprite = Sprite(textures[i % 5], layer_dist(gen), scale_dist(gen));
        entities.push_back(entity);
    }
    
    return entities;
}

/**
 * @brief Run complete benchmark suite
 */
void run_benchmark(size_t entity_count, size_t iterations = 100) {
    std::cout << "\n========================================\n";
    std::cout << "Benchmark: " << entity_count << " entities\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "========================================\n";
    
    // Generate test data
    auto entities = generate_test_data(entity_count);
    
    const std::string binary_file = "test_data.bin";
    const std::string json_file = "test_data.json";
    
    Timer timer;
    
    // ========== BINARY WRITE ==========
    std::vector<double> binary_write_times;
    for (size_t i = 0; i < iterations; ++i) {
        timer.start();
        BinaryStorage::write(binary_file, entities);
        binary_write_times.push_back(timer.stop_us());
    }
    
    // ========== JSON WRITE ==========
    std::vector<double> json_write_times;
    for (size_t i = 0; i < iterations; ++i) {
        timer.start();
        JSONStorage::write(json_file, entities);
        json_write_times.push_back(timer.stop_us());
    }
    
    // ========== BINARY READ ==========
    std::vector<double> binary_read_times;
    std::vector<EntityData> binary_entities;
    for (size_t i = 0; i < iterations; ++i) {
        timer.start();
        BinaryStorage::read(binary_file, binary_entities);
        binary_read_times.push_back(timer.stop_us());
    }
    
    // ========== JSON READ ==========
    std::vector<double> json_read_times;
    std::vector<EntityData> json_entities;
    for (size_t i = 0; i < iterations; ++i) {
        timer.start();
        JSONStorage::read(json_file, json_entities);
        json_read_times.push_back(timer.stop_us());
    }
    
    // Calculate statistics
    auto calc_avg = [](const std::vector<double>& times) {
        double sum = 0.0;
        for (double t : times) sum += t;
        return sum / times.size();
    };
    
    auto calc_min = [](const std::vector<double>& times) {
        return *std::min_element(times.begin(), times.end());
    };
    
    auto calc_max = [](const std::vector<double>& times) {
        return *std::max_element(times.begin(), times.end());
    };
    
    // Get file sizes
    auto get_file_size = [](const std::string& filename) -> size_t {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        return file.tellg();
    };
    
    size_t binary_size = get_file_size(binary_file);
    size_t json_size = get_file_size(json_file);
    
    // Print results
    std::cout << "\n--- WRITE PERFORMANCE ---\n";
    std::cout << "Binary:\n";
    std::cout << "  Avg: " << calc_avg(binary_write_times) << " μs\n";
    std::cout << "  Min: " << calc_min(binary_write_times) << " μs\n";
    std::cout << "  Max: " << calc_max(binary_write_times) << " μs\n";
    std::cout << "\nJSON:\n";
    std::cout << "  Avg: " << calc_avg(json_write_times) << " μs\n";
    std::cout << "  Min: " << calc_min(json_write_times) << " μs\n";
    std::cout << "  Max: " << calc_max(json_write_times) << " μs\n";
    std::cout << "\nSpeedup: " << calc_avg(json_write_times) / calc_avg(binary_write_times) << "x\n";
    
    std::cout << "\n--- READ PERFORMANCE ---\n";
    std::cout << "Binary:\n";
    std::cout << "  Avg: " << calc_avg(binary_read_times) << " μs\n";
    std::cout << "  Min: " << calc_min(binary_read_times) << " μs\n";
    std::cout << "  Max: " << calc_max(binary_read_times) << " μs\n";
    std::cout << "\nJSON:\n";
    std::cout << "  Avg: " << calc_avg(json_read_times) << " μs\n";
    std::cout << "  Min: " << calc_min(json_read_times) << " μs\n";
    std::cout << "  Max: " << calc_max(json_read_times) << " μs\n";
    std::cout << "\nSpeedup: " << calc_avg(json_read_times) / calc_avg(binary_read_times) << "x\n";
    
    std::cout << "\n--- FILE SIZE ---\n";
    std::cout << "Binary: " << binary_size << " bytes\n";
    std::cout << "JSON:   " << json_size << " bytes\n";
    std::cout << "Ratio:  " << static_cast<double>(json_size) / binary_size << "x larger\n";
    
    std::cout << "\n--- TOTAL TIME (Write + Read) ---\n";
    double binary_total = calc_avg(binary_write_times) + calc_avg(binary_read_times);
    double json_total = calc_avg(json_write_times) + calc_avg(json_read_times);
    std::cout << "Binary: " << binary_total << " μs\n";
    std::cout << "JSON:   " << json_total << " μs\n";
    std::cout << "Speedup: " << json_total / binary_total << "x\n";
    
    // Cleanup
    std::remove(binary_file.c_str());
    std::remove(json_file.c_str());
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║  Binary Packed vs JSON Storage Benchmark      ║\n";
    std::cout << "║  R-Type ECS Data Serialization PoC            ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    // Run benchmarks with different entity counts
    run_benchmark(100, 100);
    run_benchmark(1000, 100);
    run_benchmark(10000, 50);
    run_benchmark(50000, 20);
    
    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete!\n";
    std::cout << "========================================\n";
    
    return 0;
}
