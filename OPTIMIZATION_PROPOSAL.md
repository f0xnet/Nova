# NovaEngine - Performance Optimization Proposal

## Executive Summary

Cette document propose des optimisations concrètes pour améliorer les performances du moteur NovaEngine d'un facteur **10-50x** pour les queries ECS et le rendu UI.

**Estimation d'effort:** 80-120 heures
**ROI:** Performance 10-50x + maintainabilité améliorée

---

## 1. OPTIMISATION PRIORITAIRE: Archetype Cache pour ECS Queries

### Problem Statement

Actuellement, `EntityRegistry::getEntitiesWith()` utilise une approche **naïve O(n×m×k)**:

```cpp
// Current implementation
std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& componentTypes) {
    std::vector<Entity*> result;
    
    for (auto& pair : m_entities) {              // O(n) - ALL entities
        Entity* entity = pair.second.get();
        bool hasAll = true;
        
        for (const auto& typeID : componentTypes) { // O(m) - components to match
            if (!entity->hasComponent(typeID)) {    // O(k) - component lookup
                hasAll = false;
                break;
            }
        }
        
        if (hasAll) {
            result.push_back(entity);
        }
    }
    return result;
}

// Usage in systems (9 times per frame × 60fps = 540 queries/sec!)
auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});
```

**Performance Analysis:**
- **Avec 100 entités, 5 systèmes:** 100 × 5 × 5 = 2,500 checks/frame
- **Avec 1000 entités, 10 systèmes:** 1000 × 10 × 5 = 50,000 checks/frame (3ms+ CPU time)
- **Current bottleneck:** RenderSystem + LightingSystem (called every frame)

### Solution: Archetype-Based Caching

**Concept:** Cache les résultats des queries basé sur "signatures" de composants (archetypes).

```cpp
// Proposed implementation
struct Archetype {
    std::set<ComponentTypeID> signature;        // Ex: {Transform, Sprite}
    std::vector<Entity*> entities;              // Cache des entités matching
    bool dirty = false;                         // Flag invalidity
};

class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    std::map<std::set<ComponentTypeID>, Archetype> m_archetypes;  // NEW
    u64 m_nextID = 1;
    
public:
    // Optimized query - O(1) lookup + O(p) iteration (p = matching entities)
    std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& componentTypes) {
        auto sig = std::set(componentTypes.begin(), componentTypes.end());
        
        // Check cache
        auto it = m_archetypes.find(sig);
        if (it != m_archetypes.end() && !it->second.dirty) {
            return it->second.entities;  // O(1) hit!
        }
        
        // Cache miss - rebuild
        Archetype& arch = m_archetypes[sig];
        arch.entities.clear();
        arch.dirty = false;
        
        for (auto& pair : m_entities) {
            Entity* entity = pair.second.get();
            bool hasAll = true;
            
            for (const auto& typeID : sig) {
                if (!entity->hasComponent(typeID)) {
                    hasAll = false;
                    break;
                }
            }
            
            if (hasAll) {
                arch.entities.push_back(entity);
            }
        }
        
        return arch.entities;
    }
    
    // Invalidate cache when entity changes
    template<typename T>
    T* addComponent(u64 entityID, std::unique_ptr<T> component) {
        Entity* entity = getEntity(entityID);
        T* ptr = entity->addComponent(std::move(component));
        
        // Mark all relevant archetypes as dirty
        for (auto& [sig, arch] : m_archetypes) {
            if (sig.count(component->getTypeID()) > 0) {
                arch.dirty = true;  // This archetype might have changed
            }
        }
        
        return ptr;
    }
    
    void removeComponent(u64 entityID, const ComponentTypeID& typeID) {
        Entity* entity = getEntity(entityID);
        entity->removeComponent(typeID);
        
        // Mark all relevant archetypes as dirty
        for (auto& [sig, arch] : m_archetypes) {
            if (sig.count(typeID) > 0) {
                arch.dirty = true;
            }
        }
    }
};
```

### Performance Gains

**Before (current implementation):**
```
1000 entities, 10 systems, 5 components per query:
  Time per frame: 1000 × 10 × 5 = 50,000 checks = ~3ms CPU
  60 FPS: 180ms/frame target → 1.7% CPU for ECS
```

**After (archetype cache):**
```
Same scenario:
  First frame: 50,000 checks (rebuild cache)
  Subsequent frames: 100 checks (9 archetypes × avg 11 entities) = ~10μs CPU
  60 FPS: 180ms/frame target → 0.005% CPU for ECS
  
SPEEDUP: 50,000 / 10 = 5,000x for cached queries
```

### Implementation Effort

**Files to modify:**
- `sdk/include/NovaEngine/ECS/EntityRegistry.hpp` (+80 lines)
- Tests to write: `tests/ECS/test_archetype_cache.cpp` (+150 lines)
- Estimated time: **6-8 hours**

---

## 2. SECONDARY OPTIMIZATION: UI Draw Call Batching

### Problem Statement

UIManager appelle `GRAPHICS().draw*()` pour chaque composant:

