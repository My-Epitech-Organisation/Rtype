# API Quick Reference

## Registry

### Entity Management
```cpp
Entity spawn_entity();
void kill_entity(Entity entity);
bool is_alive(Entity entity) const;
size_t cleanup_tombstones();
void reserve_entities(size_t capacity);
```

### Component Management
```cpp
template<typename T, typename... Args>
T& emplace_component(Entity entity, Args&&... args);

template<typename T>
void remove_component(Entity entity);

template<typename T>
bool has_component(Entity entity) const;

template<typename T>
T& get_component(Entity entity);

template<typename... Ts>
bool all_of(Entity entity) const;

template<typename... Ts>
bool any_of(Entity entity) const;
```

### View Creation
```cpp
template<typename... Components>
View<Components...> view();

template<typename... Components>
ParallelView<Components...> parallel_view();

template<typename... Components>
Group<Components...> create_group();
```

### Resource Management
```cpp
template<typename T, typename... Args>
T& set_resource(Args&&... args);

template<typename T>
T& get_resource();

template<typename T>
bool has_resource() const;
```

### Signal Registration
```cpp
template<typename T>
void on_construct(std::function<void(Entity)> callback);

template<typename T>
void on_destroy(std::function<void(Entity)> callback);
```

## Entity

### Properties
```cpp
std::uint32_t index() const;
std::uint32_t generation() const;
bool is_null() const;
bool is_tombstone() const;
```

### Operators
```cpp
auto operator<=>(const Entity&) const; // Three-way comparison
```

## View

### Iteration
```cpp
template<typename Func>
void each(Func&& func);

template<typename... Excluded>
auto exclude();
```

## ParallelView

### Parallel Iteration
```cpp
template<typename Func>
void each(Func&& func);
```

## Group

### Management
```cpp
void rebuild();
size_t size() const;
bool empty() const;
const std::vector<Entity>& get_entities() const;
```

### Iteration
```cpp
template<typename Func>
void each(Func&& func);

auto begin() const;
auto end() const;
```

## CommandBuffer

### Entity Operations
```cpp
Entity spawn_entity_deferred();
void destroy_entity_deferred(Entity entity);
```

### Component Operations
```cpp
template<typename T, typename... Args>
void emplace_component_deferred(Entity entity, Args&&... args);

template<typename T>
void remove_component_deferred(Entity entity);
```

### Execution
```cpp
void flush();
size_t pending_count() const;
void clear();
```

## SystemScheduler

### System Management
```cpp
void add_system(const std::string& name, 
                SystemFunc func,
                const std::vector<std::string>& dependencies = {});

void remove_system(const std::string& name);
void clear();
```

### Execution
```cpp
void run();
void run_system(const std::string& name);
```

### Control
```cpp
void set_system_enabled(const std::string& name, bool enabled);
bool is_system_enabled(const std::string& name) const;
std::vector<std::string> get_execution_order() const;
```

## RelationshipManager

### Setting Relationships
```cpp
bool set_parent(Entity child, Entity parent);
void remove_parent(Entity child);
```

### Querying
```cpp
std::optional<Entity> get_parent(Entity child) const;
bool has_parent(Entity child) const;
std::vector<Entity> get_children(Entity parent) const;
std::vector<Entity> get_descendants(Entity parent) const;
std::vector<Entity> get_ancestors(Entity child) const;
Entity get_root(Entity entity) const;
bool is_ancestor(Entity potential_ancestor, Entity entity) const;
```

## PrefabManager

### Registration
```cpp
void register_prefab(const std::string& name, PrefabFunc func);
void unregister_prefab(const std::string& name);
bool has_prefab(const std::string& name) const;
std::vector<std::string> get_prefab_names() const;
void clear();
```

### Instantiation
```cpp
Entity instantiate(const std::string& name);
Entity instantiate(const std::string& name, PrefabFunc customizer);
std::vector<Entity> instantiate_multiple(const std::string& name, size_t count);
```

## Serializer

### Serializer Management
```cpp
template<typename T>
void register_serializer(std::shared_ptr<IComponentSerializer> serializer);
```

### File Operations
```cpp
bool save_to_file(const std::string& filename);
bool load_from_file(const std::string& filename, bool clear_existing = true);
```

### String Operations
```cpp
std::string serialize();
bool deserialize(const std::string& data, bool clear_existing = true);
```

## IComponentSerializer

### Interface
```cpp
virtual std::string serialize(Entity entity, Registry& registry) const = 0;
virtual void deserialize(Entity entity, const std::string& data, Registry& registry) const = 0;
```

## Benchmark

### Measurement
```cpp
template<typename Func>
void measure(const std::string& name, Func&& func, size_t iterations = 100);
```

### Results
```cpp
void print_results() const;
void compare(const std::string& name1, const std::string& name2) const;
const std::vector<Result>& get_results() const;
```

## Common Patterns

### Basic Entity Creation
```cpp
auto entity = registry.spawn_entity();
registry.emplace_component<Position>(entity, 0.0f, 0.0f);
registry.emplace_component<Velocity>(entity, 1.0f, 0.0f);
```

### View Iteration
```cpp
registry.view<Position, Velocity>().each([](Entity e, Position& p, Velocity& v) {
    p.x += v.dx;
    p.y += v.dy;
});
```

### Parallel Iteration
```cpp
registry.parallel_view<Position, Velocity>().each([](Entity e, Position& p, Velocity& v) {
    p.x += v.dx;
    p.y += v.dy;
});
```

### Deferred Operations
```cpp
CommandBuffer cmd(registry);
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        cmd.destroy_entity_deferred(e);
    }
});
cmd.flush();
```

### System Scheduling
```cpp
SystemScheduler scheduler(registry);
scheduler.add_system("physics", physics_system);
scheduler.add_system("collision", collision_system, {"physics"});
scheduler.add_system("render", render_system, {"collision"});
scheduler.run();
```

### Prefab Instantiation
```cpp
PrefabManager prefabs(registry);
prefabs.register_prefab("Enemy", [](Registry& r, Entity e) {
    r.emplace_component<Position>(e, 0.0f, 0.0f);
    r.emplace_component<Enemy>(e);
});
Entity enemy = prefabs.instantiate("Enemy");
```

### Serialization
```cpp
Serializer serializer(registry);
serializer.register_serializer<Position>(std::make_shared<PositionSerializer>());
serializer.save_to_file("save.dat");
serializer.load_from_file("save.dat");
```

## Type Aliases

```cpp
using SystemFunc = std::function<void(Registry&)>;
using PrefabFunc = std::function<void(Registry&, Entity)>;
using Callback = std::function<void(Entity)>;
```

## Constants

### Entity
```cpp
static constexpr std::uint32_t IndexBits = 20;
static constexpr std::uint32_t IndexMask = (1 << IndexBits) - 1;
static constexpr std::uint32_t GenerationBits = 12;
static constexpr std::uint32_t GenerationMask = (1 << GenerationBits) - 1;
static constexpr std::uint32_t MaxGeneration = GenerationMask;
static constexpr std::uint32_t NullID = std::numeric_limits<std::uint32_t>::max();
```

## Exceptions

Most ECS operations can throw `std::runtime_error` for:
- Operating on dead entities
- Missing components
- Invalid parameters
- Cyclic dependencies (RelationshipManager)
- Serialization errors

Always handle exceptions in production code:

```cpp
try {
    auto& component = registry.get_component<Position>(entity);
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << "\n";
}
```
