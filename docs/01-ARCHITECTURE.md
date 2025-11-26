# Architecture NovaEngine

## Vue d'ensemble

NovaEngine utilise une **architecture en couches** combinée avec le pattern **Entity Component System (ECS)**. Cette architecture favorise la modularité, la séparation des responsabilités et la facilité de maintenance.

## Architecture en couches

```
┌──────────────────────────────────────────────────────────────┐
│                    COUCHE APPLICATION                         │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ Game (extends Application)                             │  │
│  │ - Orchestration générale                               │  │
│  │ - Gestion du game loop                                 │  │
│  │ - Intégration des sous-systèmes                        │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                           ↓
┌──────────────────────────────────────────────────────────────┐
│                  COUCHE GAME SYSTEMS                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │  Dialogue   │  │   Player    │  │     UI      │          │
│  │   System    │  │ Controller  │  │   Manager   │          │
│  └─────────────┘  └─────────────┘  └─────────────┘          │
│  ┌─────────────────────────────────────────────────────┐    │
│  │        Post-Processing Pipeline                      │    │
│  │  CRT, Bloom, SSAO, Lighting, ColorGrading           │    │
│  └─────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
                           ↓
┌──────────────────────────────────────────────────────────────┐
│                    COUCHE ECS                                 │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  SceneManager                                          │  │
│  │  ┌──────────┐  ┌──────────┐  ┌────────────┐          │  │
│  │  │  Scene   │  │  Scene   │  │   Scene    │          │  │
│  │  │   #1     │  │   #2     │  │    #3      │          │  │
│  │  └──────────┘  └──────────┘  └────────────┘          │  │
│  │                                                        │  │
│  │  Chaque Scene contient:                               │  │
│  │  - EntityRegistry (base de données d'entités)         │  │
│  │  - Systems (RenderSystem, AnimationSystem, etc.)      │  │
│  │  - WaypointGraph (pathfinding)                        │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  DefinitionManager (définitions réutilisables)         │  │
│  │  SceneGraph (connexions inter-scènes)                  │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                           ↓
┌──────────────────────────────────────────────────────────────┐
│              COUCHE BACKEND ABSTRACTION                       │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  BackendManager (Singleton)                            │  │
│  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐     │  │
│  │  │Graphics │ │ Window  │ │  Input  │ │  Audio  │     │  │
│  │  │ Backend │ │ Backend │ │ Backend │ │ Backend │     │  │
│  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘     │  │
│  │  ┌─────────┐ ┌─────────┐ ┌─────────┐                 │  │
│  │  │  Font   │ │Resource │ │Viewport │                 │  │
│  │  │ Backend │ │ Backend │ │ Backend │                 │  │
│  │  └─────────┘ └─────────┘ └─────────┘                 │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                           ↓
┌──────────────────────────────────────────────────────────────┐
│            COUCHE IMPLÉMENTATION (SFML)                       │
│  - SFMLGraphicsBackend                                        │
│  - SFMLWindowBackend                                          │
│  - SFMLInputBackend                                           │
│  - SFMLAudioBackend                                           │
│  - SFMLFontBackend                                            │
│  - SFMLResourceBackend                                        │
│  - SFMLViewportBackend                                        │
│                                                               │
│  → OpenGL pour les shaders                                    │
└──────────────────────────────────────────────────────────────┘
```

## Patterns de conception utilisés

### 1. Entity Component System (ECS)

**Principe** : Séparation stricte entre données (Components) et logique (Systems).

```cpp
// Entité = simple conteneur d'ID + composants
Entity entity(123);

// Composants = pure data
auto transform = std::make_unique<TransformComponent>();
transform->position = Vec2f{100, 200};
entity.addComponent(std::move(transform));

// Systèmes = logique pure
class RenderSystem : public System {
    void update(float dt, EntityRegistry& registry) {
        // Opère sur toutes les entités avec Transform + Sprite
        auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});
        for (Entity* e : entities) {
            // Rendu...
        }
    }
};
```

**Avantages** :
- Composition flexible d'entités
- Ajout facile de nouveaux comportements
- Séparation données/logique claire
- Performance (data-oriented design)

### 2. Singleton

**Utilisé pour** : BackendManager, Logger, ConfigManager

```cpp
// Accès global via instance unique
BackendManager& backend = BackendManager::get();

// Macros pour simplification
#define GRAPHICS() BackendManager::get().graphics()
#define WINDOW() BackendManager::get().window()
```

**Justification** :
- Un seul backend graphique par application
- Accès global nécessaire depuis n'importe où
- Évite le "dependency injection hell"

### 3. Strategy Pattern

**Utilisé pour** : Backends interchangeables