```cpp
// Current: Each component = 1+ draw calls
void UIManager::render() {
    for (auto& component : m_components) {
        component->render();  // Button: 1 draw, Text: 1 draw, etc.
    }
    // Result: 100+ draw calls for complex UI = GPU stall
}
```

### Solution: Draw Call Batching

```cpp
class UIBatch {
    std::vector<UIVertex> vertices;
    std::vector<u32> indices;
    TextureHandle atlas;
    size_t drawCallCount = 0;
    
public:
    void addRectangle(const Rect& rect, const Color& color) {
        // Add 4 vertices + 6 indices instead of calling drawRect
        vertices.push_back({rect.left, rect.top, color});
        vertices.push_back({rect.left + rect.width, rect.top, color});
        // ... etc
    }
    
    void addText(const std::string& text, const Vec2f& pos, const Color& color) {
        // Glyph lookup + vertex addition (no immediate draw)
    }
    
    void flush() {
        if (vertices.empty()) return;
        GRAPHICS().drawBatch(vertices, indices, atlas);
        vertices.clear();
        indices.clear();
        drawCallCount++;
    }
};

class UIManager {
    UIBatch m_batch;
    
    void render() {
        m_batch.clear();
        
        // Add all UI elements to batch
        for (auto& component : m_components) {
            component->addToBatch(m_batch);  // Virtual method
        }
        
        m_batch.flush();  // Single or few draw calls
        // Result: 100 components → 1-3 GPU calls
    }
};
```

### Performance Gains

**Before:**
```
UI with 100 components:
  - 50 rectangles: 50 draw calls
  - 30 text elements: 60 draw calls
  - 20 images: 20 draw calls
  Total: 130 GPU draw calls/frame = 5-10ms GPU time
```

**After (batching):**
```
Same UI:
  - All rectangles batched: 1 call
  - All text batched (glyph atlas): 1 call
  - All images batched: 1 call
  Total: 3 GPU draw calls/frame = 0.5-1ms GPU time
  
SPEEDUP: 10-20x for UI rendering
```

### Implementation Effort

**Files to create/modify:**
- `sdk/include/NovaEngine/UI/UIBatch.hpp` (new)
- `sdk/include/NovaEngine/Backend/Interfaces/IGraphicsBackend.hpp` (+method)
- `client/src/Backend/SFML/SFMLGraphicsBackend.cpp` (+implementation)
- `client/src/UI/UIManager.cpp` (refactor render)
- Estimated time: **12-16 hours**

---

## 3. THIRD OPTIMIZATION: Component Query Caching in Systems

### Problem Statement

Chaque système recalcule les queries à chaque frame:

```cpp
class RenderSystem : public System {
    void update(float deltaTime, EntityRegistry& registry) override {
        // Called 60× per second
        auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});
        // Query result NEVER changes unless entities added/removed
    }
};
```

### Solution: System-Level Query Cache

```cpp
class System {
protected:
    struct QueryCache {
        std::vector<ComponentTypeID> signature;
        std::vector<Entity*> result;
        bool dirty = false;
    };
    
    std::vector<QueryCache> m_queryCache;
    
    std::vector<Entity*> cachedQuery(const std::vector<ComponentTypeID>& sig) {
        // Check if already cached
        for (auto& cache : m_queryCache) {
            if (cache.signature == sig) {
                if (!cache.dirty) {
                    return cache.result;  // Use cached
                }
                // Rebuild if dirty
                cache.result = registry.getEntitiesWith(sig);
                cache.dirty = false;
                return cache.result;
            }
        }
        
        // New query
        QueryCache cache{sig, registry.getEntitiesWith(sig), false};
        m_queryCache.push_back(cache);
        return cache.result;
    }
};

class RenderSystem : public System {
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = cachedQuery({"TransformComponent", "SpriteComponent"});
        // Second+ frame: returns cached result
    }
};
```

### Implementation Effort

**Time: 4-6 hours**

---

## 4. ADVANCED FEATURE: Dialogue State Machine

### Current Architecture

```cpp
class DialogueSystem {
    bool m_dialogueActive;
    std::string m_currentNPCName;
    std::vector<std::string> m_currentDialogue;
    size_t m_currentDialogueLine;
};
```

**Limitations:**
- Pas de branching (si/sinon)
- Pas de conditions
- Pas de queuing (dialogues sequentiels)
- Simple bool state

### Proposed Enhancement: Dialogue State Machine

