---
sidebar_label: Add a new system
sidebar_position: 2
---

# How to add a new ECS System

This tutorial shows the steps to add a new System (server or client), register it with the scheduler and add tests.

## 1) Identify the target: server or client

- Server systems live in: `src/games/rtype/server/Systems/`
- Client systems live in: `src/games/rtype/client/Systems/`

## 2) Create the system files

1. Add a header and a source file, e.g. for a server `HealthRegenSystem`:

- `src/games/rtype/server/Systems/HealthRegenSystem.hpp`
- `src/games/rtype/server/Systems/HealthRegenSystem.cpp`

1. Derive from `rtype::engine::ASystem` and implement `update`:
2. Inherit from `rtype::engine::ASystem` and implement `update`:

```cpp
class HealthRegenSystem : public ::rtype::engine::ASystem {
public:
    HealthRegenSystem();
    void update(ECS::Registry& registry, float dt) override;
};
```

## 3) Implement `update` with registry `view`

Use `registry.view<ComponentA, ComponentB>()` to iterate over entities with the needed components; don't assume other components exist — use `hasComponent` before accessing components.

Example:

```cpp
void HealthRegenSystem::update(ECS::Registry& registry, float dt) {
    registry.view<HealthComponent, HealthRegenTag>().each(
        [dt](auto entity, auto& health, auto& regen) {
            // apply regen logic
        });
}
```

- ## 4) Register the system with the scheduler (server) or create it in the client scene

- Server: Add your system instance to `src/games/rtype/server/GameEngine.cpp::initialize()`, and ensure it’s scheduled in the right order relative to other systems.
- Client: Create and register the system in the client scene manager or the `Graphic` setup (see `src/client/Graphic/SceneManager/*` and `src/games/rtype/client/Systems/*`).

## 5) Add tests

- Add unit tests in `tests/games/rtype/` (e.g., `test_health_regen.cpp`).
- Keep tests fast, deterministic and focused. Ensure that edge cases like component absence are covered.

## 6) Add docs and examples

- Add a short README or an entry in `docs/website/docs/Architecture/` to document the system, behavior, configuration, and dependencies.

---

- ### Tips

- Keep systems decoupled: don't access rendering logic from server systems and do not access game logic in render systems.
- Use `Event`/signals for cross-system notifications.
- Add a GTest for your system and run the full test suite.

---

Happy hacking!
