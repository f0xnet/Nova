# Entity Component System (ECS) - NovaEngine

Le cœur de NovaEngine repose sur une architecture **Entity Component System** pure qui sépare strictement les **données** (Components) de la **logique** (Systems).

## Principes ECS

### Philosophie

```
Entity    = Conteneur d'ID + Composants
Component = Pure Data (pas de logique)
System    = Pure Logic (opère sur entités ayant certains composants)
```

### Avantages

- **Composition flexible** : Entités définies par leurs composants
- **Data-oriented** : Cache-friendly, performance optimale
- **Modularité** : Facile d'ajouter/enlever comportements
- **Maintenabilité** : Séparation données/logique claire
- **Réutilisabilité** : Composants réutilisables entre entités

---

## Structure des fichiers

```
sdk/include/NovaEngine/ECS/
├── Entity.hpp              # Conteneur d'entité
├── Component.hpp           # Base class pour composants
├── Components.hpp          # 11 composants built-in
├── System.hpp              # Base class pour systèmes
├── Systems.hpp             # 7 systèmes built-in
├── EntityRegistry.hpp      # Base de données d'entités
├── Scene.hpp               # Scène = entities + systems + waypoints
├── SceneManager.hpp        # Gestion multi-scènes
├── DefinitionManager.hpp   # Chargement définitions JSON
├── SceneGraph.hpp          # Pathfinding inter-scènes
└── WaypointGraph.hpp       # Pathfinding intra-scène
```

---

## Entity

### Définition

Une **Entity** est simplement un **conteneur d'ID unique** avec une collection de **Components**.

```cpp
class Entity {
private:
    u64 m_id;
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
    explicit Entity(u64 id) : m_id(id) {}

    u64 getID() const { return m_id; }

    // Ajouter composant (transfert ownership)
    template<typename T>
    T* addComponent(std::unique_ptr<T> component);

    // Récupérer composant
    template<typename T>
    T* getComponent();

    template<typename T>
    const T* getComponent() const;

    // Vérifier existence
    template<typename T>
    bool hasComponent() const;

    bool hasComponent(const ComponentTypeID& typeID) const;

    // Supprimer composant
    template<typename T>
    void removeComponent();

    // Liste des types
    std::vector<ComponentTypeID> getComponentTypes() const;
};
```

### Utilisation

```cpp
// Créer entité (via EntityRegistry normalement)
Entity* entity = registry.createEntity();  // ID auto-assigné

// Ajouter composants
auto transform = std::make_unique<TransformComponent>();
transform->position = Vec2f{100, 200};
transform->rotation = 0.0f;
transform->scale = Vec2f{1, 1};
entity->addComponent(std::move(transform));

auto sprite = std::make_unique<SpriteComponent>();
sprite->textureHandle = RESOURCES().loadTexture("player.png");
sprite->size = Vec2f{64, 64};
entity->addComponent(std::move(sprite));

// Récupérer composant
TransformComponent* trans = entity->getComponent<TransformComponent>();
trans->position.x += 10;  // Déplacer

// Vérifier existence
if (entity->hasComponent<SpriteComponent>()) {
    // ...
}

// Supprimer composant
entity->removeComponent<SpriteComponent>();
```

### Implémentation des templates

```cpp
template<typename T>
T* Entity::addComponent(std::unique_ptr<T> component) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    ComponentTypeID typeID = component->getTypeID();
    T* ptr = component.get();
    m_components[typeID] = std::move(component);
    return ptr;
}

template<typename T>
T* Entity::getComponent() {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    T temp_instance;  // Instance temporaire pour obtenir typeID
    ComponentTypeID typeID = temp_instance.getTypeID();

    auto it = m_components.find(typeID);
    if (it != m_components.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
bool Entity::hasComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    T temp_instance;
    ComponentTypeID typeID = temp_instance.getTypeID();
    return m_components.find(typeID) != m_components.end();
}
```

---

## Component

### Base class

```cpp
using ComponentTypeID = std::string;

class Component {
public:
    virtual ~Component() = default;

    // Type ID unique (nom de classe)
    virtual ComponentTypeID getTypeID() const = 0;

    // Sérialisation JSON
    virtual void serialize(nlohmann::json& json) const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
};
```

### Macro helper

```cpp
#define COMPONENT_TYPE_ID(TypeName) \
    ComponentTypeID getTypeID() const override { return #TypeName; }
```

### Créer un composant custom

