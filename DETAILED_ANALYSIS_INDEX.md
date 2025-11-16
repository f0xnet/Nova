# NovaEngine - Index Détaillé de l'Analyse

**Date:** 16 Novembre 2025  
**Niveau d'analyse:** Extremely Thorough  
**Fichier principal:** TECHNICAL_DOCUMENTATION_COMPLETE.md (2577 lignes, 76K)

---

## SECTION 1: ARCHITECTURES SYSTÈME (Lignes 1-200)

### 1.1 Vue Globale
- **Lignes 1-50:** Header et métadonnées
- **Lignes 51-100:** Table des matières et organisation
- **Lignes 101-200:** Diagramme architecture en couches (7 niveaux)

**Clés:** Application → ECS → UI → Core → Backend → SFML → Graphics

---

## SECTION 2: ECS SYSTEM - ENTITY COMPONENT SYSTEM (Lignes 201-2000)

### 2.1 Fondamentaux Component (Lignes 201-400)

**Component.hpp:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Component.hpp`

**Structure:** Interface virtuelle pure

```cpp
class Component {
    virtual ~Component() = default;
    virtual ComponentTypeID getTypeID() const = 0;          // Type identification
    virtual void serialize(nlohmann::json& json) const = 0; // JSON export
    virtual void deserialize(const nlohmann::json& json) = 0; // JSON import
};

