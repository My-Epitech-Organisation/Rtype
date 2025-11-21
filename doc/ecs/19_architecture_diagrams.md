# ECS Architecture - UML and Sequence Diagrams

This document provides comprehensive UML diagrams to understand the complete architecture of the Entity Component System (ECS).

---

## Table of Contents

1. [Class Diagram - Complete Architecture](#class-diagram---complete-architecture)
2. [Component Diagram - Module Organization](#component-diagram---module-organization)
3. [Sequence Diagrams](#sequence-diagrams)
   - [Entity Lifecycle](#entity-lifecycle)
   - [Component Management](#component-management)
   - [View Iteration](#view-iteration)
   - [Parallel Processing](#parallel-processing)
   - [Signal Dispatching](#signal-dispatching)
4. [State Diagrams](#state-diagrams)
5. [Deployment Diagram](#deployment-diagram)

---

## Class Diagram - Complete Architecture

```mermaid
classDiagram
    %% ============================================================================
    %% CORE ENTITIES
    %% ============================================================================
    
    class Entity {
        <<struct>>
        +uint32_t id
        +static constexpr uint32_t IndexBits = 20
        +static constexpr uint32_t GenerationBits = 12
        +static constexpr uint32_t MaxGeneration
        +static constexpr uint32_t NullID
        
        +Entity()
        +Entity(uint32_t index, uint32_t generation)
        +uint32_t index() const
        +uint32_t generation() const
        +bool is_null() const
        +bool is_tombstone() const
        +operator<=>(const Entity&) const
    }
    
    %% ============================================================================
    %% REGISTRY - CENTRAL COORDINATOR
    %% ============================================================================
    
    class Registry {
        -unordered_map~uint32_t, vector~type_index~~ entity_components
        -vector~uint32_t~ generations
        -vector~uint32_t~ free_indices
        -vector~uint32_t~ tombstones
        -unordered_map~type_index, unique_ptr~ISparseSet~~ component_pools
        -unordered_map~type_index, any~ singletons
        -SignalDispatcher signal_dispatcher
        -RelationshipManager relationship_manager
        -shared_mutex entity_mutex
        -shared_mutex component_pool_mutex
        
        +Registry()
        
        %% Entity Management
        +void reserve_entities(size_t capacity)
        +Entity spawn_entity()
        +void kill_entity(Entity entity) noexcept
        +bool is_alive(Entity entity) const noexcept
        +size_t cleanup_tombstones()
        +size_t remove_entities_if(Func predicate)
        
        %% Component Management
        +void reserve_components~T~(size_t capacity)
        +void compact()
        +void compact_component~T~()
        +T& emplace_component~T~(Entity entity, Args... args)
        +T& get_or_emplace~T~(Entity entity, Args... args)
        +void remove_component~T~(Entity entity)
        +void clear_components~T~()
        +bool has_component~T~(Entity entity) const
        +size_t count_components~T~() const
        +T& get_component~T~(Entity entity)
        +void patch~T~(Entity entity, Func func)
        
        %% Signals
        +void on_construct~T~(function~void(Entity)~ callback)
        +void on_destroy~T~(function~void(Entity)~ callback)
        
        %% Views
        +View~Components...~ view()
        +ParallelView~Components...~ parallel_view()
        +Group~Components...~ create_group()
        
        %% Singletons
        +T& set_singleton~T~(Args... args)
        +T& get_singleton~T~()
        +bool has_singleton~T~() const
        +void remove_singleton~T~()
        
        %% Relationships
        +RelationshipManager& get_relationship_manager()
        
        -auto& get_sparse_set~T~()
        -const ISparseSet* get_sparse_set_const~T~() const
        -const auto& get_sparse_set_typed_const~T~() const
    }
    
    %% ============================================================================
    %% STORAGE - COMPONENT CONTAINERS
    %% ============================================================================
    
    class ISparseSet {
        <<interface>>
        +remove(Entity entity)*
        +contains(Entity entity) const*
        +clear()*
        +size() const*
        +shrink_to_fit()*
    }
    
    class SparseSet~T~ {
        -vector~T~ dense
        -vector~Entity~ packed
        -vector~size_t~ sparse
        -mutex sparse_set_mutex
        
        +bool contains(Entity entity) const
        +T& emplace(Entity entity, Args... args)
        +void remove(Entity entity)
        +T& get(Entity entity)
        +const T& get(Entity entity) const
        +void clear()
        +size_t size() const
        +void reserve(size_t capacity)
        +void shrink_to_fit()
        +const vector~Entity~& get_packed() const
        +iterator begin()
        +iterator end()
    }
    
    class TagSparseSet~T~ {
        -vector~Entity~ packed
        -vector~size_t~ sparse
        -mutex sparse_set_mutex
        -static T dummy_instance
        
        +bool contains(Entity entity) const
        +T& emplace(Entity entity, Args... args)
        +void remove(Entity entity)
        +T& get(Entity entity)
        +void clear()
        +size_t size() const
        +const vector~Entity~& get_packed() const
    }
    
    %% ============================================================================
    %% VIEWS - QUERY SYSTEM
    %% ============================================================================
    
    class View~Components...~ {
        -Registry& registry
        -tuple~SparseSet~Components~*...~ pools
        -size_t smallest_pool_index
        
        +View(Registry& registry)
        +void each(Func func)
        +ExcludeView exclude~Excluded...~()
        
        -void initialize_pools(index_sequence)
        -void each_impl(Func func, index_sequence)
        -size_t find_smallest_pool(index_sequence)
    }
    
    class ParallelView~Components...~ {
        -Registry& registry
        
        +ParallelView(Registry& registry)
        +void each(Func func)
    }
    
    class ExcludeView~Includes..., Excludes...~ {
        -Registry& registry
        -tuple~SparseSet~Includes~*...~ include_pools
        -vector~ISparseSet*~ exclude_pools
        -size_t smallest_pool_index
        
        +void each(Func func)
        
        -void each_impl(Func func, index_sequence)
        -bool is_excluded(Entity entity) const
    }
    
    class Group~Components...~ {
        -Registry& registry
        -vector~Entity~ entities
        
        +Group(Registry& reg)
        +void rebuild()
        +void each(Func func)
        +const vector~Entity~& get_entities() const
    }
    
    %% ============================================================================
    %% SIGNALS - EVENT SYSTEM
    %% ============================================================================
    
    class SignalDispatcher {
        -unordered_map~type_index, vector~Callback~~ construct_callbacks
        -unordered_map~type_index, vector~Callback~~ destroy_callbacks
        -shared_mutex callbacks_mutex
        
        +void register_construct(type_index type, Callback callback)
        +void register_destroy(type_index type, Callback callback)
        +void dispatch_construct(type_index type, Entity entity)
        +void dispatch_destroy(type_index type, Entity entity)
        +void clear_callbacks(type_index type)
        +void clear_all_callbacks()
    }
    
    %% ============================================================================
    %% RELATIONSHIPS - HIERARCHY SYSTEM
    %% ============================================================================
    
    class RelationshipManager {
        -unordered_map~uint32_t, Entity~ parent_map
        -unordered_map~uint32_t, unordered_set~Entity~~ children_map
        -shared_mutex relationship_mutex
        
        +bool set_parent(Entity child, Entity parent)
        +void remove_parent(Entity child)
        +optional~Entity~ get_parent(Entity child) const
        +bool has_parent(Entity child) const
        +vector~Entity~ get_children(Entity parent) const
        +vector~Entity~ get_descendants(Entity parent) const
        +vector~Entity~ get_ancestors(Entity child) const
        +Entity get_root(Entity entity) const
        +bool is_ancestor(Entity potential_ancestor, Entity entity) const
        +void remove_entity(Entity entity)
        +void clear()
        +size_t child_count(Entity parent) const
        +size_t get_depth(Entity entity) const
        
        -bool would_create_cycle(Entity child, Entity parent) const
        -void get_descendants_recursive(Entity parent, vector~Entity~& result) const
    }
    
    %% ============================================================================
    %% COMMAND BUFFER - DEFERRED OPERATIONS
    %% ============================================================================
    
    class CommandBuffer {
        -Registry& registry
        -vector~function~void()~~ commands
        -unordered_map~uint32_t, Entity~ placeholder_to_real
        -uint32_t next_placeholder_id
        -mutex commands_mutex
        
        +CommandBuffer(Registry& reg)
        +Entity spawn_entity_deferred()
        +void destroy_entity_deferred(Entity entity)
        +void emplace_component_deferred~T~(Entity entity, Args... args)
        +void remove_component_deferred~T~(Entity entity)
        +void flush()
        +size_t pending_count() const
        +void clear()
    }
    
    %% ============================================================================
    %% PREFAB SYSTEM
    %% ============================================================================
    
    class PrefabManager {
        -Registry& registry
        -unordered_map~string, PrefabFunc~ prefabs
        -shared_mutex prefab_mutex
        
        +PrefabManager(Registry& reg)
        +void register_prefab(string name, PrefabFunc func)
        +Entity instantiate(string name)
        +Entity instantiate(string name, PrefabFunc customizer)
        +vector~Entity~ instantiate_multiple(string name, size_t count)
        +bool has_prefab(string name) const
        +void unregister_prefab(string name)
        +vector~string~ get_prefab_names() const
        +void clear()
        +void create_from_entity(string name, Entity template_entity)
    }
    
    %% ============================================================================
    %% SYSTEM SCHEDULER
    %% ============================================================================
    
    class SystemScheduler {
        -Registry& registry
        -unordered_map~string, SystemNode~ systems
        -vector~string~ execution_order
        -bool needs_reorder
        
        +SystemScheduler(Registry& reg)
        +void add_system(string name, SystemFunc func, vector~string~ dependencies)
        +void remove_system(string name)
        +void run()
        +void run_system(string name)
        +void clear()
        +vector~string~ get_execution_order() const
        +void set_system_enabled(string name, bool enabled)
        +bool is_system_enabled(string name) const
        
        -void recompute_order()
        -void topological_sort()
        -bool has_cycle() const
    }
    
    class SystemNode {
        +string name
        +SystemFunc func
        +vector~string~ dependencies
        +bool enabled
    }
    
    %% ============================================================================
    %% SERIALIZATION
    %% ============================================================================
    
    class IComponentSerializer {
        <<interface>>
        +string serialize(Entity entity, Registry& registry) const*
        +void deserialize(Entity entity, string data, Registry& registry) const*
    }
    
    class Serializer {
        -Registry& registry
        -unordered_map~type_index, unique_ptr~IComponentSerializer~~ serializers
        
        +Serializer(Registry& reg)
        +void register_serializer~T~(unique_ptr~IComponentSerializer~ serializer)
        +bool save_to_file(string filename)
        +bool load_from_file(string filename, bool clear_existing)
        +string serialize()
        +bool deserialize(string data, bool clear_existing)
    }
    
    %% ============================================================================
    %% RELATIONSHIPS
    %% ============================================================================
    
    Registry *-- Entity : manages
    Registry *-- ISparseSet : owns
    Registry *-- SignalDispatcher : contains
    Registry *-- RelationshipManager : contains
    
    ISparseSet <|-- SparseSet : implements
    ISparseSet <|-- TagSparseSet : implements
    
    SparseSet --> Entity : stores
    TagSparseSet --> Entity : stores
    
    View --> Registry : observes
    View --> SparseSet : observes pools
    ParallelView --> Registry : observes
    ExcludeView --> Registry : observes
    ExcludeView --> ISparseSet : observes
    Group --> Registry : observes
    
    CommandBuffer --> Registry : defers operations
    PrefabManager --> Registry : creates entities
    SystemScheduler --> Registry : executes systems
    SystemScheduler *-- SystemNode : contains
    
    Serializer --> Registry : serializes/deserializes
    Serializer --> IComponentSerializer : uses
    
    SignalDispatcher --> Entity : notifies about
```

---

## Component Diagram - Module Organization

```mermaid
graph TB
    subgraph Core["Core Module"]
        Entity[Entity.hpp<br/>Type-safe entity ID]
        Registry[Registry/<br/>Central coordinator]
        CommandBuffer[CommandBuffer<br/>Deferred operations]
        Prefab[Prefab<br/>Entity templates]
        Relationship[Relationship<br/>Hierarchies]
    end
    
    subgraph Storage["Storage Module"]
        ISparseSet[ISparseSet<br/>Storage interface]
        SparseSet[SparseSet<br/>Component storage]
        TagSparseSet[TagSparseSet<br/>Tag storage]
    end
    
    subgraph View["View Module"]
        ViewClass[View<br/>Query iteration]
        ParallelView[ParallelView<br/>Parallel iteration]
        ExcludeView[ExcludeView<br/>Filtered iteration]
        Group[Group<br/>Cached queries]
    end
    
    subgraph Signal["Signal Module"]
        SignalDispatcher[SignalDispatcher<br/>Event system]
    end
    
    subgraph System["System Module"]
        SystemScheduler[SystemScheduler<br/>System execution]
    end
    
    subgraph Serialization["Serialization Module"]
        Serializer[Serializer<br/>Save/Load]
        IComponentSerializer[IComponentSerializer<br/>Component serializer]
    end
    
    subgraph Utils["Utils Module"]
        Benchmark[Benchmark<br/>Performance tools]
    end
    
    Registry --> Entity
    Registry --> Storage
    Registry --> Signal
    Registry --> Relationship
    
    View --> Registry
    View --> Storage
    
    CommandBuffer --> Registry
    Prefab --> Registry
    SystemScheduler --> Registry
    Serializer --> Registry
    
    Storage --> Entity
    View --> Entity
    Signal --> Entity
    Relationship --> Entity
```

---

## Sequence Diagrams

### Entity Lifecycle

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant EntitySystem as Entity System
    participant ComponentPools as Component Pools
    participant SignalDispatcher
    participant RelationshipManager
    
    %% Entity Creation
    Note over Client,RelationshipManager: Entity Creation Flow
    
    Client->>+Registry: spawn_entity()
    Registry->>Registry: lock(entity_mutex)
    
    alt Free indices available
        Registry->>EntitySystem: Get from free_indices
        Registry->>EntitySystem: Validate generation < MaxGeneration
        Registry->>EntitySystem: Create Entity(index, generation)
    else No free indices
        Registry->>EntitySystem: Allocate new index
        Registry->>EntitySystem: Create Entity(index, 0)
    end
    
    Registry->>Registry: entity_components[index] = []
    Registry->>Registry: unlock(entity_mutex)
    Registry-->>-Client: Entity
    
    %% Component Addition
    Note over Client,RelationshipManager: Component Addition Flow
    
    Client->>+Registry: emplace_component<Position>(entity, x, y)
    Registry->>Registry: is_alive(entity)?
    
    alt Entity is dead
        Registry-->>Client: throw runtime_error
    end
    
    Registry->>Registry: lock(entity_mutex)
    Registry->>Registry: entity_components[index].push_back(type)
    Registry->>Registry: unlock(entity_mutex)
    
    Registry->>+ComponentPools: get_sparse_set<Position>()
    ComponentPools->>ComponentPools: emplace(entity, x, y)
    ComponentPools-->>-Registry: Position&
    
    Registry->>+SignalDispatcher: dispatch_construct(type, entity)
    SignalDispatcher->>SignalDispatcher: Get callbacks for type
    
    loop For each callback
        SignalDispatcher->>Client: callback(entity)
    end
    
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: Position&
    
    %% Entity Destruction
    Note over Client,RelationshipManager: Entity Destruction Flow
    
    Client->>+Registry: kill_entity(entity)
    Registry->>Registry: lock(entity_mutex)
    Registry->>Registry: Validate entity
    Registry->>Registry: Collect components to remove
    Registry->>Registry: Increment generation
    Registry->>Registry: Add to free_indices
    Registry->>Registry: entity_components.erase(index)
    Registry->>Registry: unlock(entity_mutex)
    
    loop For each component
        Registry->>+SignalDispatcher: dispatch_destroy(type, entity)
        SignalDispatcher->>Client: callback(entity)
        SignalDispatcher-->>-Registry: void
        
        Registry->>+ComponentPools: remove(entity)
        ComponentPools-->>-Registry: void
    end
    
    Registry->>+RelationshipManager: remove_entity(entity)
    RelationshipManager->>RelationshipManager: Remove parent/child links
    RelationshipManager-->>-Registry: void
    
    Registry-->>-Client: void
```

### Component Management

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant SparseSet as SparseSet<T>
    participant SignalDispatcher
    
    %% Get or Emplace Pattern
    Note over Client,SignalDispatcher: Get or Emplace Pattern
    
    Client->>+Registry: get_or_emplace<Velocity>(entity, dx, dy)
    Registry->>+SparseSet: contains(entity)?
    SparseSet-->>-Registry: bool
    
    alt Component exists
        Registry->>+SparseSet: get(entity)
        SparseSet-->>-Registry: Velocity&
        Registry-->>Client: Velocity& (existing)
    else Component missing
        Registry->>Registry: lock(entity_mutex)
        Registry->>Registry: entity_components[index].push_back(type)
        Registry->>Registry: unlock(entity_mutex)
        
        Registry->>+SparseSet: emplace(entity, dx, dy)
        SparseSet->>SparseSet: lock(sparse_set_mutex)
        
        alt Entity index >= sparse.size()
            SparseSet->>SparseSet: sparse.resize(index + 1)
        end
        
        SparseSet->>SparseSet: sparse[index] = dense.size()
        SparseSet->>SparseSet: packed.push_back(entity)
        SparseSet->>SparseSet: dense.emplace_back(dx, dy)
        SparseSet->>SparseSet: unlock(sparse_set_mutex)
        SparseSet-->>-Registry: Velocity&
        
        Registry->>+SignalDispatcher: dispatch_construct(type, entity)
        SignalDispatcher-->>-Registry: void
        
        Registry-->>Client: Velocity& (new)
    end
    
    Registry-->>-Client: Velocity&
    
    %% Component Removal
    Note over Client,SignalDispatcher: Component Removal
    
    Client->>+Registry: remove_component<Velocity>(entity)
    
    Registry->>+SignalDispatcher: dispatch_destroy(type, entity)
    SignalDispatcher->>SignalDispatcher: Execute callbacks
    SignalDispatcher-->>-Registry: void
    
    Registry->>+SparseSet: remove(entity)
    SparseSet->>SparseSet: lock(sparse_set_mutex)
    SparseSet->>SparseSet: dense_index = sparse[entity.index()]
    SparseSet->>SparseSet: Swap with last element
    SparseSet->>SparseSet: dense.pop_back()
    SparseSet->>SparseSet: packed.pop_back()
    SparseSet->>SparseSet: Update swapped element's sparse index
    SparseSet->>SparseSet: sparse[entity.index()] = NullIndex
    SparseSet->>SparseSet: unlock(sparse_set_mutex)
    SparseSet-->>-Registry: void
    
    Registry->>Registry: lock(entity_mutex)
    Registry->>Registry: entity_components[index].erase(type)
    Registry->>Registry: unlock(entity_mutex)
    
    Registry-->>-Client: void
```

### View Iteration

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant View
    participant SparseSet1 as SparseSet<Position>
    participant SparseSet2 as SparseSet<Velocity>
    
    %% View Creation
    Note over Client,SparseSet2: View Creation and Iteration
    
    Client->>+Registry: view<Position, Velocity>()
    Registry->>+View: View(registry)
    
    View->>+Registry: get_sparse_set<Position>()
    Registry-->>-View: SparseSet<Position>*
    
    View->>+Registry: get_sparse_set<Velocity>()
    Registry-->>-View: SparseSet<Velocity>*
    
    View->>View: pools = {pos_pool*, vel_pool*}
    View->>View: Find smallest pool
    
    View->>+SparseSet1: get_packed().size()
    SparseSet1-->>-View: 100
    
    View->>+SparseSet2: get_packed().size()
    SparseSet2-->>-View: 80
    
    View->>View: smallest_pool_index = 1 (Velocity)
    
    View-->>-Registry: View<Position, Velocity>
    Registry-->>-Client: View<Position, Velocity>
    
    %% View Iteration
    Note over Client,SparseSet2: Iteration over Entities
    
    Client->>+View: each([](Entity e, Position& p, Velocity& v) {...})
    
    View->>+SparseSet2: get_packed()
    SparseSet2-->>-View: vector<Entity>& (80 entities)
    
    loop For each entity in smallest pool
        View->>View: entity = packed[i]
        
        View->>+SparseSet1: contains(entity)?
        SparseSet1-->>-View: true
        
        View->>+SparseSet2: contains(entity)?
        SparseSet2-->>-View: true
        
        alt All pools contain entity
            View->>+SparseSet1: get(entity)
            SparseSet1-->>-View: Position&
            
            View->>+SparseSet2: get(entity)
            SparseSet2-->>-View: Velocity&
            
            View->>Client: callback(entity, pos, vel)
            Client->>Client: p.x += v.dx
        end
    end
    
    View-->>-Client: void
```

### Parallel Processing

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant ParallelView
    participant Thread1
    participant Thread2
    participant ThreadN
    participant SparseSet as SparseSet<Position>
    
    Note over Client,SparseSet: Parallel View Setup
    
    Client->>+Registry: parallel_view<Position>()
    Registry->>+ParallelView: ParallelView(registry)
    ParallelView-->>-Registry: ParallelView
    Registry-->>-Client: ParallelView<Position>
    
    Note over Client,SparseSet: Parallel Execution
    
    Client->>+ParallelView: each([](Entity e, Position& p) {...})
    
    ParallelView->>+Registry: get_sparse_set<Position>()
    Registry-->>-ParallelView: SparseSet<Position>*
    
    ParallelView->>+SparseSet: get_packed()
    SparseSet-->>-ParallelView: vector<Entity>& (10000 entities)
    
    ParallelView->>ParallelView: num_threads = hardware_concurrency()
    ParallelView->>ParallelView: chunk_size = 10000 / num_threads
    
    Note over Thread1,ThreadN: Spawn Worker Threads
    
    par Thread 1 (entities 0-3333)
        ParallelView->>+Thread1: spawn([&, start=0, end=3333]() {...})
        loop i = 0 to 3333
            Thread1->>+SparseSet: contains(entities[i])?
            SparseSet-->>-Thread1: true
            Thread1->>+SparseSet: get(entities[i])
            SparseSet-->>-Thread1: Position&
            Thread1->>Thread1: callback(entity, pos)
        end
        Thread1-->>-ParallelView: void
    and Thread 2 (entities 3334-6666)
        ParallelView->>+Thread2: spawn([&, start=3334, end=6666]() {...})
        loop i = 3334 to 6666
            Thread2->>+SparseSet: contains(entities[i])?
            SparseSet-->>-Thread2: true
            Thread2->>+SparseSet: get(entities[i])
            SparseSet-->>-Thread2: Position&
            Thread2->>Thread2: callback(entity, pos)
        end
        Thread2-->>-ParallelView: void
    and Thread N (entities 6667-10000)
        ParallelView->>+ThreadN: spawn([&, start=6667, end=10000]() {...})
        loop i = 6667 to 10000
            ThreadN->>+SparseSet: contains(entities[i])?
            SparseSet-->>-ThreadN: true
            ThreadN->>+SparseSet: get(entities[i])
            SparseSet-->>-ThreadN: Position&
            ThreadN->>ThreadN: callback(entity, pos)
        end
        ThreadN-->>-ParallelView: void
    end
    
    Note over Thread1,ThreadN: Join All Threads
    
    ParallelView->>Thread1: join()
    ParallelView->>Thread2: join()
    ParallelView->>ThreadN: join()
    
    ParallelView-->>-Client: void
```

### Signal Dispatching

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant SignalDispatcher
    participant System1 as Physics System
    participant System2 as Audio System
    participant System3 as Render System
    
    %% Signal Registration
    Note over Client,System3: Signal Registration Phase
    
    Client->>+Registry: on_construct<Sprite>(callback1)
    Registry->>+SignalDispatcher: register_construct(typeid(Sprite), callback1)
    SignalDispatcher->>SignalDispatcher: lock(callbacks_mutex)
    SignalDispatcher->>SignalDispatcher: construct_callbacks[Sprite].push_back(callback1)
    SignalDispatcher->>SignalDispatcher: unlock(callbacks_mutex)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    Client->>+Registry: on_construct<Sprite>(callback2)
    Registry->>+SignalDispatcher: register_construct(typeid(Sprite), callback2)
    SignalDispatcher->>SignalDispatcher: construct_callbacks[Sprite].push_back(callback2)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    Client->>+Registry: on_destroy<Sprite>(cleanup_callback)
    Registry->>+SignalDispatcher: register_destroy(typeid(Sprite), cleanup_callback)
    SignalDispatcher->>SignalDispatcher: destroy_callbacks[Sprite].push_back(cleanup_callback)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    %% Component Addition Triggering Signals
    Note over Client,System3: Component Addition Event
    
    Client->>+Registry: emplace_component<Sprite>(entity, texture)
    
    Registry->>Registry: Add component to entity
    Registry->>Registry: Store in SparseSet
    
    Registry->>+SignalDispatcher: dispatch_construct(typeid(Sprite), entity)
    
    SignalDispatcher->>SignalDispatcher: lock(callbacks_mutex)
    SignalDispatcher->>SignalDispatcher: Copy callbacks for Sprite
    SignalDispatcher->>SignalDispatcher: unlock(callbacks_mutex)
    
    Note over SignalDispatcher,System3: Execute callbacks WITHOUT holding lock
    
    SignalDispatcher->>+System1: callback1(entity)
    System1->>System1: Initialize physics properties
    System1-->>-SignalDispatcher: void
    
    SignalDispatcher->>+System2: callback2(entity)
    System2->>System2: Load audio resources
    System2-->>-SignalDispatcher: void
    
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: Sprite&
    
    %% Component Removal Triggering Signals
    Note over Client,System3: Component Removal Event
    
    Client->>+Registry: remove_component<Sprite>(entity)
    
    Registry->>+SignalDispatcher: dispatch_destroy(typeid(Sprite), entity)
    
    SignalDispatcher->>SignalDispatcher: Copy callbacks for Sprite
    
    SignalDispatcher->>+System3: cleanup_callback(entity)
    System3->>System3: Release GPU resources
    System3->>System3: Unload textures
    System3-->>-SignalDispatcher: void
    
    SignalDispatcher-->>-Registry: void
    
    Registry->>Registry: Remove from SparseSet
    Registry->>Registry: Remove from entity_components
    
    Registry-->>-Client: void
```

### Command Buffer Deferred Operations

```mermaid
sequenceDiagram
    participant Client
    participant ParallelView
    participant Thread1
    participant Thread2
    participant CommandBuffer
    participant Registry
    
    Note over Client,Registry: Setup Phase
    
    Client->>+CommandBuffer: CommandBuffer(registry)
    CommandBuffer-->>-Client: CommandBuffer
    
    Note over Client,Registry: Parallel Iteration with Deferred Operations
    
    Client->>+Registry: parallel_view<Health>()
    Registry-->>-Client: ParallelView
    
    Client->>ParallelView: each([&cmd](Entity e, Health& h) {...})
    
    par Thread 1 Processing
        ParallelView->>+Thread1: Process entities 0-500
        loop For entities in chunk
            Thread1->>Thread1: if (health.value <= 0)
            Thread1->>+CommandBuffer: destroy_entity_deferred(entity)
            CommandBuffer->>CommandBuffer: lock(commands_mutex)
            CommandBuffer->>CommandBuffer: commands.push_back(lambda)
            CommandBuffer->>CommandBuffer: unlock(commands_mutex)
            CommandBuffer-->>-Thread1: void
        end
        Thread1-->>-ParallelView: Done
    and Thread 2 Processing
        ParallelView->>+Thread2: Process entities 501-1000
        loop For entities in chunk
            Thread2->>Thread2: if (health.value <= 0)
            Thread2->>+CommandBuffer: destroy_entity_deferred(entity)
            CommandBuffer->>CommandBuffer: lock(commands_mutex)
            CommandBuffer->>CommandBuffer: commands.push_back(lambda)
            CommandBuffer->>CommandBuffer: unlock(commands_mutex)
            CommandBuffer-->>-Thread2: void
        end
        Thread2-->>-ParallelView: Done
    end
    
    Note over Client,Registry: Flush Phase (After Parallel Iteration)
    
    Client->>+CommandBuffer: flush()
    
    CommandBuffer->>CommandBuffer: lock(commands_mutex)
    CommandBuffer->>CommandBuffer: Copy commands vector
    CommandBuffer->>CommandBuffer: unlock(commands_mutex)
    
    loop For each deferred command
        CommandBuffer->>+Registry: kill_entity(entity)
        Registry->>Registry: Remove components
        Registry->>Registry: Update generations
        Registry->>Registry: Add to free_indices
        Registry-->>-CommandBuffer: void
    end
    
    CommandBuffer->>CommandBuffer: commands.clear()
    CommandBuffer->>CommandBuffer: placeholder_to_real.clear()
    CommandBuffer->>CommandBuffer: next_placeholder_id = 0
    
    CommandBuffer-->>-Client: void
```

### System Scheduler Execution

```mermaid
sequenceDiagram
    participant Client
    participant SystemScheduler
    participant Registry
    participant PhysicsSystem
    participant CollisionSystem
    participant RenderSystem
    
    Note over Client,RenderSystem: System Registration Phase
    
    Client->>+SystemScheduler: add_system("physics", physics_func, {})
    SystemScheduler->>SystemScheduler: systems["physics"] = {func, {}, enabled=true}
    SystemScheduler->>SystemScheduler: needs_reorder = true
    SystemScheduler-->>-Client: void
    
    Client->>+SystemScheduler: add_system("collision", collision_func, {"physics"})
    SystemScheduler->>SystemScheduler: systems["collision"] = {func, {"physics"}, enabled=true}
    SystemScheduler->>SystemScheduler: needs_reorder = true
    SystemScheduler-->>-Client: void
    
    Client->>+SystemScheduler: add_system("render", render_func, {"collision"})
    SystemScheduler->>SystemScheduler: systems["render"] = {func, {"collision"}, enabled=true}
    SystemScheduler->>SystemScheduler: needs_reorder = true
    SystemScheduler-->>-Client: void
    
    Note over Client,RenderSystem: System Execution Phase
    
    Client->>+SystemScheduler: run()
    
    alt needs_reorder == true
        SystemScheduler->>SystemScheduler: topological_sort()
        SystemScheduler->>SystemScheduler: execution_order = ["physics", "collision", "render"]
        SystemScheduler->>SystemScheduler: needs_reorder = false
    end
    
    Note over SystemScheduler,RenderSystem: Execute systems in dependency order
    
    SystemScheduler->>SystemScheduler: Get "physics" from systems
    SystemScheduler->>+PhysicsSystem: physics_func(registry)
    PhysicsSystem->>+Registry: view<Position, Velocity>()
    Registry-->>-PhysicsSystem: View
    PhysicsSystem->>PhysicsSystem: Update positions
    PhysicsSystem-->>-SystemScheduler: void
    
    SystemScheduler->>SystemScheduler: Get "collision" from systems
    SystemScheduler->>+CollisionSystem: collision_func(registry)
    CollisionSystem->>+Registry: view<Position, Collider>()
    Registry-->>-CollisionSystem: View
    CollisionSystem->>CollisionSystem: Check collisions
    CollisionSystem-->>-SystemScheduler: void
    
    SystemScheduler->>SystemScheduler: Get "render" from systems
    SystemScheduler->>+RenderSystem: render_func(registry)
    RenderSystem->>+Registry: view<Transform, Sprite>()
    Registry-->>-RenderSystem: View
    RenderSystem->>RenderSystem: Draw sprites
    RenderSystem-->>-SystemScheduler: void
    
    SystemScheduler-->>-Client: void
```

---

## State Diagrams

### Entity State Machine

```mermaid
stateDiagram-v2
    [*] --> Free: spawn_entity()
    
    Free: Free Slot
    Alive: Alive Entity
    Dead: Dead Entity
    Tombstone: Tombstone (Max Generation)
    
    Free --> Alive: Allocate<br/>generation=0
    
    Alive --> Dead: kill_entity()<br/>generation++
    
    Dead --> Alive: spawn_entity()<br/>Reuse index
    Dead --> Tombstone: generation >= MaxGeneration
    
    Tombstone --> Free: cleanup_tombstones()<br/>Reset generation
    
    Alive --> Alive: add/remove components
    
    note right of Alive
        Entity can have components
        Can be queried by views
        Valid for operations
    end note
    
    note right of Dead
        Invalid entity ID
        Components removed
        Index in free_indices
    end note
    
    note right of Tombstone
        Cannot be reused
        Permanently retired
        Needs manual cleanup
    end note
```

### Component Lifecycle

```mermaid
stateDiagram-v2
    [*] --> NonExistent
    
    NonExistent: Component Does Not Exist
    Constructing: Creating Component
    Active: Component Active
    Destroying: Removing Component
    
    NonExistent --> Constructing: emplace_component()
    
    Constructing --> Active: on_construct signals fired
    
    Active --> Active: patch()<br/>get_component()<br/>Modifications
    
    Active --> Destroying: remove_component()<br/>kill_entity()
    
    Destroying --> NonExistent: on_destroy signals fired<br/>Memory freed
    
    NonExistent --> Active: get_or_emplace()<br/>(if doesn't exist)
    
    Active --> Active: get_or_emplace()<br/>(if exists)
    
    note right of Active
        Component is in SparseSet
        Accessible via get_component()
        Entity tracked in entity_components
    end note
    
    note right of Constructing
        Callbacks can initialize
        Dependencies can be set up
        Resources can be allocated
    end note
    
    note right of Destroying
        Callbacks can cleanup
        Resources can be freed
        Dependencies removed
    end note
```

---

## Deployment Diagram

```mermaid
graph TB
    subgraph Application["Application Layer"]
        GameLoop[Game Loop<br/>Main Thread]
        RenderThread[Render Thread]
        PhysicsThread[Physics Thread]
    end
    
    subgraph ECS["ECS Core<br/>(Thread-Safe)"]
        Registry[Registry<br/>Central Coordinator]
        
        subgraph StorageLayer["Storage Layer"]
            ComponentPools[Component Pools<br/>SparseSet Containers]
        end
        
        subgraph ViewLayer["View Layer"]
            Views[Views<br/>Query System]
            ParallelViews[Parallel Views<br/>Multi-threaded Iteration]
        end
        
        subgraph SystemLayer["System Layer"]
            SystemScheduler[System Scheduler<br/>Dependency Graph]
        end
        
        subgraph EventLayer["Event Layer"]
            SignalDispatcher[Signal Dispatcher<br/>Observer Pattern]
        end
    end
    
    subgraph Memory["Memory Management"]
        Heap[Heap Memory<br/>Component Data]
        Stack[Stack Memory<br/>View Objects]
    end
    
    subgraph Sync["Synchronization Primitives"]
        EntityMutex[entity_mutex<br/>shared_mutex]
        ComponentMutex[component_pool_mutex<br/>shared_mutex]
        SparseSetMutexes[Per-Pool Mutexes<br/>mutex]
    end
    
    GameLoop --> Registry
    RenderThread --> ParallelViews
    PhysicsThread --> ParallelViews
    
    Registry --> ComponentPools
    Registry --> SignalDispatcher
    Registry --> SystemScheduler
    
    Views --> ComponentPools
    ParallelViews --> ComponentPools
    SystemScheduler --> Views
    
    ComponentPools --> Heap
    Views --> Stack
    
    Registry -.-> EntityMutex
    Registry -.-> ComponentMutex
    ComponentPools -.-> SparseSetMutexes
    
    style Registry fill:#4a90e2
    style ComponentPools fill:#50c878
    style ParallelViews fill:#ff6b6b
    style SignalDispatcher fill:#ffd93d
```

---

## Architecture Summary

### Key Design Patterns

1. **Entity-Component-System (ECS)**: Core architectural pattern
2. **Sparse Set**: Efficient component storage with O(1) operations
3. **Observer Pattern**: Signal system for component lifecycle events
4. **Command Pattern**: Deferred operations via CommandBuffer
5. **Prototype Pattern**: Prefab system for entity templates
6. **Strategy Pattern**: System scheduler with pluggable systems
7. **Iterator Pattern**: Views for component traversal
8. **Dependency Injection**: Registry passed to systems
9. **Object Pool**: Entity recycling with generations
10. **Flyweight Pattern**: Tag components with zero memory overhead

### Thread Safety Model

```
┌─────────────────────────────────────────────────────────┐
│                    Thread-Safe Operations                │
├─────────────────────────────────────────────────────────┤
│ ✓ Concurrent reads (multiple threads)                   │
│ ✓ Parallel view iteration (different components)        │
│ ✓ Signal registration (with locking)                    │
│ ✓ Entity creation/destruction (with locking)            │
├─────────────────────────────────────────────────────────┤
│                  Unsafe Operations                       │
├─────────────────────────────────────────────────────────┤
│ ✗ Structural changes during parallel iteration          │
│ ✗ Modifying same component from multiple threads        │
│ ✗ Adding/removing components during view iteration      │
└─────────────────────────────────────────────────────────┘
```

### Memory Layout

```
Registry Memory Hierarchy:
├── Stack (lightweight)
│   ├── Entity (8 bytes)
│   ├── View objects (16-32 bytes)
│   └── Iterators (8-16 bytes)
│
└── Heap (component data)
    ├── SparseSet::dense (contiguous components)
    ├── SparseSet::packed (entity IDs)
    ├── SparseSet::sparse (index lookup)
    └── Relationship graphs
```

---

**Document Version**: 1.0  
**Last Updated**: November 21, 2025  
**Author**: ECS Architecture Team