```cpp
class HealthComponent : public Component {
public:
    f32 currentHealth = 100.0f;
    f32 maxHealth = 100.0f;
    bool isInvincible = false;

    COMPONENT_TYPE_ID(HealthComponent)

    void serialize(nlohmann::json& json) const override {
        json["currentHealth"] = currentHealth;
        json["maxHealth"] = maxHealth;
        json["isInvincible"] = isInvincible;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("currentHealth")) currentHealth = json["currentHealth"];
        if (json.contains("maxHealth")) maxHealth = json["maxHealth"];
        if (json.contains("isInvincible")) isInvincible = json["isInvincible"];
    }
};

// Utilisation
auto health = std::make_unique<HealthComponent>();
health->currentHealth = 75.0f;
entity->addComponent(std::move(health));
```

---

## Components built-in (11 composants)

### 1. TransformComponent

Position, rotation, échelle d'une entité.

```cpp
class TransformComponent : public Component {
public:
    Vec2f position = {0.0f, 0.0f};  // Position monde (pixels)
    f32 rotation = 0.0f;            // Rotation (degrés)
    Vec2f scale = {1.0f, 1.0f};     // Échelle
    Vec2f origin = {0.0f, 0.0f};    // Point d'origine pour rotation

    COMPONENT_TYPE_ID(TransformComponent)
    // serialize/deserialize...
};
```

**Utilisé par** : Toutes entités visibles, physiques, ou positionnées dans le monde.

### 2. SpriteComponent

Rendu de sprite 2D.

```cpp
class SpriteComponent : public Component {
public:
    ID textureID;                           // ID définition
    TextureHandle textureHandle = INVALID_HANDLE;  // Handle GPU
    IntRect textureRect = {0, 0, 0, 0};     // Sub-rectangle (0=full texture)
    Vec2f size = {0.0f, 0.0f};              // Taille affichage (0=native)
    Color tint = Color::White;              // Teinte
    BlendMode blendMode = BlendMode::Alpha;
    i32 zOrder = 0;                         // Ordre de rendu (0=fond)
    bool visible = true;

    COMPONENT_TYPE_ID(SpriteComponent)
    // ...
};
```

**Utilisé par** : Toutes entités visuelles (joueur, NPCs, objets, décors).

### 3. LightComponent

Source de lumière (point, directionnelle, spot).

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
    bool castShadows = false;      // (non implémenté actuellement)
    bool enabled = true;

    COMPONENT_TYPE_ID(LightComponent)
    // ...
};
```

**Utilisé par** : Torches, lampes, soleil/lune, projecteurs.

**Types** :
- **Point** : Omnidirectionnelle (torche)
- **Directional** : Parallèle (soleil)
- **Spot** : Cône (lampe de poche)

### 4. AnimationComponent

Animation frame-based.

```cpp
class AnimationComponent : public Component {
public:
    ID animationID;                     // Référence définition
    std::vector<IntRect> frames;        // Rectangles de chaque frame
    f32 frameDuration = 0.1f;           // Temps par frame (secondes)
    f32 currentTime = 0.0f;             // Temps écoulé dans frame actuelle
    u32 currentFrame = 0;               // Index frame actuelle
    bool loop = true;
    bool playing = true;

    COMPONENT_TYPE_ID(AnimationComponent)
    // ...
};
```

**Utilisé par** : Personnages animés, flammes, eau, etc.

**Note** : AnimationSystem avance les frames et met à jour SpriteComponent.textureRect.

### 5. ColliderComponent

Collision physique (boîte ou cercle).

```cpp
class ColliderComponent : public Component {
public:
    enum class ColliderType { Box, Circle };

    ColliderType type = ColliderType::Box;
    Vec2f size = {0.0f, 0.0f};     // Pour Box
    f32 radius = 0.0f;             // Pour Circle
    Vec2f offset = {0.0f, 0.0f};   // Offset depuis position entity
    bool isTrigger = false;        // Trigger (pas de physique) ou solide
    bool enabled = true;

    COMPONENT_TYPE_ID(ColliderComponent)
    // ...
};
```

**Utilisé par** : Joueur, NPCs, murs, objets physiques.

**Types** :
- **Box** : Rectangle (AABB)
- **Circle** : Cercle

**Trigger vs Solid** :
- Trigger : Détecte collision mais ne bloque pas (zone de dialogue, pickup)
- Solid : Bloque mouvement (mur, obstacle)

### 6. AudioComponent

Lecture son/musique.

```cpp
class AudioComponent : public Component {
public:
    ID soundID;                    // ID définition
    SoundHandle soundHandle = INVALID_HANDLE;  // Handle audio
    bool playOnStart = false;
    bool loop = false;
    f32 volume = 100.0f;
    f32 pitch = 1.0f;
    bool playing = false;

