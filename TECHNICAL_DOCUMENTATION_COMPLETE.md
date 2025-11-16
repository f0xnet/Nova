# NovaEngine - Documentation Technique Exhaustive et Détaillée

**Date:** 16 Novembre 2025  
**Version:** Architecture Complète  
**Niveau de détail:** Very Thorough  
**Statut:** Analyse complète du système ECS, Backend, UI et Core

---

## Table des Matières

1. [Vue d'ensemble architecturale](#vue-densemble-architecturale)
2. [ECS System - Entity Component System](#ecs-system---entity-component-system)
3. [Backend Architecture](#backend-architecture)
4. [UI System](#ui-system)
5. [Core Systems](#core-systems)
6. [Events System](#events-system)
7. [Scene Management](#scene-management)
8. [Pathfinding Systems](#pathfinding-systems)
9. [Resource Management](#resource-management)
10. [Implémentations SFML](#implémentations-sfml)

---

## Vue d'ensemble architecturale

NovaEngine est un moteur de jeu 2D basé sur une architecture **Entity-Component-System (ECS)** moderne. L'architecture est divisée en plusieurs couches :

```
┌─────────────────────────────────────────────┐
│         Application Layer (Game.cpp)        │
│   - Game logic                              │
│   - Scene management                        │
│   - Player controller                       │
└──────────────┬──────────────────────────────┘
               │
┌──────────────┴──────────────────────────────┐
│       ECS + Scene Management Layer          │
│  - Scenes with entities and systems         │
│  - Definition Manager                       │
│  - Scene Graph (multi-scene travel)         │
│  - Waypoint Graph (NPC pathfinding)        │
└──────────────┬──────────────────────────────┘
               │
┌──────────────┴──────────────────────────────┐
│     UI System + Event Dispatcher            │
│  - UIManager with layered rendering         │
│  - Event propagation                        │
│  - UI Components (Button, Text, etc.)      │
└──────────────┬──────────────────────────────┘
               │
┌──────────────┴──────────────────────────────┐
│      Core Systems (Logger, Config)          │
│  - Centralized logging                      │
│  - Configuration management                 │
│  - Resource Manager                         │
└──────────────┬──────────────────────────────┘
               │
┌──────────────┴──────────────────────────────┐
│       Backend Manager (Abstraction Layer)   │
│  - SFML Graphics Backend                    │
│  - SFML Audio Backend                       │
│  - SFML Input Backend                       │
│  - SFML Resource Backend                    │
│  - SFML Font Backend                        │
│  - SFML Viewport Backend                    │
│  - SFML Window Backend                      │
└─────────────────────────────────────────────┘
```

---

## ECS System - Entity Component System

### 1. Component Base Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Component.hpp` (lignes 1-46)

**Responsabilité:** Fournir l'interface de base pour tous les composants ECS

**Design Pattern:** Strategy Pattern (chaque composant implémente sa propre sérialisation)

```cpp
class Component {
public:
    virtual ~Component() = default;
    virtual ComponentTypeID getTypeID() const = 0;
    virtual void serialize(nlohmann::json& json) const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
};

// Helper macro pour implémenter getTypeID()
#define COMPONENT_TYPE_ID(TypeName) \
    ComponentTypeID getTypeID() const override { return #TypeName; }
```

**Caractéristiques clés:**
- Interface virtuelle pure pour tous les composants
- Sérialisation/désérialisation JSON intégrée
- Type identification par string (ComponentTypeID = std::string)
- Macro helper pour réduire le boilerplate

---

### 2. Components Implémentés

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 1-562)

#### 2.1 TransformComponent

**Lignes:** 11-49

**Responsabilité:** Position, rotation, et échelle des entités

```cpp
class TransformComponent : public Component {
public:
    Vec2f position = {0.0f, 0.0f};
    f32 rotation = 0.0f;
    Vec2f scale = {1.0f, 1.0f};
    Vec2f origin = {0.0f, 0.0f};
    
    // COMPONENT_TYPE_ID macro → "TransformComponent"
    // serialize() → JSON: position, rotation, scale, origin
    // deserialize() → load from JSON with null-safety checks
};
```

**Propriétés:**
- `position`: Vec2f - Position en pixels
- `rotation`: f32 - Rotation en degrés
- `scale`: Vec2f - Multiplicateur d'échelle (1.0 = taille naturelle)
- `origin`: Vec2f - Point d'origine pour la rotation/échelle

**Interactions:** Utilisé par tous les systèmes (Render, Physics, Activator, etc.)

---

#### 2.2 SpriteComponent

**Lignes:** 51-103

**Responsabilité:** Rendu de sprites 2D

```cpp
class SpriteComponent : public Component {
public:
    ID textureID;                           // ID ResourceManager
    TextureHandle textureHandle = INVALID_HANDLE;  // Handle rendu
    IntRect textureRect = {0, 0, 0, 0};     // Sous-rectangle (0,0,0,0 = texture complète)
    Vec2f size = {0.0f, 0.0f};              // Taille affichage
    Color tint = Color::White;              // Teinte de couleur
    BlendMode blendMode = BlendMode::Alpha; // Mode mélange
    i32 zOrder = 0;                         // Ordre de rendu
    bool visible = true;                    // Visibilité
};
```

**Interactions principales:**
- `RenderSystem`: Utilise textureHandle et properties pour dessiner
- `Scene::createSpriteEntity()`: Charge depuis définitions
- Supporte plusieurs états de blendMode (Alpha, Add, Multiply, None)

---

#### 2.3 LightComponent

**Lignes:** 105-166

**Responsabilité:** Éclairage dans les scènes

```cpp
class LightComponent : public Component {
public:
    enum class LightType { Point, Directional, Spot };
    
    LightType type = LightType::Point;
    Color color = Color::White;
    f32 radius = 100.0f;           // Pour Point/Spot
    f32 intensity = 1.0f;          // 0.0 à 1.0+
    Vec2f direction = {0, 0};      // Pour Directional/Spot
    f32 angle = 45.0f;             // Pour Spot (degrés)
    bool castShadows = false;       // Support shadows?
    bool enabled = true;
};
```

**Types de lumières:**
- **Point:** Lumière omni-directionnelle depuis un point (radius définit portée)
- **Directional:** Lumière parallèle (sun/moon), utilise direction
- **Spot:** Lumière conique (projecteur), utilise angle et direction

---

#### 2.4 AnimationComponent

**Lignes:** 168-215

**Responsabilité:** Animation par frame de sprites

```cpp
class AnimationComponent : public Component {
public:
    ID animationID;                         // Référence définition
    std::vector<IntRect> frames;            // Rectangles des frames
    f32 frameDuration = 0.1f;               // Durée par frame (secondes)
    f32 currentTime = 0.0f;                 // Temps courant
    u32 currentFrame = 0;                   // Index frame actuelle
    bool loop = true;                       // Boucle animation?
    bool playing = true;                    // Lecture en cours?
};
```

**Logique du système:**
- `AnimationSystem::update()` avance `currentTime` par deltaTime
- Quand `currentTime >= frameDuration`, avance frame et reset temps
- Si `currentFrame >= frames.size()` et loop=true → reset à 0
- Sinon → stop l'animation

**Intégration:**
- `AnimationSystem` met à jour textureRect du SpriteComponent associé
- Définitions chargées via `DefinitionManager`

---

#### 2.5 ColliderComponent

**Lignes:** 217-260

**Responsabilité:** Collision physique

```cpp
class ColliderComponent : public Component {
public:
    enum class ColliderType { Box, Circle };
    
    ColliderType type = ColliderType::Box;
    Vec2f size = {0.0f, 0.0f};     // Pour Box
    f32 radius = 0.0f;             // Pour Circle
    Vec2f offset = {0.0f, 0.0f};   // Offset depuis position
    bool isTrigger = false;        // Trigger (pas de physique) ou solide?
    bool enabled = true;           // Collisions actives?
};
```

**PhysicsSystem implémentation:**
- AABB collision detection pour Box-Box uniquement
- Calcule bounds = `transform->position + collider->offset`
- Teste intersection avec autres colliders
- Actuellement: logging seulement (pas de réponse physics)

---

#### 2.6 AudioComponent

**Lignes:** 262-296

**Responsabilité:** Lecture son/musique

```cpp
class AudioComponent : public Component {
public:
    ID soundID;                    // ID ResourceManager
    SoundHandle soundHandle = INVALID_HANDLE;
    bool playOnStart = false;       // Lancer au démarrage?
    bool loop = false;              // Boucle son?
    f32 volume = 100.0f;            // 0-100
    f32 pitch = 1.0f;              // 0.5-2.0 typiquement
    bool playing = false;           // État lecture
};
```

**AudioSystem::update():**
- Sur première frame: si `playOnStart && !playing`, lance le son
- Appelle `AUDIO().playSound(handle, volume, pitch, loop)`

---

#### 2.7 ActivatorComponent

**Lignes:** 298-400

**Responsabilité:** Zones d'activation (triggers)

```cpp
class ActivatorComponent : public Component {
public:
    enum class ActivatorType { Proximity, Manual, Automatic };
    enum class ActivatorShape { Box, Circle };
    
    ActivatorType type = ActivatorType::Proximity;
    ActivatorShape shape = ActivatorShape::Box;
    Vec2f size = {100.0f, 100.0f};     // Pour Box
    f32 radius = 50.0f;                // Pour Circle
    Vec2f offset = {0.0f, 0.0f};
    
    bool isActive = false;              // État courant
    bool canReactivate = true;          // Peut réactiver?
    f32 cooldownTime = 0.0f;            // Temps avant réactivation
    f32 currentCooldown = 0.0f;         // Cooldown courant
    
    std::string targetTag = "player";   // Quelle entité active?
    std::string actionID;               // ID action (logique jeu)
    
    // Debug
    bool showDebugZone = false;         // Visualiser zone?
    Color debugColor = Color{0, 255, 0, 100};
    
    // Callbacks
    std::string onActivateEvent;        // Event à feu
    std::string onDeactivateEvent;
};
```

**Types d'activations:**
- **Proximity:** S'active une fois à l'entrée, se désactive à la sortie
- **Automatic:** Reste actif tant que entité dans zone
- **Manual:** Nécessite input utilisateur (E.g., interaction explicite)

**ActivatorSystem logic:**
- Itère tous activators + entités avec TagComponent
- Teste collision (Box/Circle) entre zones et entités
- Gère cooldowns et événements

---

#### 2.8 TagComponent

**Lignes:** 402-425

**Responsabilité:** Identification entité

```cpp
class TagComponent : public Component {
public:
    std::string tag = "default";  // "player", "npc", "enemy", etc.
};
```

**Utilisation:** ActivatorSystem filtre par tag (targetTag)

---

#### 2.9 SceneTransitionComponent

**Lignes:** 427-464

**Responsabilité:** Gestion transitions inter-scène

```cpp
class SceneTransitionComponent : public Component {
public:
    std::string targetScene;        // Scène destination
    Vec2f targetPosition;           // Position dans scène destination
    bool isTransitioning = false;   // Transition en cours?
};
```

**Usage:** JourneySystem l'utilise pour marquer entités à transférer

---

#### 2.10 JourneyComponent

**Lignes:** 466-559

**Responsabilité:** Voyages multi-scène pour NPCs

```cpp
class JourneyComponent : public Component {
public:
    // Multi-scène
    std::vector<std::string> scenePath;    // Chemin scènes
    int currentSceneIndex = 0;
    
    Vec2f currentDestination;              // Destination scène courante
    bool reachedCurrentDestination = false;
    
    // Pathfinding waypoints dans scène courante
    std::vector<Vec2f> localWaypointPath;  // Chemin waypoints
    int currentLocalWaypointIndex = 0;
    
    // Personnalité
    std::vector<std::string> preferredPathTags; // Chemins préférés
    
    // État voyage
    bool isOnJourney = false;
    std::string finalDestinationScene;
    Vec2f finalDestinationPos;
};
```

**Logique complète:**
1. NPC reçoit `startJourney(currentScene, targetScene, targetPos)`
2. `JourneySystem::findPath()` utilise `SceneGraph` pour calculer route
3. En chaque scène: calcule waypoints locaux via `WaypointGraph`
4. Suit waypoints jusqu'à portail vers prochaine scène
5. À transition: `JourneyComponent` mise à jour, portail d'entrée nouvelle scène
6. Finalisation à destination

---

### 3. Entity Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Entity.hpp` (lignes 1-144)

**Responsabilité:** Conteneur composants

```cpp
class Entity {
private:
    u64 m_id;
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;
    
public:
    Entity(u64 id);
    
    u64 getID() const;
    
    // Ajouter composant (transfer ownership)
    template<typename T>
    T* addComponent(std::unique_ptr<T> component);
    
    // Récupérer composant
    template<typename T>
    T* getComponent();
    
    template<typename T>
    const T* getComponent() const;
    
    // Vérifier présence
    template<typename T>
    bool hasComponent() const;
    
    bool hasComponent(const ComponentTypeID& typeID) const;
    
    // Supprimer composant
    template<typename T>
    void removeComponent();
    
    // Lister tous types composants
    std::vector<ComponentTypeID> getComponentTypes() const;
};
```

**Utilisation pattern:**
```cpp
Entity* entity = registry.createEntity();
entity->addComponent(std::make_unique<TransformComponent>());
entity->addComponent(std::make_unique<SpriteComponent>());

// Accès
auto* transform = entity->getComponent<TransformComponent>();
if (entity->hasComponent<AnimationComponent>()) {
    auto* anim = entity->getComponent<AnimationComponent>();
}
```

---

### 4. EntityRegistry Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/EntityRegistry.hpp` (lignes 1-133)

**Responsabilité:** Gestion cycle de vie entités + requêtes

```cpp
class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    u64 m_nextID = 1;
    
public:
    Entity* createEntity();
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

**Utilisé par:**
- `Scene::update()` → exécute tous systèmes
- `System::update()` → requête entités par composants
- `JourneySystem` → trouve entités avec JourneyComponent

---

### 5. System Base Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/System.hpp` (lignes 1-46)

**Responsabilité:** Interface pour systèmes ECS

```cpp
class System {
public:
    virtual ~System() = default;
    
    // Mise à jour, appelée chaque frame
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;
    
    // Composants requis pour ce système
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;
    
    virtual void onInit() {}
    virtual void onShutdown() {}
};
```

---

### 6. Systems Implémentés

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 1-696)

#### 6.1 RenderSystem

**Lignes:** 13-61

```cpp
class RenderSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // 1. Requête entités: Transform + Sprite
        auto entities = registry.getEntitiesWith({
            "TransformComponent", "SpriteComponent"
        });
        
        // 2. Tri par zOrder (lower = behind)
        std::sort(entities.begin(), entities.end(), 
                  [](Entity* a, Entity* b) {
                      auto* spriteA = a->getComponent<SpriteComponent>();
                      auto* spriteB = b->getComponent<SpriteComponent>();
                      return spriteA->zOrder < spriteB->zOrder;
                  });
        
        // 3. Draw chaque sprite
        for (Entity* entity : entities) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* sprite = entity->getComponent<SpriteComponent>();
            
            // Vérifier visibilité + texture valide
            if (!sprite->visible || sprite->textureHandle == INVALID_HANDLE)
                continue;
            
            // Préparer données
            SpriteData data;
            data.texture = sprite->textureHandle;
            data.position = transform->position;
            data.size = sprite->size;
            data.rotation = transform->rotation;
            data.scale = transform->scale;
            data.origin = transform->origin;
            data.textureRect = sprite->textureRect;
            data.color = sprite->tint;
            data.blendMode = sprite->blendMode;
            
            // Draw
            GRAPHICS().drawSprite(data);
        }
    }
    
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SpriteComponent"};
    }
};
```

**Pipeline rendu:**
1. Tri Z-Order
2. Vérif visibilité
3. Prépare SpriteData
4. Appelle GRAPHICS().drawSprite()

---

#### 6.2 AnimationSystem

**Lignes:** 63-112

**Logique:**
- Avance `currentTime` par deltaTime
- Quand `currentTime >= frameDuration`: avance frame
- Boucle si `loop=true`, sinon pause animation
- Met à jour `textureRect` du sprite

---

#### 6.3 LightSystem

**Lignes:** 114-156

**Logique:**
- Itère entités Transform + Light
- Pour LightType::Point: dessine cercle semi-transparent
- Radius = portée lumière
- Intensity × 50 = alpha (0-255)

**Note:** Directional/Spot nécessitent shaders (non implémentés)

---

#### 6.4 AudioSystem

**Lignes:** 158-185

**Logique:**
- Sur première update: si `playOnStart && !playing`
- Appelle `AUDIO().playSound(handle, volume, pitch, loop)`
- Marque `playing = true`

---

#### 6.5 PhysicsSystem

**Lignes:** 187-245

**Logique AABB:**
```cpp
// 1. Requête: Transform + Collider
// 2. Comparaison N² entités
// 3. Pour chaque paire:
//    - Calcule AABB bounds
//    - Teste intersection AABB
//    - Log collision détectée
//    - Note: pas de réponse physics actuellement
```

---

#### 6.6 ActivatorSystem

**Lignes:** 247-430

**Logique complète:**

```cpp
class ActivatorSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // 1. Requête activators et entités taggées
        auto activators = registry.getEntitiesWith({
            "TransformComponent", "ActivatorComponent"
        });
        auto potentialTriggers = registry.getEntitiesWith({
            "TransformComponent", "TagComponent"
        });
        
        // 2. Mettre à jour cooldowns
        for (Entity* activatorEntity : activators) {
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();
            if (activator->currentCooldown > 0.0f) {
                activator->currentCooldown -= deltaTime;
            }
        }
        
        // 3. Vérifier activations
        for (Entity* activatorEntity : activators) {
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();
            
            if (activator->currentCooldown > 0.0f) continue;
            
            // Calculer zone activation
            Vec2f activatorPos = activatorTransform->position + activator->offset;
            bool wasActive = activator->isActive;
            bool entityInZone = false;
            
            // Tester tous entités taggées
            for (Entity* triggerEntity : potentialTriggers) {
                auto* tag = triggerEntity->getComponent<TagComponent>();
                if (tag->tag != activator->targetTag) continue;
                
                auto* triggerTransform = triggerEntity->getComponent<TransformComponent>();
                Vec2f triggerPos = triggerTransform->position;
                
                // Test collision Zone-Entité
                bool inZone = false;
                
                if (activator->shape == ActivatorComponent::ActivatorShape::Box) {
                    Rect activatorRect{
                        activatorPos.x - activator->size.x * 0.5f,
                        activatorPos.y - activator->size.y * 0.5f,
                        activator->size.x,
                        activator->size.y
                    };
                    inZone = activatorRect.contains(triggerPos);
                } else if (activator->shape == ActivatorComponent::ActivatorShape::Circle) {
                    f32 dx = triggerPos.x - activatorPos.x;
                    f32 dy = triggerPos.y - activatorPos.y;
                    f32 distSquared = dx*dx + dy*dy;
                    inZone = distSquared <= (activator->radius * activator->radius);
                }
                
                if (inZone) {
                    entityInZone = true;
                    break;
                }
            }
            
            // 4. Gérer activation selon type
            if (activator->type == ActivatorComponent::ActivatorType::Proximity) {
                if (entityInZone && !wasActive) {
                    activateActivator(activatorEntity, activator);
                } else if (!entityInZone && wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            } else if (activator->type == ActivatorComponent::ActivatorType::Automatic) {
                if (entityInZone) {
                    if (!wasActive) activateActivator(activatorEntity, activator);
                } else if (wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            } else if (activator->type == ActivatorComponent::ActivatorType::Manual) {
                if (entityInZone && !wasActive) {
                    LOG_DEBUG("Entity {} can be manually activated (press E)",
                             activatorEntity->getID());
                }
            }
            
            // 5. Debug visualization
            if (activator->showDebugZone) {
                // Dessiner zone debug
            }
        }
    }
    
private:
    void activateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = true;
        LOG_INFO("Activator {} activated! Action: '{}'",
                entity->getID(), activator->actionID);
        
        // Fire event
        if (!activator->onActivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onActivateEvent);
        }
        
        // Gérer cooldown
        if (!activator->canReactivate) {
            activator->currentCooldown = -1.0f;  // Permanent
        } else if (activator->cooldownTime > 0.0f) {
            activator->currentCooldown = activator->cooldownTime;
        }
    }
    
    void deactivateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = false;
        LOG_DEBUG("Activator {} deactivated", entity->getID());
        
        if (!activator->onDeactivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onDeactivateEvent);
        }
    }
};
```

---

#### 6.7 JourneySystem

**Lignes:** 432-694

**Logique multi-scène NPC travel:**

```cpp
class JourneySystem : public System {
private:
    SceneGraph* m_sceneGraph;
    
    struct PendingTransfer {
        u64 entityID;
        std::string fromScene, toScene;
        Vec2f targetPosition;
        int nextSceneIndex;
        std::vector<std::string> remainingPath;
        Vec2f finalDestination;
        std::string finalScene;
    };
    
    std::vector<PendingTransfer> m_pendingTransfers;
    
public:
    explicit JourneySystem(SceneGraph* sceneGraph)
        : m_sceneGraph(sceneGraph) {}
    
    void update(float deltaTime, EntityRegistry& registry) override {
        // Récupère entités avec Journey
        auto travelers = registry.getEntitiesWith({
            "TransformComponent",
            "SceneTransitionComponent",
            "JourneyComponent"
        });
        
        for (Entity* entity : travelers) {
            auto* journey = entity->getComponent<JourneyComponent>();
            if (!journey->isOnJourney) continue;
            
            auto* transform = entity->getComponent<TransformComponent>();
            
            // Suivre waypoints locaux si disponibles
            if (!journey->localWaypointPath.empty() &&
                journey->currentLocalWaypointIndex < journey->localWaypointPath.size()) {
                
                Vec2f currentWaypoint = journey->localWaypointPath[
                    journey->currentLocalWaypointIndex
                ];
                Vec2f toWaypoint = currentWaypoint - transform->position;
                f32 distanceSquared = toWaypoint.x * toWaypoint.x + 
                                     toWaypoint.y * toWaypoint.y;
                
                // Waypoint atteint (<5 pixels)?
                if (distanceSquared < 25.0f) {
                    journey->currentLocalWaypointIndex++;
                    
                    if (journey->currentLocalWaypointIndex >= 
                        journey->localWaypointPath.size()) {
                        journey->reachedCurrentDestination = true;
                    }
                }
            } else {
                // Pas de waypoints, navigation directe
                Vec2f toDestination = journey->currentDestination - 
                                     transform->position;
                f32 distanceSquared = toDestination.x * toDestination.x + 
                                     toDestination.y * toDestination.y;
                
                if (distanceSquared < 25.0f) {
                    journey->reachedCurrentDestination = true;
                }
            }
            
            // Destination atteinte: gérer transition
            if (journey->reachedCurrentDestination) {
                if (journey->currentSceneIndex + 1 < journey->scenePath.size()) {
                    // Prochaine scène existe
                    std::string currentScene = journey->scenePath[
                        journey->currentSceneIndex
                    ];
                    std::string nextScene = journey->scenePath[
                        journey->currentSceneIndex + 1
                    ];
                    
                    const auto* connection = m_sceneGraph->getConnection(
                        currentScene, nextScene
                    );
                    
                    if (connection) {
                        // Créer transfert
                        PendingTransfer transfer;
                        transfer.entityID = entity->getID();
                        transfer.fromScene = currentScene;
                        transfer.toScene = nextScene;
                        transfer.targetPosition = connection->entryPortalPos;
                        transfer.nextSceneIndex = 
                            journey->currentSceneIndex + 1;
                        
                        m_pendingTransfers.push_back(transfer);
                        
                        // Marquer transition
                        auto* transition = entity->getComponent<
                            SceneTransitionComponent>();
                        transition->targetScene = nextScene;
                        transition->targetPosition = 
                            connection->entryPortalPos;
                        transition->isTransitioning = true;
                    }
                } else {
                    // Fin voyage!
                    journey->isOnJourney = false;
                    journey->scenePath.clear();
                }
            }
        }
    }
    
    // API publique
    bool startJourney(Entity* entity,
                     const std::string& currentScene,
                     const std::string& targetScene,
                     const Vec2f& targetPosition) {
        auto* journey = entity->getComponent<JourneyComponent>();
        if (!journey || !m_sceneGraph) return false;
        
        // Trouver chemin via SceneGraph
        journey->scenePath = m_sceneGraph->findPath(
            currentScene, targetScene
        );
        
        if (journey->scenePath.empty()) return false;
        
        journey->currentSceneIndex = 0;
        journey->isOnJourney = true;
        journey->finalDestinationScene = targetScene;
        journey->finalDestinationPos = targetPosition;
        journey->reachedCurrentDestination = false;
        
        // Définir première destination
        if (journey->scenePath.size() > 1) {
            const auto* connection = m_sceneGraph->getConnection(
                journey->scenePath[0],
                journey->scenePath[1]
            );
            if (connection) {
                journey->currentDestination = connection->exitPortalPos;
            }
        } else {
            journey->currentDestination = targetPosition;
        }
        
        return true;
    }
    
    void calculateLocalWaypointPath(Entity* entity, Scene* currentScene);
    
    void cancelJourney(Entity* entity) {
        auto* journey = entity->getComponent<JourneyComponent>();
        if (journey) {
            journey->isOnJourney = false;
            journey->scenePath.clear();
            journey->localWaypointPath.clear();
        }
    }
};
```

**Inline implementation:**
```cpp
inline void JourneySystem::calculateLocalWaypointPath(
    Entity* entity, Scene* currentScene) {
    
    if (!entity || !currentScene) return;
    
    auto* journey = entity->getComponent<JourneyComponent>();
    auto* transform = entity->getComponent<TransformComponent>();
    if (!journey || !transform || !journey->isOnJourney) return;
    
    // Utiliser WaypointGraph de la scène
    auto& waypointGraph = currentScene->getWaypointGraph();
    
    if (waypointGraph.isEmpty()) {
        journey->localWaypointPath.clear();
        journey->currentLocalWaypointIndex = 0;
        return;
    }
    
    // Calculer chemin waypoints
    journey->localWaypointPath = waypointGraph.findPath(
        transform->position,
        journey->currentDestination,
        journey->preferredPathTags
    );
    
    journey->currentLocalWaypointIndex = 0;
}
```

---

## Backend Architecture

### 1. Backend Types

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/Core/BackendTypes.hpp` (lignes 1-186)

**Typedef Primitifs:**
```cpp
using i8 = int8_t;    using i16 = int16_t;    using i32 = int32_t;    using i64 = int64_t;
using u8 = uint8_t;   using u16 = uint16_t;   using u32 = uint32_t;   using u64 = uint64_t;
using f32 = float;     using f64 = double;
using String = std::string;

// Resource Handles
using TextureHandle = u64;  using FontHandle = u64;  using SoundHandle = u64;
using MusicHandle = u64;    using ShaderHandle = u64;
constexpr u64 INVALID_HANDLE = 0;
```

**Enums:**
```cpp
enum class BackendType { SFML, SDL, Custom };
enum class KeyCode { A-Z, Num0-9, Escape, LControl, Space, Enter, Left, Right, Up, Down, ... };
enum class MouseButton { Left, Right, Middle };
enum class InputEventType { Closed, Resized, KeyPressed, KeyReleased, 
                           MouseButtonPressed, MouseButtonReleased, MouseMoved, TextEntered };
enum class BlendMode { Alpha, Add, Multiply, None };
enum class TextStyle { Regular=0, Bold=1, Italic=2, Underlined=4, StrikeThrough=8 };
enum class SoundStatus { Stopped, Paused, Playing };
```

**Structures géométriques:**
```cpp
struct Vec2f {
    f32 x, y;
    // Opérateurs: +, -, *, /, +=, -=, *=, /=
};

struct Vec2i { i32 x, y; };
struct Vec2u { u32 x, y; };

struct Rect {
    f32 left, top, width, height;
    bool contains(f32 x, f32 y) const;
    bool contains(const Vec2f& p) const;
};

struct IntRect { i32 left, top, width, height; };

struct Color {
    u8 r, g, b, a;
    static const Color Black, White, Red, Green, Blue, Yellow, Transparent;
    operator== / operator!=
};
```

**Structures rendu:**
```cpp
struct SpriteData {
    TextureHandle texture;
    Vec2f position, size;
    f32 rotation;
    Vec2f scale, origin;
    IntRect textureRect;
    Color color;
    BlendMode blendMode;
};

struct RectData {
    Vec2f position, size;
    Color fillColor, outlineColor;
    f32 outlineThickness, rotation;
    Vec2f origin;
};

struct TextData {
    String text;
    FontHandle font;
    u32 characterSize;
    Color fillColor, outlineColor;
    f32 outlineThickness;
    TextStyle style;
    Vec2f position;
    f32 rotation;
    Vec2f scale, origin;
    BlendMode blendMode;
};

struct ViewportData {
    Rect viewport;
    Vec2f center, size;
    f32 rotation;
};
```

**Input:**
```cpp
struct InputEvent {
    InputEventType type;
    union {
        struct { u32 width, height; } size;              // Resized
        struct { u32 unicode; } text;                    // TextEntered
        struct { KeyCode code; bool alt, control, 
                 shift, system; } key;                   // Key*
        struct { MouseButton button; i32 x, y; } mouseButton;  // MouseButton*
        struct { i32 x, y; } mouseMove;
    };
};
```

---

### 2. Backend Manager (Singleton Facade)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/BackendManager.hpp` (lignes 1-67)

**Responsabilité:** Centraliser accès aux backends via Singleton + Facade patterns

```cpp
class BackendManager {
public:
    static BackendManager& get();  // Singleton
    
    bool initialize(BackendType type = BackendType::SFML,
                   u32 windowWidth = 800,
                   u32 windowHeight = 600,
                   const String& windowTitle = "NovaEngine",
                   bool fullscreen = false);
    void shutdown();
    bool isInitialized() const;
    BackendType getCurrentBackendType() const;
    void reconnectWindow();
    
    // Accesseurs interfaces
    IWindowBackend& window();
    IInputBackend& input();
    IGraphicsBackend& graphics();
    IResourceBackend& resources();
    IAudioBackend& audio();
    IFontBackend& fonts();
    IViewportBackend& viewport();
    
    // Const versions
    const IWindowBackend& window() const;
    // ...
    
private:
    BackendManager();
    ~BackendManager();
    bool createBackends(BackendType type);
    void destroyBackends();
    
    bool m_initialized;
    BackendType m_currentBackend;
    std::unique_ptr<IWindowBackend> m_window;
    std::unique_ptr<IInputBackend> m_input;
    std::unique_ptr<IGraphicsBackend> m_graphics;
    std::unique_ptr<IResourceBackend> m_resources;
    std::unique_ptr<IAudioBackend> m_audio;
    std::unique_ptr<IFontBackend> m_fonts;
    std::unique_ptr<IViewportBackend> m_viewport;
};

// Macros pour accès rapide
#define BACKEND()   NovaEngine::BackendManager::get()
#define WINDOW()    NovaEngine::BackendManager::get().window()
#define INPUT()     NovaEngine::BackendManager::get().input()
#define GRAPHICS()  NovaEngine::BackendManager::get().graphics()
#define RESOURCES() NovaEngine::BackendManager::get().resources()
#define AUDIO()     NovaEngine::BackendManager::get().audio()
#define FONTS()     NovaEngine::BackendManager::get().fonts()
#define VIEWPORT()  NovaEngine::BackendManager::get().viewport()
```

---

### 3. Backend Interfaces

Tous dans `/home/user/Nova/sdk/include/NovaEngine/Backend/Interfaces/`

#### 3.1 IWindowBackend

```cpp
class IWindowBackend {
public:
    virtual ~IWindowBackend() = default;
    
    virtual bool create(u32 width, u32 height, 
                       const String& title, bool fullscreen) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    
    virtual void clear(const Color& color) = 0;
    virtual void display() = 0;
    
    virtual void setTitle(const String& title) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual void setFramerateLimit(u32 limit) = 0;
    
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;
};
```

#### 3.2 IGraphicsBackend

```cpp
class IGraphicsBackend {
public:
    virtual bool initialize(void* windowHandle) = 0;
    virtual void shutdown() = 0;
    
    virtual void drawSprite(const SpriteData& sprite) = 0;
    virtual void drawRect(const RectData& rect) = 0;
    virtual void drawText(const TextData& text) = 0;
    virtual void drawLine(const Vec2f& from, const Vec2f& to,
                         f32 thickness, const Color& color) = 0;
};
```

#### 3.3 IInputBackend

```cpp
class IInputBackend {
public:
    virtual bool pollEvent(InputEvent& event) = 0;
    
    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual Vec2i getMousePosition() const = 0;
};
```

#### 3.4 IResourceBackend

```cpp
class IResourceBackend {
public:
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;
    virtual TextureHandle getTextureHandle(const String& path) const = 0;
    
    virtual void clearCache() = 0;
};
```

#### 3.5 IAudioBackend

```cpp
class IAudioBackend {
public:
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual SoundHandle loadSound(const String& path) = 0;
    virtual void playSound(SoundHandle handle, f32 volume = 100.0f,
                          f32 pitch = 1.0f, bool loop = false) = 0;
    virtual void stopSound(SoundHandle handle) = 0;
    virtual void stopAllSounds() = 0;
    
    virtual void playMusic(const String& path, bool loop = true) = 0;
    virtual void stopMusic() = 0;
};
```

#### 3.6 IFontBackend

```cpp
class IFontBackend {
public:
    virtual FontHandle loadFont(const String& path) = 0;
    virtual TextMetrics measureText(const TextData& text) = 0;
};
```

#### 3.7 IViewportBackend

```cpp
class IViewportBackend {
public:
    virtual void setViewCenter(const Vec2f& center) = 0;
    virtual void setViewSize(const Vec2f& size) = 0;
    virtual Vec2f getViewCenter() const = 0;
    virtual Vec2f getViewSize() const = 0;
    virtual void setViewRotation(f32 rotation) = 0;
};
```

---

### 4. SFML Backend Implementations

**Chemin:** `/home/user/Nova/client/src/Backend/SFML/`

#### 4.1 Architecture générale

Chaque backend SFML:
- Hérite interface virtuelle (IWindowBackend, IGraphicsBackend, etc.)
- Encapsule objet SFML (sf::RenderWindow, sf::Sound, etc.)
- Implémente méthodes interface

#### 4.2 SFMLWindowBackend

Gère sf::RenderWindow:
- `create()` → initialise sf::RenderWindow
- `clear()` / `display()` → appelle SFML
- Propriétés: width, height, VSync, FPS limit

#### 4.3 SFMLGraphicsBackend

Gère rendu:
- `drawSprite()` → crée sf::Sprite, applique propriétés, dessine
- `drawRect()` → sf::RectangleShape
- `drawText()` → sf::Text

#### 4.4 SFMLInputBackend

Gère input:
- `pollEvent()` → sf::Event, convertit en InputEvent
- `isKeyPressed()` / `isMouseButtonPressed()` → sf::Keyboard, sf::Mouse
- `getMousePosition()` → sf::Mouse::getPosition()

#### 4.5 SFMLResourceBackend

Gère ressources:
- Cache textures: std::unordered_map<String, sf::Texture>
- `loadTexture()` → charge et cache sf::Texture, retourne handle
- Handle = index ou pointeur converti en u64

#### 4.6 SFMLAudioBackend

Gère audio:
- SoundBuffer cache + Sound instances
- Music joué via sf::Music (1 seul à la fois)
- `playSound()` → crée sf::Sound, définit volume/pitch, joue
- `stopAllSounds()` → arrête tous sounds

---

## UI System

### 1. UIComponent Base Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIComponent.hpp` (lignes 1-62)

```cpp
class UIComponent : public EventHandler {
protected:
    ID m_id;
    Vec2f m_position;
    Vec2f m_size;
    bool m_visible;
    bool m_active;
    ID m_groupID;
    ID m_uiID;
    i32 m_layer;
    std::string m_effect;
    std::string m_description;
    
public:
    UIComponent();
    virtual ~UIComponent();
    
    // Mise à jour et rendu
    virtual void update(f32 deltaTime);
    virtual void render() const = 0;  // Pure virtual
    virtual void onEvent(const Event& event) override = 0;  // Pure virtual
    
    // Setters
    void setPosition(const Vec2f& pos);
    virtual void setSize(const Vec2f& size);
    void setVisible(bool visible);
    void setActive(bool active);
    void setID(const ID& id);
    void setGroupID(const ID& groupID);
    void setLayer(i32 layer);
    void setEffect(const std::string& effect);
    void setDescription(const std::string& description);
    void setUIID(const ID& uiID);
    
    // Getters
    const ID& getID() const;
    const ID& getGroupID() const;
    const ID& getUIID() const;
    i32 getLayer() const;
    const std::string& getEffect() const;
    const std::string& getDescription() const;
    bool isVisible() const;
    bool isActive() const;
    Vec2f getPosition() const { return m_position; }
    Vec2f getSize() const { return m_size; }
    
    virtual Rect getBounds() const = 0;  // Pure virtual
};
```

**Patterns utilisés:**
- Template Method: `update()` virtuelle
- Strategy: `render()` et `onEvent()` implémentées par sous-classes
- Composition: GroupID/UIID/Layer pour hiérarchie

---

### 2. UIManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIManager.hpp` (lignes 1-65)

```cpp
class UIManager {
public:
    using ActionCallback = std::function<void(const std::string& action,
                                             const std::string& value,
                                             const ID& componentID)>;
    
    UIManager();
    ~UIManager();
    
    // Gestion composants
    void addComponent(const std::shared_ptr<UIComponent>& component);
    void removeComponent(const ID& id);
    void removeUI(const ID& uiID);
    void removeGroup(const ID& groupID);
    
    // Gestion état
    void setGroupActive(const ID& groupID, bool active);
    void setUIActive(const ID& uiID, bool active);
    void switchToGroup(const ID& uiID, const ID& newGroupID);
    void setLayerActive(i32 layer, bool active);
    void clear();
    
    // Cycle principal
    void update(float deltaTime);
    void render() const;
    void dispatchEvent(const Event& event);
    
    // Actions
    void setActionCallback(ActionCallback callback);
    void handleAction(const std::string& action,
                     const std::string& value,
                     const ID& componentID);
    
    // Requêtes
    std::shared_ptr<UIComponent> getComponent(const ID& id);
    std::vector<std::shared_ptr<UIComponent>> getGroup(const ID& groupID);
    std::vector<std::shared_ptr<UIComponent>> getUI(const ID& uiID);
    i32 getMaxLayers() const;
    
private:
    struct ComponentInfo {
        std::shared_ptr<UIComponent> component;
        ID uiID;
        ID groupID;
        i32 layer;
    };
    
    std::unordered_map<ID, ComponentInfo> m_components;
    ActionCallback m_actionCallback;
    
    // Render cache
    mutable std::vector<std::pair<i32, std::shared_ptr<UIComponent>>>
        m_renderCache;
    mutable bool m_renderCacheDirty;
    mutable i32 m_maxLayers;
    
    void invalidateRenderCache();
    void updateRenderCache() const;
};
```

**Implémentation clés** (client/src/UI/UIManager.cpp):

```cpp
UIManager::UIManager() : m_renderCacheDirty(true), m_maxLayers(0) {
    LOG_INFO("UIManager created with backend support");
}

void UIManager::addComponent(const std::shared_ptr<UIComponent>& component) {
    if (!component) {
        LOG_WARN("Trying to add null UIComponent");
        return;
    }
    
    const ID& id = component->getID();
    if (id.empty()) {
        LOG_WARN("Trying to add UIComponent with empty ID");
        return;
    }
    
    ComponentInfo info;
    info.component = component;
    info.uiID = component->getUIID();
    info.groupID = component->getGroupID();
    info.layer = component->getLayer();
    
    m_components[id] = info;
    
    if (info.layer > m_maxLayers) {
        m_maxLayers = info.layer;
    }
    
    invalidateRenderCache();
    LOG_DEBUG("UIComponent '{}' added to UI '{}', group '{}', layer {}",
             id, info.uiID, info.groupID, info.layer);
}

void UIManager::update(float deltaTime) {
    // Update tous composants actifs
    for (auto& [id, info] : m_components) {
        if (info.component && info.component->isActive()) {
            info.component->update(deltaTime);
        }
    }
}

void UIManager::render() const {
    updateRenderCache();
    
    // Rendu par layer (sorté)
    for (const auto& [layer, component] : m_renderCache) {
        if (component && component->isVisible()) {
            component->render();
        }
    }
}

void UIManager::dispatchEvent(const Event& event) {
    for (auto& [id, info] : m_components) {
        if (info.component && info.component->isActive()) {
            info.component->onEvent(event);
        }
    }
}
```

---

### 3. UI Component Implementations

**Chemin:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/`

**Composants disponibles:**
1. **Button** - Bouton interactif
2. **Text** - Texte statique/dynamique
3. **Image** - Affichage image/sprite
4. **Panel** - Conteneur rectangle
5. **Slider** - Curseur horizontal/vertical
6. **TextInput** - Champ texte éditable
7. **Animation** - Animation UI

**Structure commune:**
```cpp
class XXXComponent : public UIComponent {
public:
    void update(f32 deltaTime) override;
    void render() const override;
    void onEvent(const Event& event) override;
    Rect getBounds() const override;
    
private:
    // Propriétés spécifiques XXX
};
```

---

## Core Systems

### 1. Application Base Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Core/Application.hpp` (lignes 1-190)

```cpp
class Application {
public:
    struct Config {
        String windowTitle = "NovaEngine Application";
        u32 windowWidth = 1920;
        u32 windowHeight = 1080;
        bool fullscreen = false;
        u32 frameRateLimit = 60;
        bool vSync = true;
        Color clearColor = Color::Black;
        String configPath = "";
    };
    
protected:
    Config m_config;
    float m_deltaTime;
    bool m_initialized;
    f32 m_lastTime;
    
public:
    Application();
    explicit Application(const Config& config);
    virtual ~Application();
    
    int run() {
        // 1. Initialize engine
        if (!initializeEngine()) {
            LOG_FATAL("Failed to initialize NovaEngine");
            return -1;
        }
        
        // 2. Call application's onInitialize
        if (!onInitialize()) {
            LOG_FATAL("Failed to initialize application");
            return -1;
        }
        
        m_initialized = true;
        
        // 3. Main loop
        runMainLoop();
        
        // 4. Cleanup
        onShutdown();
        shutdownEngine();
        
        return 0;
    }
    
    void quit();
    float getDeltaTime() const;
    const Config& getConfig() const;
    bool isInitialized() const;
    
protected:
    // Subclasses override these
    virtual bool onInitialize() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onEvent(const Event& event) {}
    virtual void onShutdown() {}
    
private:
    bool initializeEngine() {
        // BackendManager::initialize(...)
    }
    
    void runMainLoop() {
        while (WINDOW().isOpen()) {
            // Calculate deltaTime
            // Process events
            onUpdate(m_deltaTime)
            // Render
            WINDOW().clear(m_config.clearColor);
            onRender();
            WINDOW().display();
        }
    }
    
    void processEvents() {
        InputEvent inputEvent;
        while (INPUT().pollEvent(inputEvent)) {
            if (inputEvent.type == InputEventType::Closed) {
                quit();
            }
            if (inputEvent.type == InputEventType::KeyPressed &&
                inputEvent.key.code == KeyCode::Escape) {
                quit();
            }
            
            Event novaEvent(inputEvent);
            onEvent(novaEvent);
        }
    }
    
    void shutdownEngine() {
        BACKEND().shutdown();
    }
};
```

**Utilisation (Game class):**
```cpp
class Game : public Application {
private:
    NovaEngine::SceneManager m_sceneManager;
    NovaEngine::UIManager m_uiManager;
    std::unique_ptr<DialogueSystem> m_dialogueSystem;
    std::unique_ptr<PlayerController> m_playerController;
    
public:
    Game() : Application(createConfig()) { }
    
    bool onInitialize() override {
        // Initialize SceneManager
        m_sceneManager.initialize("data/definitions/", 
                                 "data/scenegraph.json");
        
        // Load first scene
        m_sceneManager.loadScene("data/scenes/ville.json", "ville");
        m_sceneManager.setActiveScene("ville");
        
        // Init UI system
        m_uiManager.setActionCallback([this](auto action, auto val, auto id) {
            handleUIAction(action, val, id);
        });
        
        return true;
    }
    
    void onUpdate(float deltaTime) override {
        // Update ECS
        m_sceneManager.update(deltaTime);
        
        // Update UI
        m_uiManager.update(deltaTime);
    }
    
    void onRender() override {
        m_sceneManager.render();
        m_uiManager.render();
    }
};
```

---

### 2. Logger

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Core/Logger.hpp` (lignes 1-80)

**Thread-safe singleton logger:**

```cpp
enum class LogLevel { Trace, Debug, Info, Warning, Error, Fatal };

class Logger {
public:
    static Logger& getInstance();
    
    void setLogFile(const std::string& filepath);
    void setLogLevel(LogLevel level);
    void enableAnsiColors(bool enable);
    
    void log(LogLevel level, std::string_view module, 
            std::string_view message);
    
    template<typename... Args>
    void logf(LogLevel level, std::string_view module,
             std::string_view format, Args&&... args) {
        if constexpr (sizeof...(args) == 0) {
            log(level, module, format);
        } else {
            std::string formatted = formatString(format, 
                                               std::forward<Args>(args)...);
            log(level, module, formatted);
        }
    }
    
private:
    Logger();
    ~Logger();
};

// Convenience macros
#define LOG_TRACE(...)   NovaEngine::Logger::getInstance().logf(...)
#define LOG_DEBUG(...)   NovaEngine::Logger::getInstance().logf(...)
#define LOG_INFO(...)    NovaEngine::Logger::getInstance().logf(...)
#define LOG_WARN(...)    NovaEngine::Logger::getInstance().logf(...)
#define LOG_ERROR(...)   NovaEngine::Logger::getInstance().logf(...)
#define LOG_FATAL(...)   NovaEngine::Logger::getInstance().logf(...)
```

---

## Events System

### 1. Event Structures

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Events/Event.hpp` (lignes 1-35)

```cpp
enum class EventType {
    Unknown,
    Input,      // Du backend (keyboard, mouse, etc.)
    UI,         // D'UI components
    Engine,     // Du moteur (scene load, etc.)
    Custom      // Événements personnalisés jeu
};

struct Event {
    EventType type = EventType::Unknown;
    
    // Pour Input events
    InputEvent inputEvent;
    
    // Pour autres types
    std::string name;
    std::string payload;
    
    Event() = default;
    
    Event(const InputEvent& evt)
        : type(EventType::Input), inputEvent(evt) {}
    
    Event(EventType type, const std::string& name, 
         const std::string& payload = "")
        : type(type), name(name), payload(payload) {}
};
```

---

### 2. EventHandler

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Events/EventHandler.hpp`

```cpp
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void onEvent(const Event& event) = 0;
};
```

---

### 3. EventDispatcher

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Events/EventDispatcher.hpp`

```cpp
class EventDispatcher {
public:
    // Enregistrer handler
    void subscribe(EventType type, std::weak_ptr<EventHandler> handler);
    
    // Dispatcher événement
    void dispatch(const Event& event);
    
    // Cleanup expired handlers
    void update();
};
```

---

## Scene Management

### 1. Scene Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Scene.hpp` (lignes 1-585)

```cpp
class Scene {
private:
    std::string m_name;
    std::string m_type;  // "interior" ou "exterior"
    Color m_backgroundColor = Color::Black;
    
    EntityRegistry m_entityRegistry;
    std::vector<std::unique_ptr<System>> m_systems;
    WaypointGraph m_waypointGraph;  // NPC pathfinding
    
public:
    explicit Scene(const std::string& name);
    
    const std::string& getName() const;
    const std::string& getType() const;
    const Color& getBackgroundColor() const;
    
    // Chargement depuis JSON
    bool loadFromJSON(const nlohmann::json& sceneData, 
                     const DefinitionManager& defManager);
    
    // Mise à jour et rendu
    void update(float deltaTime);
    void render();
    
    // Accès entités et systèmes
    EntityRegistry& getEntityRegistry();
    WaypointGraph& getWaypointGraph();
    
    // Pathfinding
    std::vector<Vec2f> findPath(const Vec2f& startPos,
                               const Vec2f& endPos,
                               const std::vector<std::string>& preferredTags = {});
    
private:
    // Entity creation helpers
    void createEntityFromJSON(const nlohmann::json& entityData,
                             const DefinitionManager& defManager);
    void createSpriteEntity(Entity* entity, 
                           const nlohmann::json& entityData,
                           const DefinitionManager& defManager);
    void createLightEntity(...);
    void createAnimatedSpriteEntity(...);
    void createAudioEntity(...);
    void createActivatorEntity(...);
    void createPlayerEntity(...);
};
```

**Ordre systèmes (constructor, lignes 38-49):**
1. AnimationSystem (mise à jour frames avant render)
2. PhysicsSystem (collisions)
3. ActivatorSystem (activation zones)
4. AudioSystem (son)
5. LightSystem (éclairage)
6. RenderSystem (rendu final)

**Logique JSON scene:**
```json
{
    "name": "interior_1",
    "type": "interior",
    "backgroundColor": [40, 40, 40, 255],
    "pathfinding": {
        "waypoints": [...],
        "connections": [...]
    },
    "entities": [
        {
            "type": "sprite",
            "position": [100, 200],
            "spriteID": "wall_01"
        },
        {
            "type": "activator",
            "position": [500, 300],
            "activatorID": "door_trigger"
        }
    ]
}
```

---

### 2. SceneManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/SceneManager.hpp` (lignes 1-303)

```cpp
class SceneManager {
private:
    DefinitionManager m_definitionManager;
    SceneGraph m_sceneGraph;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::unordered_set<std::string> m_activeScenesForUpdate;
    
public:
    // Initialisation (une seule fois au démarrage)
    bool initialize(const std::string& definitionsPath = 
                   "assets/data/definitions/",
                   const std::string& sceneGraphPath = 
                   "assets/data/scenegraph.json");
    
    // Chargement scènes
    bool loadScene(const std::string& scenePath,
                  const std::string& sceneName);
    void unloadScene(const std::string& sceneName);
    
    // Gestion active
    void setActiveScene(const std::string& sceneName);
    Scene* getActiveScene();
    
    // Requêtes
    Scene* getScene(const std::string& sceneName);
    bool hasScene(const std::string& sceneName) const;
    size_t getSceneCount() const;
    
    // Cycle principal
    void update(float deltaTime);
    void render();
    
    // Accesseurs
    const DefinitionManager& getDefinitionManager() const;
    SceneGraph& getSceneGraph();
    
    void clearScenes();
    void shutdown();
};
```

**Logique update() intelligente:**
```cpp
void SceneManager::update(float deltaTime) {
    // Collecter toutes scènes avec NPCs en voyage
    std::unordered_set<std::string> scenesOnActivePaths;
    
    for (auto& [sceneName, scene] : m_scenes) {
        auto travelers = scene->getEntityRegistry()
            .getEntitiesWith({"JourneyComponent"});
        
        for (Entity* entity : travelers) {
            auto* journey = entity->getComponent<JourneyComponent>();
            if (journey && journey->isOnJourney) {
                // Activer TOUTES scènes sur parcours
                for (const auto& sceneOnPath : journey->scenePath) {
                    scenesOnActivePaths.insert(sceneOnPath);
                }
            }
        }
    }
    
    // Update scènes
    for (auto& [name, scene] : m_scenes) {
        bool isRenderedScene = (m_activeScene == scene.get());
        bool isOnActivePath = (scenesOnActivePaths.find(name) != 
                              scenesOnActivePaths.end());
        
        if (isRenderedScene || isOnActivePath) {
            // Update scene et tous systèmes
            scene->update(deltaTime);
        }
        // Sinon: scène dormante, pas d'update
    }
}
```

**Avantage:** NPCs voyagent en arrière-plan, joueur peut les voir traverser scènes intermédiaires!

---

### 3. DefinitionManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/DefinitionManager.hpp` (lignes 1-327)

**Système deux-niveaux:**

Tier 1 (Startup): Charger TOUTES définitions une seule fois
Tier 2 (Per-scene): Scènes référencent définitions par ID

```cpp
class DefinitionManager {
private:
    std::unordered_map<ID, nlohmann::json> m_spriteDefinitions;
    std::unordered_map<ID, nlohmann::json> m_lightDefinitions;
    std::unordered_map<ID, nlohmann::json> m_animationDefinitions;
    std::unordered_map<ID, nlohmann::json> m_audioDefinitions;
    std::unordered_map<ID, nlohmann::json> m_activatorDefinitions;
    
public:
    // Charger ALL definitions
    bool loadDefinitions(const std::string& definitionsPath = 
                        "assets/data/definitions/");
    
    // Requêtes
    const nlohmann::json* getSpriteDefinition(const ID& id) const;
    const nlohmann::json* getLightDefinition(const ID& id) const;
    const nlohmann::json* getAnimationDefinition(const ID& id) const;
    const nlohmann::json* getAudioDefinition(const ID& id) const;
    const nlohmann::json* getActivatorDefinition(const ID& id) const;
    
    // Vérifications
    bool hasSpriteDefinition(const ID& id) const;
    bool hasLightDefinition(const ID& id) const;
    // ... etc
};
```

**Fichiers attendus:**
```
assets/data/definitions/
├── Sprites.json        # {"sprites": [{id, texture, texturePath, ...}, ...]}
├── Lights.json         # {"lights": [{id, type, color, radius, ...}, ...]}
├── Animations.json     # {"animations": [{id, frames, frameDuration, ...}, ...]}
├── Audio.json          # {"sounds": [{id, path, volume, ...}, ...]}
└── Activators.json     # {"activators": [{id, type, shape, size, ...}, ...]}
```

---

## Pathfinding Systems

### 1. WaypointGraph

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/WaypointGraph.hpp` (lignes 1-396)

**Structures:**

```cpp
struct Waypoint {
    std::string id;                      // "fountain", "north_plaza"
    Vec2f position;                      // Position physique
    std::vector<std::string> tags;       // "main_road", "shortcut", "scenic"
};

struct WaypointConnection {
    std::string from;
    std::string to;
    f32 cost;                            // Distance ou temps
    std::vector<std::string> tags;       // Filtrage personnalité
    bool bidirectional;
};
```

**Algorithme: BFS (Breadth-First Search)**

```cpp
class WaypointGraph {
public:
    bool loadFromJSON(const nlohmann::json& json);
    
    const Waypoint* findNearestWaypoint(const Vec2f& position,
                                       f32 maxDistance = -1.0f) const;
    
    const Waypoint* findWaypointByID(const std::string& id) const;
    
    // Chemin posititons
    std::vector<Vec2f> findPath(const Vec2f& startPos,
                               const Vec2f& endPos,
                               const std::vector<std::string>& preferredTags = {}) const;
    
    // Chemin IDs
    std::vector<std::string> findPathByID(const std::string& startID,
                                         const std::string& endID,
                                         const std::vector<std::string>& preferredTags = {}) const;
    
    const std::vector<Waypoint>& getWaypoints() const;
    const std::vector<WaypointConnection>& getConnections() const;
    bool isEmpty() const;
};
```

**findPathByID implementation (lignes 296-349):**

```cpp
inline std::vector<std::string> WaypointGraph::findPathByID(
    const std::string& startID, const std::string& endID,
    const std::vector<std::string>& preferredTags) const {
    
    if (startID == endID) {
        return {startID};
    }
    
    // BFS
    std::queue<std::string> queue;
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_set<std::string> visited;
    
    queue.push(startID);
    visited.insert(startID);
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        if (current == endID) {
            return reconstructPath(cameFrom, startID, endID);
        }
        
        // Explorer voisins (connections)
        for (const auto& conn : m_connections) {
            std::string neighbor;
            
            if (conn.from == current) {
                neighbor = conn.to;
            } else if (conn.bidirectional && conn.to == current) {
                neighbor = conn.from;
            } else {
                continue;
            }
            
            // Filtrer par preferredTags si spécifié
            if (!preferredTags.empty() && !hasPreferredTag(conn, preferredTags)) {
                continue;
            }
            
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }
    
    // Pas de chemin trouvé
    LOG_WARN("No waypoint path found from '{}' to '{}'", startID, endID);
    return {};
}
```

**findPath implementation (lignes 351-394):**

```cpp
inline std::vector<Vec2f> WaypointGraph::findPath(
    const Vec2f& startPos, const Vec2f& endPos,
    const std::vector<std::string>& preferredTags) const {
    
    // 1. Trouver waypoints les plus proches
    const Waypoint* startWp = findNearestWaypoint(startPos);
    const Waypoint* endWp = findNearestWaypoint(endPos);
    
    if (!startWp || !endWp) {
        LOG_WARN("Cannot find waypoints near start/end positions");
        return {};
    }
    
    // 2. Trouver chemin waypoints
    auto pathIDs = findPathByID(startWp->id, endWp->id, preferredTags);
    
    if (pathIDs.empty()) {
        return {};
    }
    
    // 3. Convertir IDs → positions
    std::vector<Vec2f> path;
    path.reserve(pathIDs.size() + 2);
    
    // Ajouter pos départ exacte si loin du premier waypoint
    if (distance(startPos, startWp->position) > 5.0f) {
        path.push_back(startPos);
    }
    
    // Ajouter waypoints
    for (const auto& id : pathIDs) {
        const Waypoint* wp = findWaypointByID(id);
        if (wp) {
            path.push_back(wp->position);
        }
    }
    
    // Ajouter pos destination exacte si différente
    if (distance(endPos, endWp->position) > 5.0f) {
        path.push_back(endPos);
    }
    
    return path;
}
```

**Example JSON format:**
```json
{
    "waypoints": [
        {"id": "fountain", "position": [640, 360], "tags": ["landmark", "center"]},
        {"id": "north_plaza", "position": [640, 100], "tags": ["main_road"]},
        {"id": "shop", "position": [400, 300], "tags": ["shortcut"]}
    ],
    "connections": [
        {"from": "fountain", "to": "north_plaza", "tags": ["main_road"], "bidirectional": true},
        {"from": "fountain", "to": "shop", "tags": ["shortcut"], "bidirectional": true}
    ]
}
```

---

### 2. SceneGraph

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/SceneGraph.hpp` (lignes 1-208)

**Pour voyages multi-scène NPCs:**

```cpp
struct SceneConnection {
    std::string fromScene;
    std::string toScene;
    Vec2f exitPortalPos;       // Où quitter fromScene
    Vec2f entryPortalPos;      // Où arriver dans toScene
    f32 travelTime;            // Temps estimé (pas utilisé actuellement)
    bool bidirectional;
};

class SceneGraph {
private:
    std::unordered_map<std::string, std::vector<SceneConnection>>
        m_connections;
    
public:
    void addConnection(const SceneConnection& connection);
    bool loadFromJSON(const std::string& path);
    
    // Pathfinding: trouver route scenes
    std::vector<std::string> findPath(const std::string& startScene,
                                     const std::string& endScene);
    
    // Détails connexion
    const SceneConnection* getConnection(const std::string& fromScene,
                                        const std::string& toScene) const;
    
    std::vector<std::string> getNeighbors(const std::string& sceneName) const;
    bool areConnected(const std::string& sceneA,
                     const std::string& sceneB) const;
    
    size_t getSceneCount() const;
};
```

**findPath implementation (lignes 106-159):**

```cpp
std::vector<std::string> SceneGraph::findPath(
    const std::string& startScene,
    const std::string& endScene) {
    
    if (startScene == endScene) {
        return {startScene};
    }
    
    // BFS pour chemin le plus court
    std::queue<std::string> queue;
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_set<std::string> visited;
    
    queue.push(startScene);
    visited.insert(startScene);
    cameFrom[startScene] = "";
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        if (current == endScene) {
            // Reconstruire chemin
            std::vector<std::string> path;
            std::string node = endScene;
            while (!node.empty()) {
                path.push_back(node);
                node = cameFrom[node];
            }
            std::reverse(path.begin(), path.end());
            
            LOG_DEBUG("Found path from '{}' to '{}': {} scenes",
                     startScene, endScene, path.size());
            return path;
        }
        
        // Explorer voisins
        auto it = m_connections.find(current);
        if (it != m_connections.end()) {
            for (const auto& connection : it->second) {
                if (visited.find(connection.toScene) == visited.end()) {
                    queue.push(connection.toScene);
                    visited.insert(connection.toScene);
                    cameFrom[connection.toScene] = current;
                }
            }
        }
    }
    
    LOG_WARN("No path found from '{}' to '{}'", startScene, endScene);
    return {};
}
```

**Example JSON format (scenegraph.json):**
```json
{
    "connections": [
        {
            "from": "exterior_town",
            "to": "interior_shop",
            "exitPortal": [100, 200],
            "entryPortal": [50, 100],
            "travelTime": 2.0,
            "bidirectional": true
        },
        {
            "from": "interior_shop",
            "to": "interior_storage",
            "exitPortal": [400, 300],
            "entryPortal": [10, 10],
            "travelTime": 1.0,
            "bidirectional": true
        }
    ]
}
```

---

## Resource Management

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Resources/ResourceManager.hpp` (lignes 1-58)

```cpp
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // Charger depuis JSON
    bool loadFromJSON(const std::string& path);
    
    // === TEXTURES ===
    bool loadTexture(const ID& id, const std::string& path);
    sf::Texture& getTexture(const ID& id);
    
    // === FONTS ===
    bool loadFont(const ID& id, const std::string& path);
    sf::Font& getFont(const ID& id);
    
    // === SOUNDS ===
    bool loadSoundBuffer(const ID& id, const std::string& path);
    sf::SoundBuffer& getSoundBuffer(const ID& id);
    
    // === MUSIC ===
    bool loadMusic(const ID& id, const std::string& path);
    std::string getMusicPath(const ID& id) const;
    
    void clear();
    
private:
    std::unordered_map<ID, std::unique_ptr<sf::Texture>> m_textures;
    std::unordered_map<ID, std::unique_ptr<sf::Font>> m_fonts;
    std::unordered_map<ID, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
    std::unordered_map<ID, std::string> m_musicPaths;
};
```

---

## Implémentations SFML

**Architecture générale backends:**

```
BackendManager (Singleton Facade)
    │
    ├─→ SFMLWindowBackend     : IWindowBackend
    ├─→ SFMLInputBackend      : IInputBackend
    ├─→ SFMLGraphicsBackend   : IGraphicsBackend
    ├─→ SFMLResourceBackend   : IResourceBackend
    ├─→ SFMLAudioBackend      : IAudioBackend
    ├─→ SFMLFontBackend       : IFontBackend
    └─→ SFMLViewportBackend   : IViewportBackend
```

**Chaque backend:**
- Encapsule objet SFML native
- Implémente interface virtuelle
- Gère ressources (destruction propre)
- Logs pour debug

---

## Flux Intégré Complet

### 1. Application Startup Flow

```
main()
  ↓
Game::run() [Application::run]
  ↓
initializeEngine()
  ├─ BackendManager::initialize(SFML)
  │   ├─ createBackends(SFML)
  │   │   ├─ SFMLWindowBackend::create()
  │   │   ├─ SFMLGraphicsBackend::initialize()
  │   │   └─ SFMLAudioBackend::initialize()
  │   └─ WINDOW().setVSync(), WINDOW().setFramerateLimit()
  ↓
Game::onInitialize()
  ├─ SceneManager::initialize()
  │   ├─ DefinitionManager::loadDefinitions()
  │   │   ├─ loadSpriteDefinitions()
  │   │   ├─ loadLightDefinitions()
  │   │   ├─ loadAnimationDefinitions()
  │   │   ├─ loadAudioDefinitions()
  │   │   └─ loadActivatorDefinitions()
  │   └─ SceneGraph::loadFromJSON()
  ├─ SceneManager::loadScene("ville.json", "ville")
  │   └─ Scene::loadFromJSON()
  │       ├─ WaypointGraph::loadFromJSON()
  │       ├─ EntityRegistry::createEntity() × N
  │       └─ attach components from definitions
  ├─ SceneManager::setActiveScene("ville")
  ├─ UIManager setup
  └─ DialogueSystem::initialize(&m_uiManager)
  ↓
runMainLoop()
```

### 2. Main Loop Flow (each frame)

```
while (WINDOW().isOpen())
  ↓
Process Events
  ├─ INPUT().pollEvent()
  ├─ Convert InputEvent → Event
  └─ Game::onEvent()
  ↓
Game::onUpdate(deltaTime)
  ├─ PlayerController::updateMovement()
  ├─ SceneManager::update(deltaTime)
  │   ├─ For each scene on active paths:
  │   │   ├─ scene→update()
  │   │   │   ├─ AnimationSystem::update()
  │   │   │   ├─ PhysicsSystem::update()
  │   │   │   ├─ ActivatorSystem::update()
  │   │   │   ├─ AudioSystem::update()
  │   │   │   ├─ LightSystem::update()
  │   │   │   └─ RenderSystem::update() [prepares data]
  ├─ UIManager::update(deltaTime)
  │   └─ each UIComponent.update()
  └─ DialogueSystem::update()
  ↓
Game::onRender()
  ├─ WINDOW().clear(bg_color)
  ├─ SceneManager::render()
  │   └─ For each system: execute graphics commands
  │       ├─ GRAPHICS().drawSprite() × N
  │       ├─ GRAPHICS().drawRect() × M
  │       ├─ GRAPHICS().drawText() × P
  ├─ UIManager::render()
  │   └─ For each layer:
  │       └─ UIComponent::render()
  │           ├─ GRAPHICS().drawRect()
  │           ├─ GRAPHICS().drawText()
  │           └─ GRAPHICS().drawSprite()
  │
  ├─ WINDOW().display()
```

### 3. Entity Creation Flow

```
Scene::createEntityFromJSON(entityData, defManager)
  ├─ EntityRegistry::createEntity()
  │   └─ new Entity(nextID++)
  ├─ addComponent(TransformComponent) [always]
  └─ Based on type field:
      ├─ "sprite" → createSpriteEntity()
      │   ├─ getSpriteDefinition(spriteID)
      │   ├─ new SpriteComponent()
      │   ├─ RESOURCES().loadTexture(texturePath)
      │   └─ entity→addComponent(sprite)
      ├─ "light" → createLightEntity()
      ├─ "animated_sprite" → createAnimatedSpriteEntity()
      │   ├─ createSpriteEntity() first
      │   ├─ getAnimationDefinition()
      │   ├─ new AnimationComponent()
      │   └─ entity→addComponent(anim)
      ├─ "audio" → createAudioEntity()
      ├─ "activator" → createActivatorEntity()
      └─ "player" → createPlayerEntity()
```

### 4. NPC Journey Flow

```
Game logic: "Send NPC Alice to market"
  ↓
JourneySystem::startJourney(alice_entity, "interior_home", 
                           "exterior_market", market_pos)
  ├─ JourneyComponent::scenePath = SceneGraph::findPath(...)
  │   └─ Returns: ["interior_home", "street", "exterior_market"]
  ├─ JourneyComponent::currentDestination = first portal exit pos
  ├─ JourneyComponent::isOnJourney = true
  ↓
Each Frame: JourneySystem::update(deltaTime)
  ├─ Check if reached currentDestination
  ├─ If yes AND more scenes in path:
  │   ├─ Get connection info from SceneGraph
  │   ├─ Mark for transfer via SceneTransitionComponent
  │   ├─ Add to m_pendingTransfers
  ├─ If yes AND final scene:
  │   └─ Mark journey complete
  ↓
SceneManager sees pendingTransfers
  ├─ Move entity from "interior_home" registry
  │     to "street" registry
  ├─ Set position to entry portal
  ├─ Reset JourneyComponent state
  ↓
Next frame: NPC appears in "street" scene
  ├─ Recalculate waypoint path if WaypointGraph present
  ├─ Follow waypoints to next portal
  ├─ Player might see NPC walking through street!
  ↓
Repeat until reaching final scene and destination
```

---

## Résumé Architecture

### Patterns Utilisés
1. **Singleton:** Logger, BackendManager
2. **Facade:** BackendManager pour interfaces
3. **Strategy:** Systems pour update/render logic
4. **Component-Based:** ECS
5. **Template Method:** Application
6. **Factory:** Scene entity creation
7. **Observer:** Event system

### Séparation des Responsabilités
- **ECS Layer:** Entity/Component/System management
- **Backend Layer:** Low-level graphics/input/audio
- **UI Layer:** UI rendering and events
- **Application Layer:** Game-specific logic
- **Core Layer:** Logging, Config, Resources

### Performance Considerations
- **System-based update:** Grouped entities by component type
- **Render caching:** UIManager maintains sorted layer cache
- **Multi-scene updates:** Only update scenes on active paths
- **BFS pathfinding:** O(V+E) complexity for scene/waypoint graphs

### Extensibility
- Add new components: inherit Component, implement serialize/deserialize
- Add new systems: inherit System, implement update/getRequiredComponents
- Add new UI components: inherit UIComponent, implement render/onEvent
- Add new backends: implement IXxxBackend interfaces

---

## Conclusion

NovaEngine est une architecture robuste et extensible pour jeux 2D. Le système ECS fournit flexibilité et performance. L'abstraction backend permet portabilité. Le système scene/waypoint/journey permet narrative complxe avec NPCs voyageant organiquement.

**Date de documentation:** 16 Novembre 2025  
**Auteur:** Code Analysis System  
**Version analysée:** Latest from repo

