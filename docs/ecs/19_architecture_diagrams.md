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
        +bool isNull() const
        +bool isTombstone() const
        +operator<=>(const Entity&) const
    }
    
    %% ============================================================================
    %% REGISTRY - CENTRAL COORDINATOR
    %% ============================================================================
    
    class Registry {
        -unordered_map~uint32_t, vector~type_index~~ _entityComponents
        -vector~uint32_t~ generations
        -vector~uint32_t~ _freeIndices
        -vector~uint32_t~ tombstones
        -unordered_map~type_index, unique_ptr~I_sparseSet~~ _componentPools
        -unordered_map~type_index, any~ singletons
        -SignalDispatcher _signalDispatcher
        -RelationshipManager _relationshipManager
        -shared_mutex _entityMutex
        -shared_mutex _componentPoolMutex
        
        +Registry()
        
        %% Entity Management
        +void reserveEntities(size_t capacity)
        +Entity spawnEntity()
        +void killEntity(Entity entity) noexcept
        +bool isAlive(Entity entity) const noexcept
        +size_t cleanupTombstones()
        +size_t removeEntitiesIf(Func predicate)
        
        %% Component Management
        +void reserveComponents~T~(size_t capacity)
        +void compact()
        +void compactComponent~T~()
        +T& emplaceComponent~T~(Entity entity, Args... args)
        +T& getOrEmplace~T~(Entity entity, Args... args)
        +void removeComponent~T~(Entity entity)
        +void clearComponents~T~()
        +bool hasComponent~T~(Entity entity) const
        +size_t countComponents~T~() const
        +T& getComponent~T~(Entity entity)
        +void patch~T~(Entity entity, Func func)
        
        %% Signals
        +void onConstruct~T~(function~void(Entity)~ callback)
        +void onDestroy~T~(function~void(Entity)~ callback)
        
        %% Views
        +View~Components...~ view()
        +ParallelView~Components...~ parallelView()
        +Group~Components...~ createGroup()
        
        %% Singletons
        +T& setSingleton~T~(Args... args)
        +T& getSingleton~T~()
        +bool hasSingleton~T~() const
        +void removeSingleton~T~()
        
        %% Relationships
        +RelationshipManager& getRelationshipManager()
        
        -auto& getSparseSet~T~()
        -const ISparseSet* getSparseSetConst~T~() const
        -const auto& getSparseSetTypedConst~T~() const
    }
    
    %% ============================================================================
    %% STORAGE - COMPONENT CONTAINERS
    %% ============================================================================
    
    class I_sparseSet {
        <<interface>>
        +remove(Entity entity)*
        +contains(Entity entity) const*
        +clear()*
        +size() const*
        +shrinkToFit()*
    }
    
    class _sparseSet~T~ {
        -vector~T~ dense
        -vector~Entity~ _packed
        -vector~size_t~ _sparse
        -mutex __sparseSetMutex
        
        +bool contains(Entity entity) const
        +T& emplace(Entity entity, Args... args)
        +void remove(Entity entity)
        +T& get(Entity entity)
        +const T& get(Entity entity) const
        +void clear()
        +size_t size() const
        +void reserve(size_t capacity)
        +void shrinkToFit()
        +const vector~Entity~& getPacked() const
        +iterator begin()
        +iterator end()
    }
    
    class TagSparseSet~T~ {
        -vector~Entity~ _packed
        -vector~size_t~ _sparse
        -mutex __sparseSetMutex
        -static T _dummyInstance
        
        +bool contains(Entity entity) const
        +T& emplace(Entity entity, Args... args)
        +void remove(Entity entity)
        +T& get(Entity entity)
        +void clear()
        +size_t size() const
        +const vector~Entity~& getPacked() const
    }
    
    %% ============================================================================
    %% VIEWS - QUERY SYSTEM
    %% ============================================================================
    
    class View~Components...~ {
        -Registry& registry
        -tuple~_sparseSet~Components~*...~ pools
        -size_t _smallestPoolIndex
        
        +View(Registry& registry)
        +void each(Func func)
        +ExcludeView exclude~Excluded...~()
        
        -void initializePools(index_sequence)
        -void eachImpl(Func func, index_sequence)
        -size_t findSmallestPool(index_sequence)
    }
    
    class ParallelView~Components...~ {
        -Registry& registry
        
        +ParallelView(Registry& registry)
        +void each(Func func)
    }
    
    class ExcludeView~Includes..., Excludes...~ {
        -Registry& registry
        -tuple~_sparseSet~Includes~*...~ _includePools
        -vector~ISparseSet*~ _excludePools
        -size_t _smallestPoolIndex
        
        +void each(Func func)
        
        -void eachImpl(Func func, index_sequence)
        -bool is_excluded(Entity entity) const
    }
    
    class Group~Components...~ {
        -Registry& registry
        -vector~Entity~ entities
        
        +Group(Registry& reg)
        +void rebuild()
        +void each(Func func)
        +const vector~Entity~& getEntities() const
    }
    
    %% ============================================================================
    %% SIGNALS - EVENT SYSTEM
    %% ============================================================================
    
    class SignalDispatcher {
        -unordered_map~type_index, vector~Callback~~ _constructCallbacks
        -unordered_map~type_index, vector~Callback~~ _destroyCallbacks
        -shared_mutex callbacks_mutex
        
        +void registerConstruct(type_index type, Callback callback)
        +void registerDestroy(type_index type, Callback callback)
        +void dispatchConstruct(type_index type, Entity entity)
        +void dispatchDestroy(type_index type, Entity entity)
        +void clearCallbacks(type_index type)
        +void clearAllCallbacks()
    }
    
    %% ============================================================================
    %% RELATIONSHIPS - HIERARCHY SYSTEM
    %% ============================================================================
    
    class RelationshipManager {
        -unordered_map~uint32_t, Entity~ _parentMap
        -unordered_map~uint32_t, unordered_set~Entity~~ _childrenMap
        -shared_mutex _relationshipMutex
        
        +bool setParent(Entity child, Entity parent)
        +void removeParent(Entity child)
        +optional~Entity~ getParent(Entity child) const
        +bool hasParent(Entity child) const
        +vector~Entity~ getChildren(Entity parent) const
        +vector~Entity~ getDescendants(Entity parent) const
        +vector~Entity~ getAncestors(Entity child) const
        +Entity getRoot(Entity entity) const
        +bool isAncestor(Entity potential_ancestor, Entity entity) const
        +void removeEntity(Entity entity)
        +void clear()
        +size_t childCount(Entity parent) const
        +size_t getDepth(Entity entity) const
        
        -bool wouldCreateCycle(Entity child, Entity parent) const
        -void getDescendantsRecursive(Entity parent, vector~Entity~& result) const
    }
    
    %% ============================================================================
    %% COMMAND BUFFER - DEFERRED OPERATIONS
    %% ============================================================================
    
    class CommandBuffer {
        -Registry& registry
        -vector~function~void()~~ commands
        -unordered_map~uint32_t, Entity~ _placeholdertoReal
        -uint32_t _nextPlaceholderId
        -mutex _commandsMutex
        
        +CommandBuffer(Registry& reg)
        +Entity spawnEntityDeferred()
        +void destroyEntityDeferred(Entity entity)
        +void emplaceComponentDeferred~T~(Entity entity, Args... args)
        +void removeComponentDeferred~T~(Entity entity)
        +void flush()
        +size_t pendingCount() const
        +void clear()
    }
    
    %% ============================================================================
    %% PREFAB SYSTEM
    %% ============================================================================
    
    class PrefabManager {
        -Registry& registry
        -unordered_map~string, PrefabFunc~ prefabs
        -shared_mutex _prefabMutex
        
        +PrefabManager(Registry& reg)
        +void registerPrefab(string name, PrefabFunc func)
        +Entity instantiate(string name)
        +Entity instantiate(string name, PrefabFunc customizer)
        +vector~Entity~ instantiateMultiple(string name, size_t count)
        +bool hasPrefab(string name) const
        +void unregisterPrefab(string name)
        +vector~string~ getPrefabNames() const
        +void clear()
        +void createFromEntity(string name, Entity template_entity)
    }
    
    %% ============================================================================
    %% SYSTEM SCHEDULER
    %% ============================================================================
    
    class SystemScheduler {
        -Registry& registry
        -unordered_map~string, SystemNode~ systems
        -vector~string~ _executionOrder
        -bool _needsReorder
        
        +SystemScheduler(Registry& reg)
        +void addSystem(string name, SystemFunc func, vector~string~ dependencies)
        +void removeSystem(string name)
        +void run()
        +void runSystem(string name)
        +void clear()
        +vector~string~ getExecutionOrder() const
        +void setSystemEnabled(string name, bool enabled)
        +bool isSystemEnabled(string name) const
        
        -void recomputeOrder()
        -void topologicalSort()
        -bool hasCycle() const
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
        -unordered_map~type_index, unique_ptr~IComponentSerializer~~ _serializers
        
        +Serializer(Registry& reg)
        +void registerSerializer~T~(unique_ptr~IComponentSerializer~ serializer)
        +bool saveToFile(string filename)
        +bool loadFromFile(string filename, bool clear_existing)
        +string serialize()
        +bool deserialize(string data, bool clear_existing)
    }
    
    %% ============================================================================
    %% RELATIONSHIPS
    %% ============================================================================
    
    Registry *-- Entity : manages
    Registry *-- I_sparseSet : owns
    Registry *-- SignalDispatcher : contains
    Registry *-- RelationshipManager : contains
    
    I_sparseSet <|-- _sparseSet : implements
    I_sparseSet <|-- TagSparseSet : implements
    
    _sparseSet --> Entity : stores
    TagSparseSet --> Entity : stores
    
    View --> Registry : observes
    View --> _sparseSet : observes pools
    ParallelView --> Registry : observes
    ExcludeView --> Registry : observes
    ExcludeView --> I_sparseSet : observes
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
        I_sparseSet[I_sparseSet<br/>Storage interface]
        _sparseSet[_sparseSet<br/>Component storage]
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
    
    Client->>+Registry: spawnEntity()
    Registry->>Registry: lock(_entityMutex)
    
    alt Free indices available
        Registry->>EntitySystem: Get from _freeIndices
        Registry->>EntitySystem: Validate generation < MaxGeneration
        Registry->>EntitySystem: Create Entity(index, generation)
    else No free indices
        Registry->>EntitySystem: Allocate new index
        Registry->>EntitySystem: Create Entity(index, 0)
    end
    
    Registry->>Registry: _entityComponents[index] = []
    Registry->>Registry: unlock(_entityMutex)
    Registry-->>-Client: Entity
    
    %% Component Addition
    Note over Client,RelationshipManager: Component Addition Flow
    
    Client->>+Registry: emplaceComponent<Position>(entity, x, y)
    Registry->>Registry: isAlive(entity)?
    
    alt Entity is dead
        Registry-->>Client: throw runtime_error
    end
    
    Registry->>Registry: lock(_entityMutex)
    Registry->>Registry: _entityComponents[index].push_back(type)
    Registry->>Registry: unlock(_entityMutex)
    
    Registry->>+ComponentPools: getSparseSet<Position>()
    ComponentPools->>ComponentPools: emplace(entity, x, y)
    ComponentPools-->>-Registry: Position&
    
    Registry->>+SignalDispatcher: dispatchConstruct(type, entity)
    SignalDispatcher->>SignalDispatcher: Get callbacks for type
    
    loop For each callback
        SignalDispatcher->>Client: callback(entity)
    end
    
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: Position&
    
    %% Entity Destruction
    Note over Client,RelationshipManager: Entity Destruction Flow
    
    Client->>+Registry: killEntity(entity)
    Registry->>Registry: lock(_entityMutex)
    Registry->>Registry: Validate entity
    Registry->>Registry: Collect components to remove
    Registry->>Registry: Increment generation
    Registry->>Registry: Add to _freeIndices
    Registry->>Registry: _entityComponents.erase(index)
    Registry->>Registry: unlock(_entityMutex)
    
    loop For each component
        Registry->>+SignalDispatcher: dispatchDestroy(type, entity)
        SignalDispatcher->>Client: callback(entity)
        SignalDispatcher-->>-Registry: void
        
        Registry->>+ComponentPools: remove(entity)
        ComponentPools-->>-Registry: void
    end
    
    Registry->>+RelationshipManager: removeEntity(entity)
    RelationshipManager->>RelationshipManager: Remove parent/child links
    RelationshipManager-->>-Registry: void
    
    Registry-->>-Client: void