    COMPONENT_TYPE_ID(AudioComponent)
    // ...
};
```

**Utilisé par** : Ambiance, bruits d'objets, musique de zone.

### 7. ActivatorComponent

Zone de déclenchement (trigger zone).

```cpp
class ActivatorComponent : public Component {
public:
    enum class ActivatorType {
        Proximity,      // Active quand entité entre dans zone
        Manual,         // Requiert activation manuelle (touche)
        Automatic       // Active en continu tant qu'entité dans zone
    };

    enum class ActivatorShape {
        Box,            // Rectangulaire
        Circle          // Circulaire
    };

    ActivatorType type = ActivatorType::Proximity;
    ActivatorShape shape = ActivatorShape::Box;

    Vec2f size = {100.0f, 100.0f};     // Pour Box
    f32 radius = 50.0f;                // Pour Circle
    Vec2f offset = {0.0f, 0.0f};       // Offset depuis position

    bool isActive = false;             // État actuel
    bool canReactivate = true;         // Peut être réactivé
    f32 cooldownTime = 0.0f;           // Temps avant réactivation (secondes)
    f32 currentCooldown = 0.0f;        // Timer cooldown actuel

    std::string targetTag = "player";  // Quel tag peut activer
    std::string actionID;              // ID action (pour game logic)

    // Debug visuel
    bool showDebugZone = false;
    Color debugColor = Color{0, 255, 0, 100};

    // Événements
    std::string onActivateEvent;       // Nom événement à fire
    std::string onDeactivateEvent;

    COMPONENT_TYPE_ID(ActivatorComponent)
    // ...
};
```

**Utilisé par** : Interrupteurs, portes, plaques de pression, zones de dialogue.

**Types** :
- **Proximity** : Active une fois à l'entrée
- **Manual** : Nécessite touche (ex: E)
- **Automatic** : Active continuellement

### 8. TagComponent

Identification d'entité (simple string).

```cpp
class TagComponent : public Component {
public:
    std::string tag = "default";

    COMPONENT_TYPE_ID(TagComponent)
    // ...
};
```

**Utilisé par** : Identification entités pour triggers, collisions, recherches.

**Tags courants** : "player", "npc", "enemy", "door", "chest", etc.

### 9. SceneTransitionComponent

Transition entre scènes.

```cpp
class SceneTransitionComponent : public Component {
public:
    std::string targetScene;     // Scène destination
    Vec2f targetPosition;        // Position dans scène destination
    bool isTransitioning = false;

    COMPONENT_TYPE_ID(SceneTransitionComponent)
    // ...
};
```

**Utilisé par** : Entités devant se déplacer entre scènes (NPCs, joueur).

### 10. ShaderComponent

Shader personnalisé par entité.

```cpp
class ShaderComponent : public Component {
public:
    ShaderHandle shader = INVALID_HANDLE;
    bool enabled = true;

    COMPONENT_TYPE_ID(ShaderComponent)
    // ...
};
```

**Utilisé par** : Entités avec effet shader spécial (distorsion, glow, etc.).

### 11. JourneyComponent

Voyage multi-scènes pour NPCs.

```cpp
class JourneyComponent : public Component {
public:
    // Chemin inter-scènes
    std::vector<std::string> scenePath;    // ["ville", "taverne", "maison"]
    int currentSceneIndex = 0;

    Vec2f currentDestination;              // Destination dans scène actuelle
    bool reachedCurrentDestination = false;

    // Chemin intra-scène (waypoints)
    std::vector<Vec2f> localWaypointPath;  // Waypoints à suivre
    int currentLocalWaypointIndex = 0;

    // Personnalité (préférence de chemins)
    std::vector<std::string> preferredPathTags;  // ["main_road"], ["shortcut"]

    bool isOnJourney = false;
    std::string finalDestinationScene;
    Vec2f finalDestinationPos;

    COMPONENT_TYPE_ID(JourneyComponent)
    // ...
};
```

**Utilisé par** : NPCs voyageant entre scènes (marchands, messagers, etc.).

**Fonctionnement** :
1. JourneySystem calcule chemin inter-scènes via SceneGraph
2. Pour chaque scène, calcule chemin de waypoints via WaypointGraph
3. NPC suit waypoints jusqu'au portail de sortie
4. Transition vers scène suivante
5. Répète jusqu'à destination finale

---

## EntityRegistry

### Base de données d'entités

```cpp
class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    u64 m_nextEntityID = 1;

