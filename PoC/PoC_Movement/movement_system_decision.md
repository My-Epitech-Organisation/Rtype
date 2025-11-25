# Movement System Architecture - Final Decision

## Decision Summary

**Date:** November 24, 2025  
**Status:** ✅ Approved  
**Decision Makers:** R-Type Development Team

After thorough evaluation of four movement system prototypes, we have decided to implement a **hybrid multi-system architecture** that combines the strengths of different approaches.

---

## Selected Systems

### ✅ Core Systems (Must Implement)

#### 1. Linear Movement System
**Priority:** Critical  
**Implementation Timeline:** Sprint 1

**Rationale:**
- Foundation for all projectile-based gameplay
- Highest performance characteristics
- Simple, predictable, and reliable
- Essential for bullets, particles, and simple enemies

**Usage:**
- All bullet types
- Projectiles
- Particle effects
- Simple enemy charges
- Background elements

---

#### 2. Sine Wave Movement System
**Priority:** High  
**Implementation Timeline:** Sprint 1

**Rationale:**
- Creates the classic R-Type aesthetic
- Excellent performance-to-visual-appeal ratio
- Simple to tune and adjust
- Perfect for arcade-style enemy patterns

**Usage:**
- Wave formation enemies
- Floating power-ups
- Background animations
- Enemy patrol patterns
- Classic shooter feel

---

#### 3. Scripted Movement System
**Priority:** High  
**Implementation Timeline:** Sprint 2

**Rationale:**
- Empowers designers to create content
- Enables rapid prototyping and iteration
- Data-driven approach for maintainability
- Flexible for complex patterns

**Usage:**
- Boss movement patterns
- Multi-phase enemy behaviors
- Cutscene movements
- Tutorial sequences
- Complex attack choreography

---

### 📋 Optional System (Evaluate Later)

#### 4. Bézier Curve Movement System
**Priority:** Medium  
**Implementation Timeline:** Sprint 3-4 (Polish Phase)

**Rationale:**
- High computational cost
- Complex to author without visual tools
- Best reserved for cinematic moments
- Can be added after core gameplay is solid

**Usage:**
- Boss entrance/exit cinematics
- Special dramatic enemy entrances
- Key story moments
- Visual polish for important sequences

**Decision:** Implement only if:
- Performance budget allows
- Visual editor tool is available
- Art direction requires cinematic curves
- Core gameplay is complete

---

## Implementation Architecture

### Component Design

```cpp
namespace RType::Movement {
    // Shared base
    struct Position {
        float x, y;
    };

    // Linear movement
    struct Velocity {
        float dx, dy;  // Direction * Speed combined
    };

    // Sine wave movement
    struct SineWave {
        float centerY;
        float frequency;
        float amplitude;
        float phase;
        float time;
    };

    // Scripted movement
    struct MovementScript {
        std::vector<std::unique_ptr<ICommand>> commands;
        size_t currentIndex;
    };
}
```

### System Execution Order

```cpp
void GameLoop::updateMovement(float deltaTime) {
    // 1. Base movement (Linear)
    linearMovementSystem.update(registry, deltaTime);
    
    // 2. Modifiers (Sine Wave)
    sineWaveMovementSystem.update(registry, deltaTime);
    
    // 3. Scripted overrides (Scripted)
    scriptedMovementSystem.update(registry, deltaTime);
    
    // 4. Optional (Bézier) - Only if implemented
    // bezierMovementSystem.update(registry, deltaTime);
}
```

### Component Composition Strategy

Entities can combine multiple movement components:

```cpp
// Simple bullet - Linear only
auto bullet = registry.spawnEntity();
registry.emplaceComponent<Position>(bullet, x, y);
registry.emplaceComponent<Velocity>(bullet, dx, dy);

// Wave enemy - Linear + Sine Wave
auto waveEnemy = registry.spawnEntity();
registry.emplaceComponent<Position>(waveEnemy, x, y);
registry.emplaceComponent<Velocity>(waveEnemy, 1.0f, 0.0f);  // Move right
registry.emplaceComponent<SineWave>(waveEnemy, centerY, freq, amp);

// Boss - Scripted (can include Linear/Sine in script)
auto boss = registry.spawnEntity();
registry.emplaceComponent<Position>(boss, x, y);
registry.emplaceComponent<MovementScript>(boss, std::move(script));
```

---

## Performance Targets

### Target Entity Counts @ 60 FPS

| Entity Type | System Used | Target Count | Expected Performance |
|-------------|-------------|--------------|---------------------|
| Bullets | Linear | 2,000 | Excellent (100k+ capacity) |
| Particles | Linear | 5,000 | Excellent |
| Basic Enemies | Linear + Sine | 100 | Excellent |
| Elite Enemies | Scripted | 20 | Good |
| Bosses | Scripted | 1-3 | Excellent |
| **Total** | **Mixed** | **~7,000** | **Excellent** |

### Performance Budget

- **Linear:** <0.1ms per frame (2,000 entities)
- **Sine Wave:** <0.2ms per frame (100 entities)
- **Scripted:** <0.5ms per frame (20 entities)
- **Total Movement:** <1.0ms per frame (target <2ms)

---

## Development Phases

### Phase 1: Foundation (Sprint 1)
**Timeline:** Week 1-2