```

### Component Management

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant _sparseSet as _sparseSet<T>
    participant SignalDispatcher
    
    %% Get or Emplace Pattern
    Note over Client,SignalDispatcher: Get or Emplace Pattern
    
    Client->>+Registry: getOrEmplace<Velocity>(entity, dx, dy)
    Registry->>+_sparseSet: contains(entity)?
    _sparseSet-->>-Registry: bool
    
    alt Component exists
        Registry->>+_sparseSet: get(entity)
        _sparseSet-->>-Registry: Velocity&
        Registry-->>Client: Velocity& (existing)
    else Component missing
        Registry->>Registry: lock(_entityMutex)
        Registry->>Registry: _entityComponents[index].push_back(type)
        Registry->>Registry: unlock(_entityMutex)
        
        Registry->>+_sparseSet: emplace(entity, dx, dy)
        _sparseSet->>_sparseSet: lock(__sparseSetMutex)
        
        alt Entity index >= _sparse.size()
            _sparseSet->>_sparseSet: _sparse.resize(index + 1)
        end
        
        _sparseSet->>_sparseSet: _sparse[index] = dense.size()
        _sparseSet->>_sparseSet: _packed.push_back(entity)
        _sparseSet->>_sparseSet: dense.emplace_back(dx, dy)
        _sparseSet->>_sparseSet: unlock(__sparseSetMutex)
        _sparseSet-->>-Registry: Velocity&
        
        Registry->>+SignalDispatcher: dispatchConstruct(type, entity)
        SignalDispatcher-->>-Registry: void
        
        Registry-->>Client: Velocity& (new)
    end
    
    Registry-->>-Client: Velocity&
    
    %% Component Removal
    Note over Client,SignalDispatcher: Component Removal
    
    Client->>+Registry: removeComponent<Velocity>(entity)
    
    Registry->>+SignalDispatcher: dispatchDestroy(type, entity)
    SignalDispatcher->>SignalDispatcher: Execute callbacks
    SignalDispatcher-->>-Registry: void
    
    Registry->>+_sparseSet: remove(entity)
    _sparseSet->>_sparseSet: lock(__sparseSetMutex)
    _sparseSet->>_sparseSet: dense_index = _sparse[entity.index()]
    _sparseSet->>_sparseSet: Swap with last element
    _sparseSet->>_sparseSet: dense.pop_back()
    _sparseSet->>_sparseSet: _packed.pop_back()
    _sparseSet->>_sparseSet: Update swapped element's _sparse index
    _sparseSet->>_sparseSet: _sparse[entity.index()] = NullIndex
    _sparseSet->>_sparseSet: unlock(__sparseSetMutex)
    _sparseSet-->>-Registry: void
    
    Registry->>Registry: lock(_entityMutex)
    Registry->>Registry: _entityComponents[index].erase(type)
    Registry->>Registry: unlock(_entityMutex)
    
    Registry-->>-Client: void
```