public:
    // Créer/détruire
    Entity* createEntity();
    void destroyEntity(u64 entityID);
    void destroyEntity(Entity* entity);
    void clear();

    // Recherche
    Entity* getEntity(u64 entityID);
    const Entity* getEntity(u64 entityID) const;

    // Queries
    std::vector<Entity*> getAllEntities();
    std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& types);
    std::vector<Entity*> getEntitiesWithTag(const std::string& tag);

    // Stats
    size_t getEntityCount() const;
};
```

### Utilisation

```cpp
EntityRegistry registry;

// Créer entités
Entity* player = registry.createEntity();
Entity* enemy = registry.createEntity();

// Query : toutes entités avec Transform + Sprite
auto renderables = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});

for (Entity* e : renderables) {
    auto* transform = e->getComponent<TransformComponent>();
    auto* sprite = e->getComponent<SpriteComponent>();
    // Rendre...
}

// Query par tag
auto enemies = registry.getEntitiesWithTag("enemy");

// Détruire
registry.destroyEntity(enemy);

// Tout nettoyer
registry.clear();
```

### Implémentation de createEntity

```cpp
Entity* EntityRegistry::createEntity() {
    u64 id = m_nextEntityID++;
    auto entity = std::make_unique<Entity>(id);
    Entity* ptr = entity.get();
    m_entities[id] = std::move(entity);
    LOG_TRACE("Created entity with ID: {}", id);
    return ptr;
}
```

### Implémentation de getEntitiesWith

```cpp
std::vector<Entity*> EntityRegistry::getEntitiesWith(
    const std::vector<ComponentTypeID>& types) {

    std::vector<Entity*> result;

    for (auto& pair : m_entities) {
        Entity* entity = pair.second.get();

        // Vérifier si entité a TOUS les composants requis
        bool hasAll = true;
        for (const auto& type : types) {
            if (!entity->hasComponent(type)) {
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
```

---

## System

### Base class

```cpp
class System {
public:
    virtual ~System() = default;

    // Update appelé chaque frame
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;

    // Composants requis pour ce système
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;
};
```

### Créer un système custom

```cpp
class DamageSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // Query toutes entités avec Health + Collider
        auto entities = registry.getEntitiesWith(
            {"HealthComponent", "ColliderComponent"}
        );

        for (Entity* entity : entities) {
            auto* health = entity->getComponent<HealthComponent>();
            auto* collider = entity->getComponent<ColliderComponent>();

            // Logique de dégâts
            if (collider->isTakingDamage) {
                health->currentHealth -= damageAmount * deltaTime;

                if (health->currentHealth <= 0) {
                    LOG_INFO("Entity {} died", entity->getID());
                    registry.destroyEntity(entity);
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"HealthComponent", "ColliderComponent"};
    }

private:
    f32 damageAmount = 10.0f;
};
```

---

## Scene

### Conteneur scène = entities + systems

```cpp
class Scene {
private:
    std::string m_name;
    std::string m_type;  // "interior" ou "exterior"
    Color m_backgroundColor = Color::Black;

    EntityRegistry m_entityRegistry;
    std::vector<std::unique_ptr<System>> m_systems;
    WaypointGraph m_waypointGraph;  // Pathfinding local

public:
    explicit Scene(const std::string& name);

    const std::string& getName() const;
    const std::string& getType() const;
    const Color& getBackgroundColor() const;

    // Chargement depuis JSON
    bool loadFromJSON(const nlohmann::json& sceneData,
                     const DefinitionManager& defManager);

    // Update/Render
    void update(float deltaTime);
    void render();

    // Accès registry
    EntityRegistry& getEntityRegistry();
    const EntityRegistry& getEntityRegistry() const;

    // Accès waypoint graph
    WaypointGraph& getWaypointGraph();
    const WaypointGraph& getWaypointGraph() const;

    // Pathfinding
    std::vector<Vec2f> findPath(const Vec2f& startPos, const Vec2f& endPos,
                               const std::vector<std::string>& preferredTags = {});
};
```

### Initialisation dans constructeur

```cpp
Scene::Scene(const std::string& name) : m_name(name) {
    // Créer systèmes par défaut (ordre important!)
    m_systems.push_back(std::make_unique<AnimationSystem>());
    m_systems.push_back(std::make_unique<PhysicsSystem>());
    m_systems.push_back(std::make_unique<ActivatorSystem>());
    m_systems.push_back(std::make_unique<AudioSystem>());
    m_systems.push_back(std::make_unique<LightSystem>());
    m_systems.push_back(std::make_unique<RenderSystem>());  // Dernier!

    LOG_DEBUG("Created scene: {}", m_name);
}
```

### Update loop

```cpp
void Scene::update(float deltaTime) {
    // Update tous les systèmes SAUF RenderSystem (dernier)
    for (size_t i = 0; i < m_systems.size() - 1; ++i) {
        m_systems[i]->update(deltaTime, m_entityRegistry);
    }
}

void Scene::render() {
    WINDOW().clear(m_backgroundColor);

    // Appeler RenderSystem (dernier système)
    if (!m_systems.empty()) {
        m_systems.back()->update(0.0f, m_entityRegistry);
    }
}
```

---

## Exemple complet : Créer une entité

### Via code

```cpp
// 1. Créer entité
Entity* player = registry.createEntity();

// 2. Transform (obligatoire pour entités visuelles)
auto transform = std::make_unique<TransformComponent>();
transform->position = Vec2f{100, 200};
transform->scale = Vec2f{1, 1};
player->addComponent(std::move(transform));

// 3. Sprite
auto sprite = std::make_unique<SpriteComponent>();
sprite->textureHandle = RESOURCES().loadTexture("assets/player.png");
sprite->size = Vec2f{64, 64};
sprite->zOrder = 10;  // Au-dessus du décor
player->addComponent(std::move(sprite));

// 4. Tag
auto tag = std::make_unique<TagComponent>();
tag->tag = "player";
player->addComponent(std::move(tag));

// 5. Collider
auto collider = std::make_unique<ColliderComponent>();
collider->type = ColliderComponent::ColliderType::Box;
collider->size = Vec2f{60, 60};  // Légèrement plus petit que sprite
collider->isTrigger = false;  // Solide
player->addComponent(std::move(collider));

// L'entité est maintenant complète et fonctionnelle!
```

### Via JSON (recommandé)

**Définition** (`data/definitions/Sprites.json`) :
```json
{
  "player": {
    "texture": "assets/textures/player.png",
    "size": [64, 64],
    "origin": [32, 32],
    "zOrder": 10
  }
}
```

**Scene** (`data/scenes/test.json`) :
```json
{
  "name": "test",
  "type": "exterior",
  "backgroundColor": [30, 30, 40, 255],
  "entities": [
    {
      "type": "player",
      "spriteID": "player",
      "position": [100, 200],
      "tag": "player"
    }
  ]
}
```

**Chargement** :
```cpp
SceneManager sceneManager;
sceneManager.initialize("data/definitions/", "data/scenegraph.json");
sceneManager.loadScene("data/scenes/test.json", "test");
```

---

## Diagrammes

### Relation Entity-Component-System

```
┌──────────────────────────────────────────────┐
│              EntityRegistry                   │
│  ┌────────┐  ┌────────┐  ┌────────┐         │
│  │Entity 1│  │Entity 2│  │Entity 3│         │
│  │  ID=1  │  │  ID=2  │  │  ID=3  │         │
│  └────────┘  └────────┘  └────────┘         │
│      │           │           │               │
│      │ owns      │ owns      │ owns          │
│      ▼           ▼           ▼               │
│  ┌────────┐  ┌────────┐  ┌────────┐         │
│  │Transform│ │Transform│ │Transform│         │
│  │ Sprite  │ │ Light   │ │ Sprite  │         │
│  │ Collider│ │         │ │Animation│         │
│  └────────┘  └────────┘  └────────┘         │
└──────────────────────────────────────────────┘
                   ↓
        ┌──────────────────────┐
        │      Systems         │
        │  ┌────────────────┐  │
        │  │ RenderSystem   │──┼──→ Query: Transform + Sprite
        │  │ AnimationSystem│──┼──→ Query: Sprite + Animation
        │  │ PhysicsSystem  │──┼──→ Query: Transform + Collider
        │  └────────────────┘  │
        └──────────────────────┘
```

### Flow de création d'entité

```
JSON Definition          JSON Scene             Code
     ↓                       ↓                   ↓
┌─────────────┐      ┌──────────────┐    ┌───────────┐
│ Sprites.json│      │ test.json    │    │ Game code │
│   "player": │      │   type: "player"│  │ or Scene  │
│     texture │      │   position   │    │           │
│     size    │      │   ...        │    │           │
└─────────────┘      └──────────────┘    └───────────┘
        │                    │                  │
        └────────────────────┼──────────────────┘
                             ↓
                  ┌────────────────────┐
                  │ DefinitionManager  │
                  │  + SceneLoader     │
                  └────────────────────┘
                             ↓
                  ┌────────────────────┐
                  │  EntityRegistry    │
                  │  createEntity()    │
                  └────────────────────┘
                             ↓
                  ┌────────────────────┐
                  │   Entity created   │
                  │   with components  │
                  └────────────────────┘
```

---

**Prochaine section** : [Systèmes ECS](05-SYSTEMS.md)
