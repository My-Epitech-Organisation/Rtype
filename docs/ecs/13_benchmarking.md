# Benchmarking

## Overview

The **Benchmark** utility provides performance measurement and comparison tools for profiling ECS operations and systems.

## Basic Usage

```cpp
#include "ECS/Utils/Benchmark.hpp"

ECS::Benchmark bench;

// Measure execution time
bench.measure("Test Name", []() {
    // Code to benchmark
}, 100); // Run 100 iterations

// Print results
bench.print_results();
```

## API Reference

### Measurement

```cpp
template<typename Func>
void measure(const std::string& name, Func&& func, size_t iterations = 100);
```

### Results

```cpp
struct Result {
    std::string name;
    double avg_time_us;  // Average time in microseconds
    double min_time_us;  // Minimum time
    double max_time_us;  // Maximum time
    size_t iterations;   // Number of runs
};

void print_results() const;
void compare(const std::string& name1, const std::string& name2) const;
const std::vector<Result>& get_results() const;
```

## Practical Examples

### Compare View vs Group

```cpp
ECS::Registry registry;
ECS::Benchmark bench;

// Populate with entities
for (int i = 0; i < 10000; ++i) {
    auto e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 0.0f, 0.0f);
    registry.emplaceComponent<Velocity>(e, 1.0f, 0.0f);
}

// Benchmark View
bench.measure("View", [&]() {
    registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
        p.x += v.dx;
        p.y += v.dy;
    });
}, 1000);

// Benchmark Group
auto group = registry.createGroup<Position, Velocity>();
group.rebuild();

bench.measure("Group", [&]() {
    group.each([](auto e, auto& p, auto& v) {
        p.x += v.dx;
        p.y += v.dy;
    });
}, 1000);

bench.print_results();
bench.compare("View", "Group");
```

### Output Example

```
=== BENCHMARK RESULTS ===
Test Name                      Avg (μs)       Min (μs)       Max (μs)  Iterations
-----------------------------------------------------------------------------------
View                             125.50         118.20         142.30        1000
Group                             42.30          39.10          48.70        1000

=== COMPARISON ===
Group is 2.97× faster than View
```

### Benchmark System Performance

```cpp
void benchmark_systems(ECS::Registry& registry) {
    ECS::Benchmark bench;
    
    // Benchmark physics system
    bench.measure("Physics", [&]() {
        registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
            p.x += v.dx * 0.016f;
            p.y += v.dy * 0.016f;
        });
    }, 100);
    
    // Benchmark collision system
    bench.measure("Collision", [&]() {
        auto entities = registry.view<Position, Collider>().getEntities();
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                // Check collision
            }
        }
    }, 100);
    
    // Benchmark render system
    bench.measure("Render", [&]() {
        registry.view<Position, Sprite>().each([](auto e, auto& p, auto& s) {
            // Render sprite at position
        });
    }, 100);
    
    bench.print_results();
}
```

### Component Access Patterns

```cpp
void benchmark_access_patterns(ECS::Registry& registry) {
    ECS::Benchmark bench;
    
    std::vector<Entity> entities;
    for (int i = 0; i < 10000; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, i * 1.0f, i * 1.0f);
        entities.push_back(e);
    }
    
    // Sequential access (cache-friendly)
    bench.measure("Sequential", [&]() {
        registry.view<Position>().each([](auto e, auto& p) {
            p.x += 1.0f;
        });
    }, 1000);
    
    // Random access (cache-unfriendly)
    bench.measure("Random", [&]() {
        for (auto e : entities) {
            if (registry.hasComponent<Position>(e)) {
                auto& p = registry.getComponent<Position>(e);
                p.x += 1.0f;
            }
        }
    }, 1000);
    
    bench.print_results();
    bench.compare("Sequential", "Random");
}
```

### Parallel vs Sequential

```cpp
void benchmark_parallelism(ECS::Registry& registry) {
    ECS::Benchmark bench;
    
    // Create large dataset
    for (int i = 0; i < 100000; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, i * 1.0f, i * 1.0f);
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    }
    
    // Sequential
    bench.measure("Sequential", [&]() {
        registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
            p.x += v.dx;
            p.y += v.dy;
        });
    }, 100);
    
    // Parallel
    bench.measure("Parallel", [&]() {
        registry.parallelView<Position, Velocity>().each([](auto e, auto& p, auto& v) {
            p.x += v.dx;
            p.y += v.dy;
        });
    }, 100);
    
    bench.print_results();
    bench.compare("Sequential", "Parallel");
}
```

## Advanced Usage

### Custom Benchmarking