```cpp
// Interface commune
class IGraphicsBackend {
    virtual void drawSprite(const SpriteData& data) = 0;
    // ...
};

// Implémentations multiples
class SFMLGraphicsBackend : public IGraphicsBackend { /* ... */ };
class SDLGraphicsBackend : public IGraphicsBackend { /* ... */ };

// Sélection au runtime
BackendManager::initialize(BackendType::SFML);
```

**Avantages** :
- Portabilité entre bibliothèques graphiques
- Tests facilités (mock backends)
- Flexibilité

### 4. Template Method

**Utilisé pour** : Application base class

```cpp
class Application {
public:
    int run() {
        initializeEngine();
        onInitialize();      // ← Hook pour sous-classe

        while (window.isOpen()) {
            onUpdate(dt);     // ← Hook
            onRender();       // ← Hook
        }

        onShutdown();        // ← Hook
        shutdownEngine();
    }

protected:
    virtual bool onInitialize() = 0;
    virtual void onUpdate(float dt) = 0;
    virtual void onRender() = 0;
};

// Game implémente les hooks
class Game : public Application {
    bool onInitialize() override { /* ... */ }
    void onUpdate(float dt) override { /* ... */ }
    void onRender() override { /* ... */ }
};
```

### 5. Observer Pattern

**Utilisé pour** : Système d'événements

```cpp
// Dispatcher
EventDispatcher dispatcher;

// Observer
dispatcher.subscribe(EventType::KeyPressed, [](const Event& e) {
    // Réaction à l'événement
});

// Émission
Event event(InputEventType::KeyPressed);
dispatcher.dispatch(event);
```

### 6. Factory Pattern

**Utilisé pour** : Création d'entités depuis JSON

```cpp
// DefinitionManager = Factory
Entity* entity = registry.createEntity();

// Création basée sur type
if (type == "sprite") {
    createSpriteEntity(entity, json, definitions);
}
else if (type == "light") {
    createLightEntity(entity, json, definitions);
}
```

### 7. Adapter Pattern

**Utilisé pour** : Adaptation SFML → API NovaEngine

```cpp
// Conversion types SFML → NovaEngine
Vec2f SFMLConversions::toVec2f(const sf::Vector2f& vec) {
    return Vec2f{vec.x, vec.y};
}

Color SFMLConversions::toColor(const sf::Color& color) {
    return Color{color.r, color.g, color.b, color.a};
}
```

### 8. Composite Pattern

**Utilisé pour** : Hiérarchie UI

```cpp
// UIComponent = base
class UIComponent { /* ... */ };

// Container = composite
class Panel : public UIComponent {
    std::vector<UIComponent*> children;
};

// Leaf components
class Button : public UIComponent { /* ... */ };
class Text : public UIComponent { /* ... */ };
```

## Flux de données

### Démarrage de l'application

```
main()
  ↓
Game::Game()
  → Initialise membres (DialogueSystem, PlayerController, etc.)
  ↓
Game::run() [hérité de Application]
  ↓
Application::initializeEngine()
  → BACKEND().initialize(BackendType::SFML)
  → Crée tous les backends SFML
  → Configure fenêtre, VSync, FPS
  ↓
Game::onInitialize()
  → Configure viewport avec letterboxing
  → SceneManager::initialize()
    → Charge définitions (Sprites.json, Lights.json, etc.)
    → Charge SceneGraph pour pathfinding
  → SceneManager::loadScene("test.json")
    → Parse JSON scène
    → Crée entités depuis définitions
    → Configure systèmes ECS
  → Initialise UIManager
  → Charge UI dialogues
  → Initialise PostProcessPipeline
    → Ajoute effets (SSAO, Bloom, ColorGrading, DynamicLighting)
  → Trouve entité joueur
  ↓
Application::runMainLoop()
  → Boucle principale
```

### Game Loop (frame par frame)