// Helper macro
#define COMPONENT_TYPE_ID(TypeName) \
    ComponentTypeID getTypeID() const override { return #TypeName; }
```

**Usage Pattern:**
- Chaque composant concrète hérite Component
- Utilise macro COMPONENT_TYPE_ID(XxxComponent) dans classe
- Implémente serialize/deserialize pour persistance

**Responsabilité:** Contenu pur données (pas de logique)

---

### 2.2 Components Implémentés (Lignes 401-1200)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (562 lignes)

#### A) TransformComponent (Lignes 401-450)
**Responsabilité:** Transformation spatiale

**Propriétés:**
- `Vec2f position` - Position pixels
- `f32 rotation` - Degrés
- `Vec2f scale` - Multiplicateur (1.0 = normal)
- `Vec2f origin` - Point pivot

**Interactions:** TOUS systèmes (Render, Physics, Audio, Animation)

#### B) SpriteComponent (Lignes 451-550)
**Responsabilité:** Rendu 2D

**Propriétés:**
- `ID textureID` - Référence ResourceManager
- `TextureHandle textureHandle` - Handle rendu SFML
- `IntRect textureRect` - Sub-region (0,0,0,0 = full)
- `Vec2f size` - Dimensions affichage
- `Color tint` - Teinte
- `BlendMode blendMode` - Alpha/Add/Multiply/None
- `i32 zOrder` - Ordre de rendu (lower = behind)
- `bool visible` - Visibilité

**Système:** RenderSystem trie par zOrder, utilise textureHandle pour GRAPHICS().drawSprite()

#### C) LightComponent (Lignes 551-650)
**Responsabilité:** Éclairage

**Énums:**
- `LightType:` Point, Directional, Spot
- Point: radius, intensity
- Directional/Spot: direction, angle
- castShadows: bool (non implémenté)

**Système:** LightSystem dessine visualisation simple (cercle semi-transparent pour Point)

#### D) AnimationComponent (Lignes 651-750)
**Responsabilité:** Animation frame-based

**Propriétés:**
- `std::vector<IntRect> frames` - Rectangles textures
- `f32 frameDuration` - Durée par frame
- `f32 currentTime` - Temps accumulé
- `u32 currentFrame` - Index frame courant
- `bool loop` - Boucler?
- `bool playing` - État

**Système:** AnimationSystem avance currentTime par deltaTime, quand >= frameDuration → frame++

#### E) ColliderComponent (Lignes 751-850)
**Responsabilité:** Collision physique

**Énums:**
- `ColliderType:` Box, Circle

**Propriétés:**
- `Vec2f size` - Pour Box
- `f32 radius` - Pour Circle
- `Vec2f offset` - Décalage depuis position
- `bool isTrigger` - Trigger ou solide
- `bool enabled` - Actif

**Système:** PhysicsSystem teste AABB collisions (Box-Box uniquement)

#### F) AudioComponent (Lignes 851-950)
**Responsabilité:** Son/Musique

**Propriétés:**
- `ID soundID` - Référence ResourceManager
- `SoundHandle soundHandle` - Handle rendu
- `bool playOnStart` - Lancer auto
- `bool loop` - Boucler
- `f32 volume` - 0-100
- `f32 pitch` - 0.5-2.0 typiquement

**Système:** AudioSystem appelle AUDIO().playSound() si playOnStart=true

#### G) ActivatorComponent (Lignes 951-1100)
**Responsabilité:** Zones trigger/activation

**Énums:**
- `ActivatorType:` Proximity, Manual, Automatic
- `ActivatorShape:` Box, Circle

**Propriétés clés:**
- `ActivatorType type` - Mode activation
- `bool isActive` - État courant
- `f32 cooldownTime` - Délai réactivation
- `std::string targetTag` - Quelle entité active
- `std::string actionID` - ID action logique jeu
- Événements: `onActivateEvent`, `onDeactivateEvent`

**Types:**
1. **Proximity:** S'active une fois à entrée, se désactive à sortie
2. **Automatic:** Reste actif tant qu'entité dans zone
3. **Manual:** Nécessite interaction utilisateur

**Système:** ActivatorSystem teste collision Zone-Entity (Box rect.contains(pos) / Circle distance)

#### H) TagComponent (Lignes 1101-1150)
**Responsabilité:** Identification entité

```cpp
class TagComponent {
    std::string tag = "default";  // "player", "npc", "enemy", etc.
};
```

**Usage:** ActivatorSystem filtre entités par `targetTag`

#### I) SceneTransitionComponent (Lignes 1151-1200)
**Responsabilité:** Marqueur transition inter-scène

```cpp
class SceneTransitionComponent {
    std::string targetScene;
    Vec2f targetPosition;
    bool isTransitioning = false;
};
```

**Usage:** JourneySystem marque entités à transférer entre scènes

#### J) JourneyComponent (Lignes 1201-1350)
**Responsabilité:** Voyage multi-scène NPCs

**Multi-scène:**
- `std::vector<std::string> scenePath` - Chemin complet
- `int currentSceneIndex` - Index courant
- `Vec2f currentDestination` - Destination scène courante
- `bool reachedCurrentDestination` - Flag arrivée

**Pathfinding Local:**
- `std::vector<Vec2f> localWaypointPath` - Waypoints scène courante
- `int currentLocalWaypointIndex` - Waypoint courant

**Personnalité:**
- `std::vector<std::string> preferredPathTags` - Chemins préférés

**Système:** JourneySystem contrôle mouvement multi-scène complet

---

### 2.3 Entity Class (Lignes 1351-1450)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Entity.hpp` (144 lignes)

```cpp
class Entity {
private:
    u64 m_id;
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;
    
public:
    u64 getID() const;
    
    // Template methods
    template<typename T> T* addComponent(std::unique_ptr<T> component);
    template<typename T> T* getComponent();
    template<typename T> const T* getComponent() const;
    template<typename T> bool hasComponent() const;
    template<typename T> void removeComponent();
    
    bool hasComponent(const ComponentTypeID& typeID) const;
    std::vector<ComponentTypeID> getComponentTypes() const;
};
```

**Storage:** unordered_map<string, unique_ptr<Component>>

**Type Safety:** Templates à la compilation, ID string runtime fallback

---

### 2.4 EntityRegistry Class (Lignes 1451-1550)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/EntityRegistry.hpp` (133 lignes)

```cpp
class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    u64 m_nextID = 1;
    
public:
    Entity* createEntity();                                    // ID auto-incrémenté
    void destroyEntity(u64 entityID);
    Entity* getEntity(u64 entityID);
    
    // Requête: toutes entités avec ces composants
    std::vector<Entity*> getEntitiesWith(
        const std::vector<ComponentTypeID>& componentTypes);
    
    std::vector<Entity*> getAllEntities();
    size_t getEntityCount() const;
    void clear();
};
```

**Requêtes:** Itère tous les entités, teste hasComponent() pour chaque type requis

**Utilisé par:** Scene::update(), System::update()

---

### 2.5 System Base Class (Lignes 1551-1600)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/System.hpp` (46 lignes)

```cpp
class System {
public:
    virtual ~System() = default;
    
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;
    
    virtual void onInit() {}
    virtual void onShutdown() {}
};
```

**Contract:** Chaque système déclare composants requis, exécute logique sur matched entities

---

### 2.6 Systems Implémentés (Lignes 1601-2000)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (696 lignes)

#### a) RenderSystem (Lignes 1601-1650)
**Required:** TransformComponent, SpriteComponent
**Logic:**
1. Requête entities with both components
2. Sort by SpriteComponent.zOrder (lower = behind)
3. For each visible entity:
   - Prepare SpriteData from components
   - Call GRAPHICS().drawSprite(data)

#### b) AnimationSystem (Lignes 1651-1700)
**Required:** SpriteComponent, AnimationComponent
**Logic:**
1. Advance currentTime by deltaTime
2. When currentTime >= frameDuration:
   - currentFrame++
   - currentTime reset to 0
3. If currentFrame >= frames.size():
   - If loop=true: currentFrame = 0
   - Else: playing = false, freeze on last frame
4. Update sprite.textureRect = frames[currentFrame]

#### c) LightSystem (Lignes 1701-1750)
**Required:** TransformComponent, LightComponent
**Logic:** For Point lights: draw semi-transparent circle at position with radius

#### d) AudioSystem (Lignes 1751-1800)
**Required:** AudioComponent
**Logic:** On first frame: if playOnStart=true and !playing → AUDIO().playSound()

#### e) PhysicsSystem (Lignes 1801-1850)
**Required:** TransformComponent, ColliderComponent
**Logic:** N² collision check, Box-Box AABB intersection only

#### f) ActivatorSystem (Lignes 1851-1950)
**Required:** TransformComponent, ActivatorComponent (plus TagComponent for targets)
**Logic:**
1. Update cooldowns
2. For each activator:
   - Calculate activation zone (box/circle)
   - Test if any entity.tag == targetTag inside zone
   - Handle activation based on ActivatorType
   - Draw debug zone if showDebugZone=true

#### g) JourneySystem (Lignes 1951-2000)
**Required:** TransformComponent, SceneTransitionComponent, JourneyComponent
**Logic:**
1. Follow localWaypointPath if available
2. Otherwise go direct to currentDestination
3. When destination reached:
   - If more scenes in path: mark for transfer
   - Else: journey complete

---

## SECTION 3: BACKEND ARCHITECTURE (Lignes 2001-2200)

### 3.1 Backend Types (Lignes 2001-2050)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/Core/BackendTypes.hpp` (186 lignes)

**Primitive Types:**
```cpp
using i8 = int8_t;   using i16 = int16_t;   using i32 = int32_t;   using i64 = int64_t;
using u8 = uint8_t;  using u16 = uint16_t;  using u32 = uint32_t;  using u64 = uint64_t;
using f32 = float;    using f64 = double;
using String = std::string;
```

**Resource Handles:** u64 (INVALID_HANDLE = 0)
- TextureHandle, FontHandle, SoundHandle, MusicHandle, ShaderHandle

**Enums:**
- `BackendType:` SFML, SDL, Custom
- `KeyCode:` A-Z, Num0-9, Escape, Space, Enter, Arrows, etc.
- `MouseButton:` Left, Right, Middle
- `InputEventType:` Closed, Resized, KeyPressed, KeyReleased, MouseButton*, MouseMoved, TextEntered
- `BlendMode:` Alpha, Add, Multiply, None
- `TextStyle:` Regular, Bold (bitwise flags), Italic, Underlined, StrikeThrough
- `SoundStatus:` Stopped, Paused, Playing

**Structs:**
- `Vec2f, Vec2i, Vec2u` - Math vectors with operators
- `Rect, IntRect` - Rectangles with contains()
- `Color` - RGBA with predefined colors
- `InputEvent` - Union for different input types
- `SpriteData, RectData, TextData` - Rendering data
- `ViewportData` - Camera/view data
- `TextMetrics` - Text measurement (width, height, baseline)

---

### 3.2 Backend Manager (Lignes 2051-2150)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/BackendManager.hpp` (67 lignes)

**Pattern:** Singleton + Facade

```cpp
class BackendManager {
public:
    static BackendManager& get();  // Singleton access
    
    bool initialize(BackendType type, u32 width, u32 height,
                   const String& title, bool fullscreen);
    void shutdown();
    bool isInitialized() const;
    
    // Facade: accessors to all interfaces
    IWindowBackend& window();
    IInputBackend& input();
    IGraphicsBackend& graphics();
    IResourceBackend& resources();
    IAudioBackend& audio();
    IFontBackend& fonts();
    IViewportBackend& viewport();
    
private:
    std::unique_ptr<IWindowBackend> m_window;
    std::unique_ptr<IInputBackend> m_input;
    std::unique_ptr<IGraphicsBackend> m_graphics;
    std::unique_ptr<IResourceBackend> m_resources;
    std::unique_ptr<IAudioBackend> m_audio;
    std::unique_ptr<IFontBackend> m_fonts;
    std::unique_ptr<IViewportBackend> m_viewport;
};

// Macros for easy access
#define WINDOW()    BackendManager::get().window()
#define INPUT()     BackendManager::get().input()
#define GRAPHICS()  BackendManager::get().graphics()
#define RESOURCES() BackendManager::get().resources()
#define AUDIO()     BackendManager::get().audio()
#define FONTS()     BackendManager::get().fonts()
#define VIEWPORT()  BackendManager::get().viewport()
```

---

### 3.3 Backend Interfaces (Lignes 2151-2200)

**Location:** `/home/user/Nova/sdk/include/NovaEngine/Backend/Interfaces/`

**7 Interfaces:**
1. IWindowBackend - Window management
2. IGraphicsBackend - Rendering
3. IInputBackend - Input handling
4. IResourceBackend - Texture caching
5. IAudioBackend - Audio playback
6. IFontBackend - Font handling
7. IViewportBackend - Camera/view

---

## SECTION 4: UI SYSTEM (Lignes 2201-2350)

### 4.1 UIComponent Base (Lignes 2201-2250)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIComponent.hpp` (62 lignes)

```cpp
class UIComponent : public EventHandler {
protected:
    ID m_id;
    Vec2f m_position, m_size;
    bool m_visible, m_active;
    ID m_groupID, m_uiID;
    i32 m_layer;
    std::string m_effect, m_description;
    
public:
    virtual void update(f32 deltaTime);
    virtual void render() const = 0;          // Pure virtual
    virtual void onEvent(const Event& event) override = 0;  // Pure virtual
    virtual Rect getBounds() const = 0;      // Pure virtual
    
    // Setters/getters
    void setPosition(const Vec2f& pos);
    virtual void setSize(const Vec2f& size);
    // ... etc
};
```

---

### 4.2 UIManager (Lignes 2251-2350)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIManager.hpp` (65 lignes)

**Key Features:**
- Add/remove components
- Manage groups, UIs, layers
- Layered rendering (sorted by layer)
- Action callbacks for button presses
- Event dispatching

---

## SECTION 5: SCENE MANAGEMENT (Lignes 2351-2500)

### 5.1 Scene Class (Lignes 2351-2400)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Scene.hpp` (585 lignes)

```cpp
class Scene {
private:
    std::string m_name, m_type;  // "interior"/"exterior"
    Color m_backgroundColor;
    
    EntityRegistry m_entityRegistry;
    std::vector<std::unique_ptr<System>> m_systems;
    WaypointGraph m_waypointGraph;  // NPC pathfinding
    
public:
    bool loadFromJSON(const nlohmann::json& sceneData,
                     const DefinitionManager& defManager);
    
    void update(float deltaTime);   // Runs all systems
    void render();                   // Clear background
    
    // Entity creation helpers
    void createSpriteEntity(...);
    void createLightEntity(...);
    void createAnimatedSpriteEntity(...);
    void createAudioEntity(...);
    void createActivatorEntity(...);
    void createPlayerEntity(...);
};
```

**System Order (constructor):**
1. AnimationSystem
2. PhysicsSystem
3. ActivatorSystem
4. AudioSystem
5. LightSystem
6. RenderSystem (last, for final rendering)

---

### 5.2 SceneManager (Lignes 2401-2450)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/SceneManager.hpp` (303 lignes)

```cpp
class SceneManager {
private:
    DefinitionManager m_definitionManager;
    SceneGraph m_sceneGraph;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::unordered_set<std::string> m_activeScenesForUpdate;
    
public:
    bool initialize(const std::string& definitionsPath,
                   const std::string& sceneGraphPath);
    
    bool loadScene(const std::string& scenePath,
                  const std::string& sceneName);
    void setActiveScene(const std::string& sceneName);
    
    void update(float deltaTime);  // Smart update (only active scenes)
    void render();
};
```

**Smart Update Logic:**
- Collects all scenes with NPCs on journeys
- Updates only activeScene + scenes on active NPC paths
- Sleeping scenes (no updates) = performance optimization

---

### 5.3 DefinitionManager (Lignes 2451-2500)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/DefinitionManager.hpp` (327 lignes)

**Two-Tier System:**
- **Tier 1 (Startup):** Load ALL definitions once (Sprites.json, Lights.json, etc.)
- **Tier 2 (Per-Scene):** Scenes reference definitions by ID

**Expected Files:**
```
assets/data/definitions/
├── Sprites.json        # {"sprites": [...]}
├── Lights.json         # {"lights": [...]}
├── Animations.json     # {"animations": [...]}
├── Audio.json          # {"sounds": [...]}
└── Activators.json     # {"activators": [...]}
```

---

## SECTION 6: PATHFINDING SYSTEMS (Lignes 2501-2577)

### 6.1 WaypointGraph - Scène Local (Lignes 2501-2550)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/WaypointGraph.hpp` (396 lignes)

**Use Case:** NPCs pathfinding within a scene

**Structures:**
```cpp
struct Waypoint {
    std::string id;              // "fountain", "plaza"
    Vec2f position;              // Physical location
    std::vector<std::string> tags;  // "main_road", "shortcut"
};

struct WaypointConnection {
    std::string from, to;
    f32 cost;
    std::vector<std::string> tags;
    bool bidirectional;
};
```

**Algorithm:** BFS (Breadth-First Search)
- `findPathByID()` - Returns waypoint IDs
- `findPath()` - Returns Vec2f positions
- Tag filtering for personality-based paths

**JSON Format:**
```json
{
    "waypoints": [
        {"id": "fountain", "position": [640, 360], "tags": ["landmark"]},
        ...
    ],
    "connections": [
        {"from": "fountain", "to": "plaza", "tags": ["main_road"], "bidirectional": true},
        ...
    ]
}
```

---

### 6.2 SceneGraph - Multi-Scene Travel (Lignes 2551-2577)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/SceneGraph.hpp` (208 lignes)

**Use Case:** Multi-scene NPC journeys

```cpp
struct SceneConnection {
    std::string fromScene, toScene;
    Vec2f exitPortalPos;      // Where to exit fromScene
    Vec2f entryPortalPos;     // Where to arrive in toScene
    f32 travelTime;           // Estimated time (unused)
    bool bidirectional;
};
```

**Algorithm:** BFS for scene routing
- `findPath()` - Returns scene path
- `getConnection()` - Portal info between adjacent scenes
- Enables realistic NPC travel through intermediary scenes

---

## FILE LOCATIONS SUMMARY

### Headers (SDK)
```
/home/user/Nova/sdk/include/NovaEngine/
├── ECS/
│   ├── Component.hpp
│   ├── Components.hpp         [10 components: Transform, Sprite, Light, Animation, Collider, Audio, Activator, Tag, SceneTransition, Journey]
│   ├── Entity.hpp
│   ├── EntityRegistry.hpp
│   ├── System.hpp
│   ├── Systems.hpp            [7 systems: Render, Animation, Light, Audio, Physics, Activator, Journey]
│   ├── Scene.hpp
│   ├── SceneManager.hpp
│   ├── DefinitionManager.hpp
│   ├── WaypointGraph.hpp
│   └── SceneGraph.hpp
├── Backend/
│   ├── BackendManager.hpp
│   ├── Core/BackendTypes.hpp
│   └── Interfaces/
│       ├── IWindowBackend.hpp
│       ├── IGraphicsBackend.hpp
│       ├── IInputBackend.hpp
│       ├── IResourceBackend.hpp
│       ├── IAudioBackend.hpp
│       ├── IFontBackend.hpp
│       └── IViewportBackend.hpp
├── UI/
│   ├── UIManager.hpp
│   ├── UIComponent.hpp
│   ├── UILoader.hpp
│   └── Components/
│       ├── Button.hpp
│       ├── Text.hpp
│       ├── Image.hpp
│       ├── Panel.hpp
│       ├── Slider.hpp
│       ├── TextInput.hpp
│       └── Animation.hpp
├── Core/
│   ├── Application.hpp
│   ├── Logger.hpp
│   ├── ConfigManager.hpp
│   └── NovaEngine.hpp
├── Events/
│   ├── Event.hpp
│   ├── EventDispatcher.hpp
│   └── EventHandler.hpp
└── Resources/
    ├── ResourceManager.hpp
    └── ResourceTypes.hpp
```

### Implementations (Client)
```
/home/user/Nova/client/src/
├── Backend/
│   ├── BackendManager.cpp
│   ├── SFML/
│   │   ├── SFMLWindowBackend.cpp
│   │   ├── SFMLGraphicsBackend.cpp
│   │   ├── SFMLInputBackend.cpp
│   │   ├── SFMLResourceBackend.cpp
│   │   ├── SFMLAudioBackend.cpp
│   │   ├── SFMLFontBackend.cpp
│   │   └── SFMLViewportBackend.cpp
│   └── Core/BackendTypes.cpp
├── UI/
│   ├── UIManager.cpp
│   ├── UIComponent.cpp
│   ├── UILoader.cpp
│   └── Components/
│       ├── Button.cpp
│       ├── Text.cpp
│       ├── Image.cpp
│       ├── Panel.cpp
│       ├── Slider.cpp
│       ├── TextInput.cpp
│       └── Animation.cpp
├── Core/
│   ├── NovaEngine.cpp
│   ├── Logger.cpp
│   └── ConfigManager.cpp
├── Events/
│   ├── Event.cpp
│   ├── EventDispatcher.cpp
│   └── EventHandler.cpp
├── Resources/
│   ├── ResourceManager.cpp
│   └── ResourceTypes.cpp
├── Audio/
│   ├── AudioManager.cpp
│   ├── SoundPlayer.cpp
│   └── MusicPlayer.cpp
├── Dialogue/
│   ├── DialogueSystem.cpp
│   ├── DialogueComponent.hpp
│   └── DialogueSystem.hpp
├── Player/
│   ├── PlayerController.cpp
│   └── PlayerController.hpp
├── Game.cpp
└── main.cpp
```

---

## KEY DESIGN PATTERNS

1. **Singleton:** Logger, BackendManager, NovaEngine
2. **Facade:** BackendManager (7 interfaces)
3. **Strategy:** Component update, System logic, UI rendering
4. **Template Method:** Application main loop
5. **Factory:** Scene entity creation
6. **Observer:** Event dispatcher
7. **Component Pattern:** ECS architecture
8. **Data-Oriented:** Components as pure data

---

## EXECUTION FLOWS

### Application Startup
```
main()
  ↓ Game::run() [Application::run()]
    ↓ initializeEngine()
      ├─ BackendManager::initialize(SFML)
      │   ├─ createBackends()
      │   └─ SFML backend initialization
      └─ WINDOW().setVSync(), setFramerateLimit()
    ↓ Game::onInitialize()
      ├─ SceneManager::initialize()
      │   ├─ DefinitionManager::loadDefinitions()
      │   │   ├─ loadSpriteDefinitions()
      │   │   ├─ loadLightDefinitions()
      │   │   ├─ loadAnimationDefinitions()
      │   │   ├─ loadAudioDefinitions()
      │   │   └─ loadActivatorDefinitions()
      │   └─ SceneGraph::loadFromJSON()
      ├─ SceneManager::loadScene()
      │   └─ Scene::loadFromJSON()
      │       ├─ WaypointGraph::loadFromJSON()
      │       └─ Entity creation loop
      ├─ SceneManager::setActiveScene()
      ├─ UIManager setup
      └─ DialogueSystem::initialize()
    ↓ runMainLoop()
```

### Main Loop (Each Frame)
```
while (WINDOW().isOpen())
  ├─ Process Events
  │   ├─ INPUT().pollEvent()
  │   └─ Game::onEvent()
  ├─ Game::onUpdate(deltaTime)
  │   ├─ PlayerController update
  │   ├─ SceneManager::update()
  │   │   └─ For each active scene:
  │   │       ├─ AnimationSystem::update()
  │   │       ├─ PhysicsSystem::update()
  │   │       ├─ ActivatorSystem::update()
  │   │       ├─ AudioSystem::update()
  │   │       ├─ LightSystem::update()
  │   │       └─ RenderSystem::update() [prepares data]
  │   ├─ UIManager::update()
  │   └─ DialogueSystem::update()
  ├─ Game::onRender()
  │   ├─ WINDOW().clear()
  │   ├─ SceneManager::render()
  │   │   └─ GRAPHICS().drawSprite() × N
  │   ├─ UIManager::render()
  │   │   └─ UIComponent::render() × M
  │   ├─ Dialogue rendering
  │   └─ WINDOW().display()
```

---

## PERFORMANCE CHARACTERISTICS

- **Entity Creation:** O(1) amortized
- **Component Access:** O(1) HashMap lookup
- **System Updates:** O(n) where n = matched entities
- **Scene Pathfinding:** O(V+E) BFS
- **Waypoint Pathfinding:** O(W+C) BFS
- **Render Sorting:** O(n log n) z-order sort
- **UI Rendering:** O(n log l) where l = layers

---

## EXTENSION POINTS

1. **New Components:** Inherit Component, implement serialize/deserialize
2. **New Systems:** Inherit System, define required components
3. **New UI Components:** Inherit UIComponent, implement render/onEvent
4. **New Backends:** Implement IXxxBackend interfaces
5. **Game Mechanics:** Use Application::onUpdate() / onEvent()

---

## DOCUMENTATION STATISTICS

- **Total Lines:** 2577 (this file + main doc)
- **File Size:** 76K
- **Sections:** 10 major
- **Classes Documented:** 40+
- **Methods Analyzed:** 150+
- **Code Snippets:** 50+
- **Flow Diagrams:** 5

---

**End of Index**  
For detailed analysis, see: `/home/user/Nova/TECHNICAL_DOCUMENTATION_COMPLETE.md`