```cpp
enum class DialogueState {
    Idle,           // Pas de dialogue
    Starting,       // Animation debut
    Playing,        // Dialogue en cours
    WaitingInput,   // Attends E key
    Ending,         // Animation fin
};

struct DialogueChoice {
    std::string text;       // "Oui", "Non"
    std::string nextScene;  // ID dialogue suivante
    std::function<bool()> condition;  // Condition (nullptr = always)
};

struct DialogueNode {
    std::string id;
    std::string text;
    std::string npcName;
    std::vector<DialogueChoice> choices;
    bool autoAdvance = false;
    float autoAdvanceDelay = 3.0f;
};

class DialogueTree {
    std::map<std::string, DialogueNode> nodes;
    std::string m_currentNodeID;
    float m_autoAdvanceTimer = 0.0f;
    
public:
    bool loadFromJSON(const std::string& path);
    DialogueNode* getCurrentNode();
    bool advance(size_t choiceIndex);
    void playDialogue(const std::string& startNodeID);
};

class DialogueSystem {
    DialogueState m_state = DialogueState::Idle;
    DialogueTree m_tree;
    float m_stateTimer = 0.0f;
    
    void update(float dt) {
        switch (m_state) {
            case DialogueState::Starting:
                m_stateTimer += dt;
                if (m_stateTimer > 0.2f) {
                    m_state = DialogueState::Playing;
                    updateUI();
                }
                break;
                
            case DialogueState::Playing:
                if (m_tree.getCurrentNode()->autoAdvance) {
                    m_stateTimer += dt;
                    if (m_stateTimer > m_tree.getCurrentNode()->autoAdvanceDelay) {
                        advance(0);  // Auto-advance
                    }
                }
                break;
                
            case DialogueState::Ending:
                m_stateTimer += dt;
                if (m_stateTimer > 0.2f) {
                    m_state = DialogueState::Idle;
                    m_tree.reset();
                }
                break;
                
            default:
                break;
        }
    }
};
```

### Example Dialogue Tree (JSON)

```json
{
  "dialogue_merchant": {
    "nodes": [
      {
        "id": "start",
        "npcName": "Merchant",
        "text": "Welcome! What brings you here?",
        "autoAdvance": false,
        "choices": [
          {
            "text": "I want to buy",
            "nextNode": "shop_menu",
            "condition": null
          },
          {
            "text": "Just looking",
            "nextNode": "end",
            "condition": null
          }
        ]
      },
      {
        "id": "shop_menu",
        "npcName": "Merchant",
        "text": "We have fine wares!",
        "autoAdvance": true,
        "autoAdvanceDelay": 2.0,
        "choices": []
      },
      {
        "id": "end",
        "npcName": "Merchant",
        "text": "Come back soon!",
        "autoAdvance": true,
        "autoAdvanceDelay": 1.5,
        "choices": []
      }
    ]
  }
}
```

### Implementation Effort

**Time: 20-24 hours** (includes JSON parsing, state machine, UI integration)

---

## 5. Implementation Roadmap

### Phase 1: Quick Wins (Week 1)
- [ ] Archetype cache ECS (6-8h) - **HIGHEST ROI**
- [ ] System query caching (4-6h)
- [ ] Testing + benchmarking (4-6h)

**Expected result: 10-50x speedup for ECS queries**

### Phase 2: Medium Term (Week 2-3)
- [ ] UI draw call batching (12-16h)
- [ ] Dialogue state machine (20-24h)
- [ ] Integration testing (8-10h)

**Expected result: 10-20x speedup UI, advanced dialogue**

### Phase 3: Polish & Docs (Week 4)
- [ ] Performance benchmarking report
- [ ] Update documentation with optimization guides
- [ ] Create performance testing suite

---

## 6. Validation & Testing

### ECS Performance Benchmarks

```cpp
// tests/Benchmarks/benchmark_ecs_queries.cpp
#include <benchmark/benchmark.h>

static void BM_QueryBefore(benchmark::State& state) {
    EntityRegistry registry;
    
    // Create 1000 entities
    for (int i = 0; i < 1000; ++i) {
        auto* e = registry.createEntity();
        e->addComponent(std::make_unique<TransformComponent>());
        e->addComponent(std::make_unique<SpriteComponent>());
    }
    
    for (auto _ : state) {
        auto results = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_QueryBefore);

static void BM_QueryAfter(benchmark::State& state) {
    // Same test with archetype cache
    // Expected: 1000x faster
}
BENCHMARK(BM_QueryAfter);
```

### Expected Results

```
BM_QueryBefore:    500 ns  (linear scan)
BM_QueryAfter:     0.5 ns  (cache hit)
Speedup:           1000x
```

---

## 7. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Breaking existing code | Low | High | Maintain backward-compatible API |
| Cache invalidation bugs | Medium | High | Comprehensive unit tests |
| Memory overhead (archetype cache) | Low | Low | Only store signatures that are queried |
| UI batching compatibility | Medium | Medium | Gradual rollout, fallback to old code |

---

## 8. Cost-Benefit Analysis

**Estimated Effort:** 80-120 hours (2.5-3 person-weeks)
**Expected Speedup:** 10-50x for common workloads
**ROI:** **Extremely High** - Foundation for scaling engine

**Comparison with alternatives:**
- Rewrite in Rust: 500+ hours, lose all existing code
- Use Unreal/Godot: Lose all learning value, commercial
- Optimize current: 80 hours, massive gains, maintainable

---

## Conclusion

Ces optimisations transforment NovaEngine d'un prototype pédagogique en moteur performant capable de gérer **10,000+ entités en 60 FPS**.

**Recommendation: Implement Phase 1 immediately (2 week sprint)**