```
┌─────────────────────────────────────────────┐
│         DÉBUT DE FRAME                      │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│   1. TRAITEMENT DES ÉVÉNEMENTS               │
│   ────────────────────────────────────────   │
│   Application::processEvents()               │
│     → INPUT().pollEvent(event)               │
│     → Conversion InputEvent → Event          │
│     → Game::onEvent(event)                   │
│       → UIManager::dispatchEvent()           │
│       → Gestion input clavier                │
│         (E=dialogue, T=time, 1-4=effets)     │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│   2. UPDATE (LOGIQUE)                        │
│   ────────────────────────────────────────   │
│   Game::onUpdate(deltaTime)                  │
│     → PlayerController::updateMovement()     │
│       → Détecte collisions                   │
│       → Déplace joueur                       │
│     → PlayerController::updateNPCDetection() │
│       → Trouve NPC le plus proche            │
│     → VIEWPORT().setViewCenter(playerPos)    │
│       → Caméra suit le joueur                │
│     → LightingSystem::update()               │
│       → Collecte lumières ECS                │
│       → Envoie au DynamicLightingEffect      │
│     → DialogueSystem::showNPCIndicator()     │
│     → SceneManager::update(deltaTime)        │
│       → Scene::update(deltaTime)             │
│         → AnimationSystem::update()          │
│           → Avance frames d'animation        │
│         → PhysicsSystem::update()            │
│           → Détecte collisions AABB          │
│         → ActivatorSystem::update()          │
│           → Vérifie triggers                 │
│           → Active/désactive zones           │
│         → AudioSystem::update()              │
│           → Joue sons si playOnStart         │
│         → JourneySystem::update()            │
│           → Déplace NPCs sur leur trajet     │
│           → Gère transitions multi-scènes    │
│     → UIManager::update(deltaTime)           │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│   3. RENDER (AFFICHAGE)                      │
│   ────────────────────────────────────────   │
│   Game::onRender()                           │
│     ┌─────────────────────────────────────┐ │
│     │ 3.1. RENDER SCÈNE (vers texture)    │ │
│     │ ──────────────────────────────────  │ │
│     │ PostProcessPipeline::beginSceneRender()│
│     │   → Active RenderTexture            │ │
│     │   → Clear avec backgroundColor      │ │
│     │                                     │ │
│     │ SceneManager::render()              │ │
│     │   → Scene::render()                 │ │
│     │     → RenderSystem::update()        │ │
│     │       → Trie entités par zOrder     │ │
│     │       → GRAPHICS().drawSprite()     │ │
│     │         (dessine vers RenderTexture)│ │
│     └─────────────────────────────────────┘ │
│                                               │
│     ┌─────────────────────────────────────┐ │
│     │ 3.2. POST-PROCESSING                │ │
│     │ ──────────────────────────────────  │ │
│     │ PostProcessPipeline::endSceneRender()│
│     │   → Applique effets séquentiellement:│
│     │                                     │ │
│     │   1. SSAOEffect                     │ │
│     │      → Ambient occlusion            │ │
│     │   2. BloomEffect                    │ │
│     │      → Extract bright → Blur → Add  │ │
│     │   3. ColorGradingEffect             │ │
│     │      → Saturation, contrast         │ │
│     │   4. DynamicLightingEffect          │ │
│     │      → Cycle jour/nuit              │ │
│     │      → Applique lumières ECS        │ │
│     │      → Point/Directional/Spot       │ │
│     │                                     │ │
│     │   → Ping-pong entre textures        │ │
│     │   → Résultat final vers écran       │ │
│     └─────────────────────────────────────┘ │
│                                               │
│     ┌─────────────────────────────────────┐ │
│     │ 3.3. RENDER UI (direct à l'écran)  │ │
│     │ ──────────────────────────────────  │ │
│     │ VIEWPORT().resetView()              │ │
│     │   → Remet vue par défaut            │ │
│     │                                     │ │
│     │ UIManager::render()                 │ │
│     │   → Dessine composants UI           │ │
│     │   → Direct, pas de shader           │ │
│     └─────────────────────────────────────┘ │
│                                               │
│   WINDOW().display()                          │
│     → Affiche frame à l'écran                 │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│         FIN DE FRAME                         │
│   Retour début de loop                       │
└─────────────────────────────────────────────┘
```

## Système de gestion de ressources

### Système à deux niveaux

**Niveau 1 : Définitions réutilisables**
```json
// data/definitions/Sprites.json
{
  "torch": {
    "texture": "textures/torch.png",
    "size": [32, 64],
    "origin": [16, 32]
  }
}
```

**Niveau 2 : Placement dans scènes**
```json
// data/scenes/tavern.json
{
  "entities": [
    {
      "type": "sprite",
      "spriteID": "torch",
      "position": [100, 200],
      "zOrder": 5
    }
  ]
}
```

**Avantages** :
- Réutilisation maximale
- Modification centralisée
- Séparation préoccupations (définition vs placement)
- Optimisation mémoire (une texture chargée une fois)

### Handle-based resource management

```cpp
// Ressources identifiées par handles opaques (u64)
TextureHandle handle = RESOURCES().loadTexture("path/to/texture.png");

// Évite pointeurs invalides
// Permet resource pooling
// Simplifie hot-reloading
```

## Pathfinding multi-échelle

### Niveau 1 : Pathfinding inter-scènes (SceneGraph)

```
SceneGraph :
  ville ──[porte_taverne]──> taverne
         [porte_maison]──> maison_bob

Connexion = {
  fromScene: "ville",
  toScene: "taverne",
  exitPortalPos: Vec2f{500, 300},  // Position du portail dans ville
  entryPortalPos: Vec2f{50, 400}   // Position d'entrée dans taverne
}
```

### Niveau 2 : Pathfinding intra-scène (WaypointGraph)