```cpp
class GameBenchmark {
    ECS::Benchmark bench;
    std::chrono::high_resolution_clock::time_point frame_start;
    std::vector<double> frame_times;
    
public:
    void start_frame() {
        frame_start = std::chrono::high_resolution_clock::now();
    }
    
    void end_frame() {
        auto frame_end = std::chrono::high_resolution_clock::now();
        double frame_time = std::chrono::duration<double, std::milli>(
            frame_end - frame_start).count();
        frame_times.push_back(frame_time);
    }
    
    void print_frame_stats() {
        double sum = 0.0, min_time = frame_times[0], max_time = frame_times[0];
        for (double t : frame_times) {
            sum += t;
            min_time = std::min(min_time, t);
            max_time = std::max(max_time, t);
        }
        
        double avg = sum / frame_times.size();
        std::cout << "Frame Stats:\n"
                  << "  Average: " << avg << " ms (" << (1000.0 / avg) << " FPS)\n"
                  << "  Min: " << min_time << " ms\n"
                  << "  Max: " << max_time << " ms\n";
    }
    
    void benchmark_system(const std::string& name, std::function<void()> system) {
        bench.measure(name, system, 100);
    }
    
    void print_system_stats() {
        bench.print_results();
    }
};
```

### Memory Profiling

```cpp
struct MemoryStats {
    size_t entity_count;
    size_t component_memory;
    size_t total_memory;
};

MemoryStats get_memory_stats(ECS::Registry& registry) {
    MemoryStats stats{};
    
    // Count entities
    registry.view<Position>().each([&](auto e, auto&) {
        stats.entity_count++;
    });
    
    // Estimate component memory
    stats.component_memory += registry.component_count<Position>() * sizeof(Position);
    stats.component_memory += registry.component_count<Velocity>() * sizeof(Velocity);
    // ... other components
    
    stats.total_memory = stats.component_memory + 
                        stats.entity_count * sizeof(ECS::Entity);
    
    return stats;
}

void print_memory_stats(const MemoryStats& stats) {
    std::cout << "Memory Usage:\n"
              << "  Entities: " << stats.entity_count << "\n"
              << "  Component Memory: " << (stats.component_memory / 1024) << " KB\n"
              << "  Total: " << (stats.total_memory / 1024) << " KB\n";
}
```

### Profiling Integration

```cpp
class ProfiledSystem {
    std::string name;
    std::function<void(ECS::Registry&)> func;
    double total_time_us = 0.0;
    size_t call_count = 0;
    
public:
    ProfiledSystem(std::string n, std::function<void(ECS::Registry&)> f)
        : name(std::move(n)), func(std::move(f)) {}
    
    void execute(ECS::Registry& registry) {
        auto start = std::chrono::high_resolution_clock::now();
        func(registry);
        auto end = std::chrono::high_resolution_clock::now();
        
        double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();
        total_time_us += elapsed_us;
        call_count++;
    }
    
    void print_stats() const {
        double avg_us = total_time_us / call_count;
        std::cout << name << ": " 
                  << avg_us << " μs avg (" 
                  << call_count << " calls)\n";
    }
};
```

## Performance Tips

### What to Benchmark

1. **System execution time**
2. **Component iteration speed**
3. **Entity creation/destruction**
4. **Component add/remove**
5. **View vs Group performance**
6. **Sequential vs parallel iteration**

### Benchmark Best Practices

```cpp
// ✅ Good: Warm up before benchmarking
for (int i = 0; i < 10; ++i) {
    // Run code without measuring
}
bench.measure("Test", []() { /* ... */ });

// ✅ Good: Multiple iterations
bench.measure("Test", []() { /* ... */ }, 1000);

// ✅ Good: Isolate what you're measuring
bench.measure("Component Access", [&]() {
    // Only measure component access, not entity creation
});

// ❌ Bad: Too few iterations
bench.measure("Test", []() { /* ... */ }, 1); // Unreliable

// ❌ Bad: Including setup in measurement
bench.measure("Test", [&]() {
    auto entities = create_test_entities(); // Don't include setup!
    process(entities);
});
```

## Interpreting Results

### Speedup Calculation

```cpp
// If System A takes 100μs and System B takes 50μs:
// Speedup = 100 / 50 = 2.0× faster
```

### When to Optimize

- Average time > target frame time
- Maximum time causes frame drops
- Significant variation (max >> avg)

### Common Bottlenecks

1. **Cache misses**: Random access patterns
2. **Allocations**: Entity/component creation in hot loops
3. **Lock contention**: Concurrent access to shared data
4. **Redundant work**: Processing unchanged data

## See Also

- [Optimization Guide](14_optimization.md) - Performance best practices
- [Parallel Processing](05_parallel_processing.md) - Multi-threading
- [Views](04_views.md) - Query optimization