- ✅ Implement Linear Movement System
- ✅ Implement Sine Wave Movement System
- ✅ Create basic enemy types
- ✅ Create bullet patterns
- ✅ Performance testing and optimization

**Deliverables:**
- Working bullet system
- 3-5 basic enemy types with different patterns
- Performance benchmark suite

---

### Phase 2: Content Tools (Sprint 2)
**Timeline:** Week 3-4

- ✅ Implement Scripted Movement System
- ✅ Create script parser and command library
- ✅ Design script file format
- ✅ Build script validation tools
- ✅ Create example scripts for designers

**Deliverables:**
- Working script system
- Script documentation for designers
- 10+ example movement scripts
- Error handling and validation

---

### Phase 3: Polish (Sprint 3-4)
**Timeline:** Week 5-6 (Conditional)

- ⏸️ **Evaluate** Bézier implementation need
- ⏸️ Build visual curve editor (if implementing Bézier)
- ✅ Optimize existing systems
- ✅ Add advanced scripting commands
- ✅ Designer training and documentation

**Deliverables:**
- Polished movement systems
- Complete designer documentation
- Performance optimizations
- (Optional) Bézier system if approved

---

## Technical Specifications

### Memory Budget
- **Per Entity Average:** 32 bytes
- **Total for 7,000 entities:** ~224 KB
- **System Overhead:** ~50 KB
- **Total:** ~300 KB (well within budget)

### Threading Strategy
- Movement systems are **read-write** operations
- Use **parallel processing** for Linear system (highest count)
- Keep Sine Wave and Scripted single-threaded (lower counts)
- No thread contention with proper ECS view iteration

### Serialization Requirements
All movement components must be serializable for:
- Save/Load game state
- Network synchronization
- Replay recording
- Debug visualization

---

## Risks and Mitigations

### Risk 1: Scripted System Performance
**Risk Level:** Medium  
**Impact:** Scripted entities may cause performance issues at scale

**Mitigation:**
- Limit scripted entities to 20-30 maximum
- Use object pooling for command objects
- Cache parsed scripts, don't parse every frame
- Profile regularly and optimize hotspots

---

### Risk 2: Designer Learning Curve
**Risk Level:** Medium  
**Impact:** Designers may struggle with script syntax

**Mitigation:**
- Provide comprehensive documentation
- Create visual script editor (future enhancement)
- Include 20+ example scripts
- Regular designer training sessions
- Built-in script validation with clear error messages

---

### Risk 3: Bézier Decision Delay
**Risk Level:** Low  
**Impact:** May need Bézier later, requires retrofitting

**Mitigation:**
- Design system architecture to support adding Bézier
- Keep component interface flexible
- Document Bézier requirements for future implementation
- Prototype is already complete, can be integrated quickly

---

## Success Metrics

### Performance Metrics
- ✅ Maintain 60 FPS with 7,000+ moving entities
- ✅ Movement systems use <2ms per frame total
- ✅ Memory usage <500 KB for movement systems

### Gameplay Metrics
- ✅ 10+ distinct enemy movement patterns
- ✅ Designer can create new pattern in <30 minutes
- ✅ Players find movement patterns interesting and fair

### Technical Metrics
- ✅ 100% test coverage for core movement systems
- ✅ Zero movement-related crashes
- ✅ Network sync <50ms latency for movement

---

## Future Enhancements

### Phase 4+ (Post-Launch)
If the game is successful, consider:

1. **Visual Script Editor**
   - Drag-and-drop movement command builder
   - Real-time preview of movement patterns
   - Bézier curve visual editor with control points

2. **Advanced Movement Types**
   - Flocking/swarming behavior
   - Orbital movement around points
   - Elastic/spring physics
   - Gravity-affected projectiles

3. **AI-Assisted Design**
   - Generate movement patterns from descriptions
   - Auto-balance movement difficulty
   - Suggest variations on existing patterns

4. **Performance Optimizations**
   - SIMD vectorization for Linear system
   - GPU compute for large particle counts
   - Spatial partitioning for collision-based movement

---

## Conclusion

The hybrid approach combining **Linear**, **Sine Wave**, and **Scripted** movement systems provides:

✅ **Performance:** Handles thousands of entities efficiently  
✅ **Flexibility:** Supports simple to complex patterns  
✅ **Designer Empowerment:** Data-driven content creation  
✅ **Visual Appeal:** Classic arcade aesthetics  
✅ **Maintainability:** Clean component-based architecture  
✅ **Scalability:** Easy to extend with new systems  

This architecture balances technical requirements with gameplay needs, enabling the team to create the classic R-Type experience while maintaining modern development practices.

---

## Approval Signatures

**Technical Lead:** _____________________ Date: ___________  
**Gameplay Designer:** __________________ Date: ___________  
**Performance Engineer:** _______________ Date: ___________  

---

## References

- [Movement Analysis Report](../../PoC/Movement/MOVEMENT_ANALYSIS.md)
- [ECS Architecture Documentation](../ecs/README.md)
- [Linear Movement PoC](../../PoC/Movement/LinearMovement/)
- [Sine Wave Movement PoC](../../PoC/Movement/SineWaveMovement/)
- [Bézier Movement PoC](../../PoC/Movement/BezierMovement/)
- [Scripted Movement PoC](../../PoC/Movement/ScriptedMovement/)

---

**Document Version:** 1.0  
**Last Updated:** November 24, 2025  
**Next Review:** End of Sprint 2