```
Waypoints dans une scène :
  wp1 ──[main_road]──> wp2 ──[main_road]──> wp3
   └────[shortcut]────────────────────────┘

Waypoint = {
  position: Vec2f{x, y},
  tags: ["main_road", "scenic"]
}

Connection = {
  from: wp1,
  to: wp2,
  tags: ["main_road"]  // Personnalité NPC: préfère certains chemins
}
```

### Voyages multi-scènes (JourneyComponent)

```cpp
// NPC voyage de "ville" à "maison_bob"
JourneySystem::startJourney(npc, "ville", "maison_bob", targetPos);

// Calcule chemin inter-scènes
scenePath = ["ville", "maison_bob"]

// Pour chaque scène intermédiaire:
//   1. Calcule waypoint path local
//   2. NPC suit waypoints
//   3. Atteint portail de sortie
//   4. Transition vers scène suivante
//   5. Apparaît au portail d'entrée
//   6. Répète jusqu'à destination finale
```

## Séparation rendu scène / UI

```
┌────────────────────────────────────┐
│  SCENE RENDERING                   │
│  → Vers RenderTexture              │
│  → Shaders appliqués               │
│  → Post-processing                 │
└────────────────────────────────────┘
         ↓
┌────────────────────────────────────┐
│  POST-PROCESSING PIPELINE          │
│  → SSAO, Bloom, Lighting, etc.     │
│  → Transforme la scène             │
└────────────────────────────────────┘
         ↓
┌────────────────────────────────────┐
│  UI RENDERING                      │
│  → Direct à l'écran                │
│  → PAS de shaders                  │
│  → Par-dessus la scène processée   │
└────────────────────────────────────┘
```

**Pourquoi ?**
- UI toujours nette (pas déformée par CRT effect)
- Performance (UI pas rerendu avec chaque effet)
- Contrôle précis sur ce qui est affecté

## Threading model

**Actuellement : Single-threaded**

```
Main Thread:
  - Input processing
  - Game logic (ECS systems)
  - Rendering
  - Display
```

**Pourquoi ?**
- Simplicité
- Pas de race conditions
- SFML/OpenGL pas thread-safe par défaut
- Performance suffisante pour 2D

**Opportunités futures** :
- Asset loading en background
- Pathfinding asynchrone
- Audio streaming

## Gestion de la mémoire

### Smart pointers utilisés

```cpp
// Ownership unique
std::unique_ptr<Scene> scene;
std::unique_ptr<PostProcessEffect> effect;

// Ownership partagé (rare)
std::shared_ptr<Texture> texture;  // Dans ResourceManager

// Observers (pas d'ownership)
Scene* activeScene;  // Pointeur vers scene dans map
```

### Entités et composants

```cpp
// EntityRegistry possède les entités
std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;

// Entité possède ses composants
std::unordered_map<TypeID, std::unique_ptr<Component>> m_components;
```

## Configuration et extensibilité

### Ajouter un nouveau composant

1. Créer classe dans `Components.hpp`
2. Hériter de `Component`
3. Implémenter `getTypeID()`, `serialize()`, `deserialize()`
4. Utiliser macro `COMPONENT_TYPE_ID(MonComposant)`

### Ajouter un nouveau système

1. Créer classe dans `Systems.hpp`
2. Hériter de `System`
3. Implémenter `update()` et `getRequiredComponents()`
4. Ajouter au constructeur de `Scene`

### Ajouter un nouvel effet

1. Créer classe héritant de `PostProcessEffect`
2. Implémenter `initialize()`, `apply()`, `shutdown()`
3. Créer shader GLSL
4. Ajouter au pipeline dans `Game::onInitialize()`

## Philosophie de design

### Principes clés

1. **Composition over inheritance** : ECS favorise la composition
2. **Data-oriented** : Composants = data, Systems = logic
3. **Separation of concerns** : Chaque système a une responsabilité
4. **Modularity** : Backends interchangeables, effets activables
5. **Performance** : Cache-friendly ECS, batching, culling
6. **Maintainability** : Code clair, bien documenté, patterns standards

### Décisions architecturales importantes

**Pourquoi ECS ?**
- Flexibilité maximale pour composer entités
- Performance (data locality)
- Facilite l'ajout de features

**Pourquoi backend abstraction ?**
- Portabilité (SFML → SDL → Custom)
- Tests facilités (mock backends)
- Isolation de dépendances externes

**Pourquoi JSON pour assets ?**
- Lisible par humains
- Facile à éditer
- Standard bien supporté
- Sérialisation/désérialisation simple

**Pourquoi handle-based resources ?**
- Évite dangling pointers
- Permet hot-reloading
- Resource pooling facile
- Type safety (TextureHandle ≠ SoundHandle)

---

**Prochaine section** : [Système Core](02-CORE.md)