### View Iteration

```mermaid
sequenceDiagram
    participant Client
    participant Registry
    participant View
    participant _sparseSet1 as _sparseSet<Position>
    participant _sparseSet2 as _sparseSet<Velocity>
    
    %% View Creation
    Note over Client,_sparseSet2: View Creation and Iteration
    
    Client->>+Registry: view<Position, Velocity>()
    Registry->>+View: View(registry)
    
    View->>+Registry: getSparseSet<Position>()
    Registry-->>-View: _sparseSet<Position>*
    
    View->>+Registry: getSparseSet<Velocity>()
    Registry-->>-View: _sparseSet<Velocity>*
    
    View->>View: pools = {pos_pool*, vel_pool*}
    View->>View: Find smallest pool
    
    View->>+_sparseSet1: getPacked().size()
    _sparseSet1-->>-View: 100
    
    View->>+_sparseSet2: getPacked().size()
    _sparseSet2-->>-View: 80
    
    View->>View: _smallestPoolIndex = 1 (Velocity)
    
    View-->>-Registry: View<Position, Velocity>
    Registry-->>-Client: View<Position, Velocity>
    
    %% View Iteration
    Note over Client,_sparseSet2: Iteration over Entities
    
    Client->>+View: each([](Entity e, Position& p, Velocity& v) {...})
    
    View->>+_sparseSet2: getPacked()
    _sparseSet2-->>-View: vector<Entity>& (80 entities)
    
    loop For each entity in smallest pool
        View->>View: entity = _packed[i]
        
        View->>+_sparseSet1: contains(entity)?
        _sparseSet1-->>-View: true
        
        View->>+_sparseSet2: contains(entity)?
        _sparseSet2-->>-View: true
        
        alt All pools contain entity
            View->>+_sparseSet1: get(entity)
            _sparseSet1-->>-View: Position&
            
            View->>+_sparseSet2: get(entity)
            _sparseSet2-->>-View: Velocity&
            
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
    participant _sparseSet as _sparseSet<Position>
    
    Note over Client,_sparseSet: Parallel View Setup
    
    Client->>+Registry: parallelView<Position>()
    Registry->>+ParallelView: ParallelView(registry)
    ParallelView-->>-Registry: ParallelView
    Registry-->>-Client: ParallelView<Position>
    
    Note over Client,_sparseSet: Parallel Execution
    
    Client->>+ParallelView: each([](Entity e, Position& p) {...})
    
    ParallelView->>+Registry: getSparseSet<Position>()
    Registry-->>-ParallelView: _sparseSet<Position>*
    
    ParallelView->>+_sparseSet: getPacked()
    _sparseSet-->>-ParallelView: vector<Entity>& (10000 entities)
    
    ParallelView->>ParallelView: num_threads = hardware_concurrency()
    ParallelView->>ParallelView: chunk_size = 10000 / num_threads
    
    Note over Thread1,ThreadN: Spawn Worker Threads
    
    par Thread 1 (entities 0-3333)
        ParallelView->>+Thread1: spawn([&, start=0, end=3333]() {...})
        loop i = 0 to 3333
            Thread1->>+_sparseSet: contains(entities[i])?
            _sparseSet-->>-Thread1: true
            Thread1->>+_sparseSet: get(entities[i])
            _sparseSet-->>-Thread1: Position&
            Thread1->>Thread1: callback(entity, pos)
        end
        Thread1-->>-ParallelView: void
    and Thread 2 (entities 3334-6666)
        ParallelView->>+Thread2: spawn([&, start=3334, end=6666]() {...})
        loop i = 3334 to 6666
            Thread2->>+_sparseSet: contains(entities[i])?
            _sparseSet-->>-Thread2: true
            Thread2->>+_sparseSet: get(entities[i])
            _sparseSet-->>-Thread2: Position&
            Thread2->>Thread2: callback(entity, pos)
        end
        Thread2-->>-ParallelView: void
    and Thread N (entities 6667-10000)
        ParallelView->>+ThreadN: spawn([&, start=6667, end=10000]() {...})
        loop i = 6667 to 10000
            ThreadN->>+_sparseSet: contains(entities[i])?
            _sparseSet-->>-ThreadN: true
            ThreadN->>+_sparseSet: get(entities[i])
            _sparseSet-->>-ThreadN: Position&
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
    
    Client->>+Registry: onConstruct<Sprite>(callback1)
    Registry->>+SignalDispatcher: registerConstruct(typeid(Sprite), callback1)
    SignalDispatcher->>SignalDispatcher: lock(callbacks_mutex)
    SignalDispatcher->>SignalDispatcher: _constructCallbacks[Sprite].push_back(callback1)
    SignalDispatcher->>SignalDispatcher: unlock(callbacks_mutex)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    Client->>+Registry: onConstruct<Sprite>(callback2)
    Registry->>+SignalDispatcher: registerConstruct(typeid(Sprite), callback2)
    SignalDispatcher->>SignalDispatcher: _constructCallbacks[Sprite].push_back(callback2)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    Client->>+Registry: onDestroy<Sprite>(cleanup_callback)
    Registry->>+SignalDispatcher: registerDestroy(typeid(Sprite), cleanup_callback)
    SignalDispatcher->>SignalDispatcher: _destroyCallbacks[Sprite].push_back(cleanup_callback)
    SignalDispatcher-->>-Registry: void
    Registry-->>-Client: void
    
    %% Component Addition Triggering Signals
    Note over Client,System3: Component Addition Event
    
    Client->>+Registry: emplaceComponent<Sprite>(entity, texture)
    
    Registry->>Registry: Add component to entity
    Registry->>Registry: Store in _sparseSet
    
    Registry->>+SignalDispatcher: dispatchConstruct(typeid(Sprite), entity)
    
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
    
    Client->>+Registry: removeComponent<Sprite>(entity)
    
    Registry->>+SignalDispatcher: dispatchDestroy(typeid(Sprite), entity)
    
    SignalDispatcher->>SignalDispatcher: Copy callbacks for Sprite
    
    SignalDispatcher->>+System3: cleanup_callback(entity)
    System3->>System3: Release GPU resources
    System3->>System3: Unload textures
    System3-->>-SignalDispatcher: void
    
    SignalDispatcher-->>-Registry: void
    
    Registry->>Registry: Remove from _sparseSet
    Registry->>Registry: Remove from _entityComponents
    
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
    
    Client->>+Registry: parallelView<Health>()
    Registry-->>-Client: ParallelView
    
    Client->>ParallelView: each([&cmd](Entity e, Health& h) {...})
    
    par Thread 1 Processing
        ParallelView->>+Thread1: Process entities 0-500
        loop For entities in chunk
            Thread1->>Thread1: if (health.value <= 0)
            Thread1->>+CommandBuffer: destroyEntityDeferred(entity)
            CommandBuffer->>CommandBuffer: lock(_commandsMutex)
            CommandBuffer->>CommandBuffer: commands.push_back(lambda)
            CommandBuffer->>CommandBuffer: unlock(_commandsMutex)
            CommandBuffer-->>-Thread1: void
        end
        Thread1-->>-ParallelView: Done
    and Thread 2 Processing
        ParallelView->>+Thread2: Process entities 501-1000
        loop For entities in chunk
            Thread2->>Thread2: if (health.value <= 0)
            Thread2->>+CommandBuffer: destroyEntityDeferred(entity)
            CommandBuffer->>CommandBuffer: lock(_commandsMutex)
            CommandBuffer->>CommandBuffer: commands.push_back(lambda)
            CommandBuffer->>CommandBuffer: unlock(_commandsMutex)
            CommandBuffer-->>-Thread2: void
        end
        Thread2-->>-ParallelView: Done
    end
    
    Note over Client,Registry: Flush Phase (After Parallel Iteration)
    
    Client->>+CommandBuffer: flush()
    
    CommandBuffer->>CommandBuffer: lock(_commandsMutex)
    CommandBuffer->>CommandBuffer: Copy commands vector
    CommandBuffer->>CommandBuffer: unlock(_commandsMutex)
    
    loop For each deferred command
        CommandBuffer->>+Registry: killEntity(entity)
        Registry->>Registry: Remove components
        Registry->>Registry: Update generations
        Registry->>Registry: Add to _freeIndices
        Registry-->>-CommandBuffer: void
    end
    
    CommandBuffer->>CommandBuffer: commands.clear()
    CommandBuffer->>CommandBuffer: _placeholdertoReal.clear()
    CommandBuffer->>CommandBuffer: _nextPlaceholderId = 0
    
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
    
    Client->>+SystemScheduler: addSystem("physics", physics_func, {})
    SystemScheduler->>SystemScheduler: systems["physics"] = {func, {}, enabled=true}
    SystemScheduler->>SystemScheduler: _needsReorder = true
    SystemScheduler-->>-Client: void
    
    Client->>+SystemScheduler: addSystem("collision", collision_func, {"physics"})
    SystemScheduler->>SystemScheduler: systems["collision"] = {func, {"physics"}, enabled=true}
    SystemScheduler->>SystemScheduler: _needsReorder = true
    SystemScheduler-->>-Client: void
    
    Client->>+SystemScheduler: addSystem("render", render_func, {"collision"})
    SystemScheduler->>SystemScheduler: systems["render"] = {func, {"collision"}, enabled=true}
    SystemScheduler->>SystemScheduler: _needsReorder = true
    SystemScheduler-->>-Client: void
    
    Note over Client,RenderSystem: System Execution Phase
    
    Client->>+SystemScheduler: run()
    
    alt _needsReorder == true
        SystemScheduler->>SystemScheduler: topologicalSort()
        SystemScheduler->>SystemScheduler: _executionOrder = ["physics", "collision", "render"]
        SystemScheduler->>SystemScheduler: _needsReorder = false
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
    [*] --> Free: spawnEntity()
    
    Free: Free Slot
    Alive: Alive Entity
    Dead: Dead Entity
    Tombstone: Tombstone (Max Generation)
    
    Free --> Alive: Allocate<br/>generation=0
    
    Alive --> Dead: killEntity()<br/>generation++
    
    Dead --> Alive: spawnEntity()<br/>Reuse index
    Dead --> Tombstone: generation >= MaxGeneration
    
    Tombstone --> Free: cleanupTombstones()<br/>Reset generation
    
    Alive --> Alive: add/remove components
    
    note right of Alive
        Entity can have components
        Can be queried by views
        Valid for operations
    end note
    
    note right of Dead
        Invalid entity ID
        Components removed
        Index in _freeIndices
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
    
    NonExistent --> Constructing: emplaceComponent()
    
    Constructing --> Active: onConstruct signals fired
    
    Active --> Active: patch()<br/>getComponent()<br/>Modifications
    
    Active --> Destroying: removeComponent()<br/>killEntity()
    
    Destroying --> NonExistent: onDestroy signals fired<br/>Memory freed
    
    NonExistent --> Active: getOrEmplace()<br/>(if doesn't exist)
    
    Active --> Active: getOrEmplace()<br/>(if exists)
    
    note right of Active
        Component is in _sparseSet
        Accessible via getComponent()
        Entity tracked in _entityComponents
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
            ComponentPools[Component Pools<br/>_sparseSet Containers]
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
        EntityMutex[_entityMutex<br/>shared_mutex]
        ComponentMutex[_componentPoolMutex<br/>shared_mutex]
        _sparseSetMutexes[Per-Pool Mutexes<br/>mutex]
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
    ComponentPools -.-> _sparseSetMutexes
    
    style Registry fill:#4a90e2
    style ComponentPools fill:#50c878
    style ParallelViews fill:#ff6b6b
    style SignalDispatcher fill:#ffd93d
```

---

## Architecture Summary

### Key Design Patterns

1. **Entity-Component-System (ECS)**: Core architectural pattern
2. **_sparse Set**: Efficient component storage with O(1) operations
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
    ├── _sparseSet::dense (contiguous components)
    ├── _sparseSet::_packed (entity IDs)
    ├── _sparseSet::_sparse (index lookup)
    └── Relationship graphs
```

---

**Document Version**: 1.0  
**Last Updated**: November 21, 2025  
**Author**: ECS Architecture Team
