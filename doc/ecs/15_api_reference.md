# API Quick Reference

## Registry

### Entity Management
```cpp
Entity spawnEntity();
void killEntity(Entity entity);
bool isAlive(Entity entity) const;
size_t cleanupTombstones();
void reserveEntities(size_t capacity);
```

### Component Management
```cpp
template<typename T, typename... Args>
T& emplaceComponent(Entity entity, Args&&... args);

template<typename T>
void removeComponent(Entity entity);

template<typename T>
bool hasComponent(Entity entity) const;

template<typename T>
T& getComponent(Entity entity);

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
ParallelView<Components...> parallelView();

template<typename... Components>
Group<Components...> createGroup();
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
void onConstruct(std::function<void(Entity)> callback);

template<typename T>
void onDestroy(std::function<void(Entity)> callback);
```

## Entity

### Properties
```cpp
std::uint32_t index() const;
std::uint32_t generation() const;
bool isNull() const;
bool isTombstone() const;
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
const std::vector<Entity>& getEntities() const;
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
Entity spawnEntityDeferred();
void destroyEntityDeferred(Entity entity);
```

### Component Operations
```cpp
template<typename T, typename... Args>
void emplaceComponentDeferred(Entity entity, Args&&... args);

template<typename T>
void removeComponentDeferred(Entity entity);
```

### Execution
```cpp
void flush();
size_t pendingCount() const;
void clear();
```

## SystemScheduler

### System Management
```cpp
void addSystem(const std::string& name, 
                SystemFunc func,
                const std::vector<std::string>& dependencies = {});

void removeSystem(const std::string& name);
void clear();
```

### Execution
```cpp
void run();
void runSystem(const std::string& name);
```

### Control
```cpp
void setSystemEnabled(const std::string& name, bool enabled);
bool isSystemEnabled(const std::string& name) const;
std::vector<std::string> getExecutionOrder() const;
```

## RelationshipManager

### Setting Relationships
```cpp
bool setParent(Entity child, Entity parent);
void removeParent(Entity child);
```

### Querying
```cpp
std::optional<Entity> getParent(Entity child) const;
bool hasParent(Entity child) const;
std::vector<Entity> getChildren(Entity parent) const;
std::vector<Entity> getDescendants(Entity parent) const;
std::vector<Entity> getAncestors(Entity child) const;
Entity getRoot(Entity entity) const;
bool isAncestor(Entity potential_ancestor, Entity entity) const;
```

## PrefabManager

### Registration
```cpp
void registerPrefab(const std::string& name, PrefabFunc func);
void unregisterPrefab(const std::string& name);
bool hasPrefab(const std::string& name) const;
std::vector<std::string> getPrefabNames() const;
void clear();
```

### Instantiation
```cpp
Entity instantiate(const std::string& name);
Entity instantiate(const std::string& name, PrefabFunc customizer);
std::vector<Entity> instantiateMultiple(const std::string& name, size_t count);
```

## Serializer

### Serializer Management
```cpp
template<typename T>
void registerSerializer(std::shared_ptr<IComponentSerializer> serializer);
```

### File Operations
```cpp
bool saveToFile(const std::string& filename);
bool loadFromFile(const std::string& filename, bool clear_existing = true);
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
auto entity = registry.spawnEntity();
registry.emplaceComponent<Position>(entity, 0.0f, 0.0f);
registry.emplaceComponent<Velocity>(entity, 1.0f, 0.0f);
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
registry.parallelView<Position, Velocity>().each([](Entity e, Position& p, Velocity& v) {
    p.x += v.dx;
    p.y += v.dy;
});
```

### Deferred Operations
```cpp
CommandBuffer cmd(registry);
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        cmd.destroyEntityDeferred(e);
    }
});
cmd.flush();
```

### System Scheduling
```cpp
SystemScheduler scheduler(registry);
scheduler.addSystem("physics", physics_system);
scheduler.addSystem("collision", collision_system, {"physics"});
scheduler.addSystem("render", render_system, {"collision"});
scheduler.run();
```

### Prefab Instantiation
```cpp
PrefabManager prefabs(registry);
prefabs.registerPrefab("Enemy", [](Registry& r, Entity e) {
    r.emplaceComponent<Position>(e, 0.0f, 0.0f);
    r.emplaceComponent<Enemy>(e);
});
Entity enemy = prefabs.instantiate("Enemy");
```

### Serialization
```cpp
Serializer serializer(registry);
serializer.registerSerializer<Position>(std::make_shared<PositionSerializer>());
serializer.saveToFile("save.dat");
serializer.loadFromFile("save.dat");
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
    auto& component = registry.getComponent<Position>(entity);
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << "\n";
}
```
