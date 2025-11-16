# Documentation Complète - NovaEngine ECS

## Table des Matières

1. [Vue d'ensemble](#vue-densemble)
2. [Architecture ECS](#architecture-ecs)
3. [Composants (Components)](#composants-components)
4. [Systèmes (Systems)](#systèmes-systems)
5. [Scènes (Scenes)](#scènes-scenes)
6. [Pathfinding avec Waypoints](#pathfinding-avec-waypoints)
7. [Système de Journey Multi-Scènes](#système-de-journey-multi-scènes)
8. [Formats JSON](#formats-json)
9. [Guide d'Intégration](#guide-dintégration)
10. [Exemples Complets](#exemples-complets)

---

## Vue d'ensemble

### Qu'est-ce que NovaEngine ?

NovaEngine est un moteur de jeu 2D modulaire basé sur C++17 utilisant SFML comme backend graphique. Il implémente une architecture **Entity Component System (ECS)** complète pour la gestion d'entités, avec un système de scènes et de pathfinding avancé.

### Philosophie de Design

- **Composition over Inheritance** : Les entités sont des conteneurs, la logique est dans les systèmes
- **Data-Driven** : Configuration via JSON, séparation code/données
- **Modulaire** : Chaque système est indépendant et réutilisable
- **Performance** : Optimisé pour gérer des centaines d'entités simultanément

### Architecture Globale

```
NovaEngine/
├── Backend/          # Abstraction SFML (Graphics, Audio, Input, Resources)
├── Core/             # Logger, ConfigManager, Types de base
├── ECS/              # Entity Component System complet
│   ├── Component.hpp
│   ├── Components.hpp      # Tous les composants built-in
│   ├── Entity.hpp
│   ├── EntityRegistry.hpp
│   ├── System.hpp
│   ├── Systems.hpp         # Tous les systèmes built-in
│   ├── Scene.hpp
│   ├── SceneManager.hpp
│   ├── SceneGraph.hpp
│   ├── WaypointGraph.hpp
│   └── DefinitionManager.hpp
├── Resources/        # ResourceManager pour textures, sons, fonts
└── UI/               # Système UI (Buttons, Panels, Text, etc.)
```

---

## Architecture ECS

### Concepts Fondamentaux

#### 1. **Entity (Entité)**

Une entité est simplement un **conteneur d'ID** avec une collection de composants.

```cpp
class Entity {
private:
    u64 m_id;  // Identifiant unique
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
    // Ajouter un composant
    template<typename T>
    T* addComponent(std::unique_ptr<T> component);

    // Récupérer un composant
    template<typename T>
    T* getComponent();

    // Vérifier la présence d'un composant
    template<typename T>
    bool hasComponent() const;

    // Supprimer un composant
    template<typename T>
    void removeComponent();
};
```

**Exemple d'utilisation** :
```cpp
Entity* player = registry.createEntity();
player->addComponent(std::make_unique<TransformComponent>(Vec2f{100, 200}));
player->addComponent(std::make_unique<SpriteComponent>());

auto* transform = player->getComponent<TransformComponent>();
transform->position.x += 10.0f;
```

#### 2. **Component (Composant)**

Un composant est **pure data**, sans logique. Il stocke les données d'un aspect d'une entité.

```cpp
class Component {
public:
    virtual ~Component() = default;
    virtual ComponentTypeID getTypeID() const = 0;
    virtual void serialize(nlohmann::json& json) const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
};
```

**Macro helper** pour définir le type ID :
```cpp
#define COMPONENT_TYPE_ID(ClassName) \
    ComponentTypeID getTypeID() const override { return #ClassName; }
```

#### 3. **System (Système)**

Un système contient la **logique** et opère sur les entités ayant certains composants.

```cpp
class System {
public:
    virtual ~System() = default;
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;
};
```

**Workflow typique** :
1. Le système demande au registry les entités avec certains composants
2. Le système itère sur ces entités
3. Le système lit/modifie les composants
4. Le système applique la logique métier

#### 4. **EntityRegistry**

Le registry gère toutes les entités et fournit des queries efficaces.

```cpp
class EntityRegistry {
public:
    // Création/Destruction
    Entity* createEntity();
    void destroyEntity(u64 entityID);
    Entity* getEntity(u64 entityID);

    // Queries
    std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& componentTypes);
    size_t getEntityCount() const;

    // Itération
    const std::unordered_map<u64, std::unique_ptr<Entity>>& getAllEntities() const;
};
```

**Exemple de query** :
```cpp
// Récupérer toutes les entités avec Transform + Sprite
auto renderables = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});

for (Entity* entity : renderables) {
    auto* transform = entity->getComponent<TransformComponent>();
    auto* sprite = entity->getComponent<SpriteComponent>();
    // ... render
}
```

---

## Composants (Components)

### TransformComponent

Gère la **position, rotation, échelle** d'une entité.

```cpp
class TransformComponent : public Component {
public:
    Vec2f position = {0.0f, 0.0f};
    f32 rotation = 0.0f;          // En degrés
    Vec2f scale = {1.0f, 1.0f};
    Vec2f origin = {0.0f, 0.0f};  // Point d'origine pour rotation
};
```

**Usage** :
```cpp
auto* transform = entity->addComponent(std::make_unique<TransformComponent>());
transform->position = Vec2f{640.0f, 360.0f};
transform->rotation = 45.0f;
transform->scale = Vec2f{2.0f, 2.0f};
```

### SpriteComponent

Affiche une **texture** à l'écran.

```cpp
class SpriteComponent : public Component {
public:
    ID textureID;                 // ID de la texture (ex: "player_idle")
    TextureHandle textureHandle;  // Handle SFML (chargé depuis ResourceManager)
    IntRect textureRect;          // Rectangle dans la texture (pour spritesheet)
    Vec2f size;                   // Taille d'affichage
    Color tint = Color::White;    // Teinte
    i32 zOrder = 0;               // Ordre d'affichage (plus grand = devant)
    bool visible = true;
};
```

**Chargement depuis JSON** :
```json
{
  "type": "sprite",
  "spriteID": "player",
  "position": [100, 200],
  "scale": [2, 2],
  "zOrder": 10
}
```

### LightComponent

Crée des **sources de lumière** (Point, Directional, Spot).

```cpp
class LightComponent : public Component {
public:
    enum class Type { Point, Directional, Spot };

    Type type = Type::Point;
    Color color = Color::White;
    f32 radius = 100.0f;        // Pour Point/Spot
    f32 intensity = 1.0f;
    Vec2f direction = {1, 0};   // Pour Directional/Spot
    f32 angle = 45.0f;          // Pour Spot (en degrés)
};
```

### AnimationComponent

Gère les **animations frame-by-frame**.

```cpp
class AnimationComponent : public Component {
public:
    std::vector<IntRect> frames;  // Liste des rectangles dans la texture
    f32 frameDuration = 0.1f;     // Durée d'une frame en secondes
    int currentFrame = 0;
    f32 elapsedTime = 0.0f;
    bool loop = true;
    bool playing = true;
};
```

**Exemple d'animation** :
```json
{
  "id": "player_walk_down",
  "frames": [[0,0,32,32], [32,0,32,32], [64,0,32,32], [96,0,32,32]],
  "frameDuration": 0.15,
  "loop": true
}
```

### ColliderComponent

Détection de **collisions** (Box ou Circle).

```cpp
class ColliderComponent : public Component {
public:
    enum class Type { Box, Circle };

    Type type = Type::Box;
    Vec2f size = {32, 32};     // Pour Box
    f32 radius = 16.0f;        // Pour Circle
    Vec2f offset = {0, 0};     // Décalage par rapport à Transform
    bool isTrigger = false;    // Trigger = pas de collision physique
};
```

### AudioComponent

Joue des **sons**.

```cpp
class AudioComponent : public Component {
public:
    ID soundID;
    SoundHandle soundHandle;
    bool playOnStart = false;
    bool loop = false;
    f32 volume = 100.0f;
    f32 pitch = 1.0f;
};
```

### ActivatorComponent

Crée des **zones d'activation** (pressure plates, triggers).

```cpp
class ActivatorComponent : public Component {
public:
    enum class Type { Proximity, Manual, Automatic };
    enum class Shape { Box, Circle };

    Type type = Type::Proximity;
    Shape shape = Shape::Box;
    Vec2f size = {64, 64};           // Pour Box
    f32 radius = 50.0f;              // Pour Circle

    std::string targetTag;           // Tag des entités à détecter (ex: "player")
    std::string onActivateEvent;     // Événement à déclencher
    std::string onDeactivateEvent;

    bool canReactivate = false;      // Peut se réactiver après cooldown
    f32 cooldownTime = 1.0f;
    bool isActive = false;
};
```

### TagComponent

**Marque** une entité avec un tag simple (pour ciblage).

```cpp
class TagComponent : public Component {
public:
    std::string tag;  // Ex: "player", "enemy", "npc"
};
```

### SceneTransitionComponent

Gère les **transitions entre scènes**.

```cpp
class SceneTransitionComponent : public Component {
public:
    std::string targetScene;     // Scène de destination
    Vec2f targetPosition;        // Position dans la scène cible
    bool isTransitioning = false;
};
```

### JourneyComponent

Gère les **voyages multi-scènes** des NPCs.

```cpp
class JourneyComponent : public Component {
public:
    // Voyage entre scènes
    std::vector<std::string> scenePath;    // ["maison_bob", "ville", "magasin"]
    int currentSceneIndex = 0;

    Vec2f currentDestination;              // Destination dans la scène actuelle
    bool reachedCurrentDestination = false;

    // Waypoints locaux (pathfinding dans la scène)
    std::vector<Vec2f> localWaypointPath;  // Chemin de waypoints à suivre
    int currentLocalWaypointIndex = 0;

    // Personnalité (pour choix de chemins)
    std::vector<std::string> preferredPathTags;  // ["shortcut"], ["main_road"], ["scenic"]

    // État du voyage
    bool isOnJourney = false;
    std::string finalDestinationScene;
    Vec2f finalDestinationPos;
};
```

---

## Systèmes (Systems)

### RenderSystem

**Rend toutes les entités** avec Sprite + Transform.

**Fonctionnalités** :
- Tri par z-order (entités avec z-order plus élevé sont devant)
- Support de rotation, échelle, origine
- Teinte de couleur
- Visibility flag

```cpp
class RenderSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SpriteComponent"};
    }
};
```

**Ordre de rendu** :
1. Collecte toutes les entités avec Transform + Sprite
2. Trie par `sprite->zOrder` (ascendant)
3. Pour chaque entité visible :
   - Configure la transformation SFML
   - Dessine la texture

### AnimationSystem

**Met à jour les animations** frame-by-frame.

**Fonctionnalités** :
- Avance automatiquement les frames selon `frameDuration`
- Support loop/no-loop
- Mise à jour du `SpriteComponent.textureRect` automatique

```cpp
class AnimationSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"SpriteComponent", "AnimationComponent"};
    }
};
```

**Workflow** :
1. `elapsedTime += deltaTime`
2. Si `elapsedTime >= frameDuration` :
   - Passe à la frame suivante
   - Si dernière frame et loop : retour à 0
   - Sinon : stop animation
3. Met à jour `sprite->textureRect` avec la frame actuelle

### LightSystem

**Rend les lumières** (cercles colorés pour l'instant, peut être étendu).

```cpp
class LightSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "LightComponent"};
    }
};
```

### AudioSystem

**Gère la lecture audio**.

**Fonctionnalités** :
- `playOnStart` : Joue automatiquement au démarrage
- Support loop
- Volume et pitch configurables

```cpp
class AudioSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"AudioComponent"};
    }
};
```

### PhysicsSystem

**Détection de collisions** simple (AABB et Circle).

**Fonctionnalités** :
- Collision Box-Box
- Collision Circle-Circle
- Collision Box-Circle
- Support des triggers (isTrigger = true)

```cpp
class PhysicsSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ColliderComponent"};
    }
};
```

### ActivatorSystem

**Gère les zones d'activation**.

**Fonctionnalités** :
- Détection Proximity (quand une entité avec le bon tag entre dans la zone)
- Cooldown après activation
- Événements onActivate / onDeactivate
- Debug : affiche les zones en mode debug

```cpp
class ActivatorSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override;

    // Activation/Désactivation manuelles
    void activate(Entity* entity);
    void deactivate(Entity* entity);

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ActivatorComponent"};
    }
};
```

### JourneySystem

**Gère les voyages multi-scènes** des NPCs.

**Fonctionnalités critiques** :
- Pathfinding entre scènes via SceneGraph
- Pathfinding dans une scène via WaypointGraph
- Suivi de waypoints local
- Gestion des transitions de scènes
- Support des personnalités via `preferredPathTags`

```cpp
class JourneySystem : public System {
private:
    SceneGraph* m_sceneGraph;  // Pour pathfinding entre scènes

public:
    explicit JourneySystem(SceneGraph* sceneGraph);

    void update(float deltaTime, EntityRegistry& registry) override;

    // Démarrer un voyage
    bool startJourney(Entity* entity,
                     const std::string& currentScene,
                     const std::string& targetScene,
                     const Vec2f& targetPosition);

    // Calculer le chemin de waypoints local
    void calculateLocalWaypointPath(Entity* entity, Scene* currentScene);

    // Mettre à jour après transition de scène
    void updateTransferredEntities(EntityRegistry& registry);

    // Annuler un voyage
    void cancelJourney(Entity* entity);
};
```

**Workflow d'un voyage** :
1. `startJourney("maison_bob", "magasin")` trouvé le chemin : `["maison_bob", "ville", "magasin"]`
2. Définit `currentDestination` = portail de sortie vers "ville"
3. `calculateLocalWaypointPath()` calcule le chemin de waypoints dans "maison_bob"
4. L'entité suit les waypoints un par un
5. Quand portail atteint → transition vers "ville"
6. `updateTransferredEntities()` recalcule le chemin de waypoints dans "ville"
7. Répète jusqu'à la destination finale

---

## Scènes (Scenes)

### Scene

Une scène contient un **EntityRegistry** et une collection de **Systems**.

```cpp
class Scene {
private:
    std::string m_name;
    std::string m_type;  // "interior" ou "exterior"
    Color m_backgroundColor;

    EntityRegistry m_entityRegistry;
    std::vector<std::unique_ptr<System>> m_systems;
    WaypointGraph m_waypointGraph;  // Pour pathfinding

public:
    // Chargement depuis JSON
    bool loadFromJSON(const nlohmann::json& sceneData,
                     const DefinitionManager& defManager);

    // Update et Render
    void update(float deltaTime);
    void render();

    // Accès
    EntityRegistry& getEntityRegistry();
    WaypointGraph& getWaypointGraph();

    // Pathfinding helper
    std::vector<Vec2f> findPath(const Vec2f& start, const Vec2f& end,
                               const std::vector<std::string>& tags = {});
};
```

**Ordre des systèmes** (défini dans le constructeur) :
1. AnimationSystem
2. PhysicsSystem
3. ActivatorSystem
4. AudioSystem
5. LightSystem
6. RenderSystem (toujours en dernier)

### SceneManager

Gère **plusieurs scènes** et leurs états.

```cpp
class SceneManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;  // Scène actuellement rendue
    DefinitionManager m_definitionManager;
    SceneGraph m_sceneGraph;

public:
    // Initialisation
    bool initialize(const std::string& definitionsPath,
                   const std::string& sceneGraphPath);

    // Gestion des scènes
    bool loadScene(const std::string& sceneName, const std::string& scenePath);
    void setActiveScene(const std::string& sceneName);
    Scene* getScene(const std::string& sceneName);

    // Update et Render
    void update(float deltaTime);
    void render();

    // Accès
    SceneGraph& getSceneGraph();
};
```

**Logique d'update** :
- Scène **rendue** (où est le joueur) : Update + Render
- Scènes **actives** (NPCs en transit) : Update seulement
- Scènes **dormantes** : Aucun update

```cpp
void SceneManager::update(float deltaTime) {
    // 1. Collecter les scènes avec NPCs en voyage
    std::unordered_set<std::string> scenesOnActivePaths;
    for (auto& [name, scene] : m_scenes) {
        auto travelers = scene->getEntityRegistry().getEntitiesWith({"JourneyComponent"});
        for (Entity* entity : travelers) {
            auto* journey = entity->getComponent<JourneyComponent>();
            if (journey && journey->isOnJourney) {
                for (const auto& sceneName : journey->scenePath) {
                    scenesOnActivePaths.insert(sceneName);
                }
            }
        }
    }

    // 2. Update les scènes appropriées
    for (auto& [name, scene] : m_scenes) {
        bool isRendered = (scene.get() == m_activeScene);
        bool isOnPath = (scenesOnActivePaths.find(name) != scenesOnActivePaths.end());

        if (isRendered || isOnPath) {
            scene->update(deltaTime);
        }
    }
}
```

### SceneGraph

Représente les **connexions entre scènes** pour le pathfinding.

```cpp
struct SceneConnection {
    std::string fromScene;
    std::string toScene;
    Vec2f exitPortalPos;    // Position du portail de sortie dans fromScene
    Vec2f entryPortalPos;   // Position d'entrée dans toScene
    f32 travelTime;
    bool bidirectional;
};

class SceneGraph {
private:
    std::vector<SceneConnection> m_connections;

public:
    // Chargement depuis JSON
    bool loadFromJSON(const nlohmann::json& json);

    // Pathfinding (BFS)
    std::vector<std::string> findPath(const std::string& startScene,
                                     const std::string& endScene) const;

    // Récupérer une connexion
    const SceneConnection* getConnection(const std::string& from,
                                        const std::string& to) const;
};
```

**Algorithme BFS** :
1. Queue initialisée avec la scène de départ
2. Pour chaque scène visitée, explorer les voisins
3. Marquer les scènes visitées pour éviter les cycles
4. Reconstruire le chemin avec `cameFrom` map

---

## Pathfinding avec Waypoints

### WaypointGraph

Système de **pathfinding fixe** basé sur des waypoints pré-définis.

```cpp
struct Waypoint {
    std::string id;              // "fountain", "north_plaza", etc.
    Vec2f position;
    std::vector<std::string> tags;  // ["main_road"], ["shortcut"], ["scenic"]
};

struct WaypointConnection {
    std::string from;
    std::string to;
    f32 cost;                    // Distance ou temps
    std::vector<std::string> tags;
    bool bidirectional;
};

class WaypointGraph {
private:
    std::vector<Waypoint> m_waypoints;
    std::vector<WaypointConnection> m_connections;

public:
    // Chargement depuis JSON
    bool loadFromJSON(const nlohmann::json& json);

    // Recherche
    const Waypoint* findNearestWaypoint(const Vec2f& position,
                                       f32 maxDistance = -1.0f) const;
    const Waypoint* findWaypointByID(const std::string& id) const;

    // Pathfinding (BFS avec filtrage par tags)
    std::vector<Vec2f> findPath(const Vec2f& start, const Vec2f& end,
                               const std::vector<std::string>& preferredTags = {}) const;

    std::vector<std::string> findPathByID(const std::string& startID,
                                         const std::string& endID,
                                         const std::vector<std::string>& tags = {}) const;
};
```

### Personnalités via Tags

Les NPCs peuvent avoir des `preferredPathTags` pour influencer leurs chemins :

**Exemple de waypoints** :
```json
{
  "waypoints": [
    {"id": "fountain", "position": [640, 360], "tags": ["landmark", "center"]},
    {"id": "shortcut_ne", "position": [850, 250], "tags": ["shortcut"]},
    {"id": "scenic_garden", "position": [500, 360], "tags": ["scenic", "peaceful"]}
  ],
  "connections": [
    {"from": "fountain", "to": "shortcut_ne", "tags": ["shortcut"]},
    {"from": "fountain", "to": "scenic_garden", "tags": ["scenic"]}
  ]
}
```

**Résultat** :
- NPC avec `preferredPathTags: ["shortcut"]` → Prendra `shortcut_ne`
- NPC avec `preferredPathTags: ["scenic"]` → Prendra `scenic_garden`
- NPC sans tags → Prendra le chemin le plus court (BFS standard)

### Algorithme de Pathfinding avec Tags

```cpp
std::vector<std::string> WaypointGraph::findPathByID(
    const std::string& startID,
    const std::string& endID,
    const std::vector<std::string>& preferredTags) const {

    // BFS classique
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

        // Explorer les connexions
        for (const auto& conn : m_connections) {
            std::string neighbor;

            if (conn.from == current) {
                neighbor = conn.to;
            } else if (conn.bidirectional && conn.to == current) {
                neighbor = conn.from;
            } else {
                continue;
            }

            // FILTRAGE PAR TAGS
            if (!preferredTags.empty() && !hasPreferredTag(conn, preferredTags)) {
                continue;  // Ignorer cette connexion
            }

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    return {};  // Pas de chemin trouvé
}
```

**Comportement** :
- Si `preferredTags` est vide → BFS normal, chemin le plus court
- Si `preferredTags` contient des tags → Ne considère QUE les connexions avec ces tags
- Si aucun chemin trouvé avec les tags → Retourne vide (fallback vers navigation directe)

---

## Système de Journey Multi-Scènes

### Problème Résolu

**Avant** : Les NPCs se téléportaient entre scènes
```
NPC à scene_3 doit aller à scene_1
→ TÉLÉPORTATION directe
→ Le joueur dans scene_2 ne voit RIEN
```

**Après** : Les NPCs traversent physiquement toutes les scènes
```
NPC à scene_3 doit aller à scene_1
→ scene_3 → scene_2 → scene_1 (physiquement)
→ Le joueur dans scene_2 VOIT le NPC passer
```

### Workflow Complet

#### 1. Démarrage du Voyage

```cpp
// Bob veut aller de "maison_bob" à "magasin"
journeySystem->startJourney(
    bobEntity,
    "maison_bob",    // Scène actuelle
    "magasin",       // Scène destination
    Vec2f{300, 250}  // Position finale dans le magasin
);
```

**Ce qui se passe** :
1. SceneGraph trouve le chemin : `["maison_bob", "ville", "magasin"]`
2. JourneyComponent initialisé :
   ```cpp
   journey->scenePath = ["maison_bob", "ville", "magasin"];
   journey->currentSceneIndex = 0;
   journey->finalDestinationScene = "magasin";
   journey->finalDestinationPos = Vec2f{300, 250};
   journey->isOnJourney = true;
   ```
3. Première destination définie : portail vers "ville" à [400, 580]

#### 2. Calcul du Chemin de Waypoints Local

```cpp
journeySystem->calculateLocalWaypointPath(bobEntity, maisoBobScene);
```

**Ce qui se passe** :
1. Position actuelle de Bob : [200, 300]
2. Destination dans cette scène : [400, 580]
3. `preferredPathTags` de Bob : `["shortcut"]`
4. WaypointGraph calcule : `[[200, 300], [350, 400], [400, 580]]`
5. JourneyComponent mis à jour :
   ```cpp
   journey->localWaypointPath = [[200, 300], [350, 400], [400, 580]];
   journey->currentLocalWaypointIndex = 0;
   ```

#### 3. Suivi des Waypoints

**Dans JourneySystem::update()** :
```cpp
if (!journey->localWaypointPath.empty()) {
    Vec2f currentWaypoint = journey->localWaypointPath[journey->currentLocalWaypointIndex];
    Vec2f toWaypoint = currentWaypoint - transform->position;
    f32 distance = sqrt(toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y);

    if (distance < 5.0f) {
        // Waypoint atteint !
        journey->currentLocalWaypointIndex++;

        if (journey->currentLocalWaypointIndex >= journey->localWaypointPath.size()) {
            // Tous les waypoints atteints = destination de la scène atteinte
            journey->reachedCurrentDestination = true;
        }
    }
}
```

**Votre code de mouvement** doit lire `currentWaypoint` et déplacer le NPC vers ce point.

#### 4. Transition de Scène

Quand `reachedCurrentDestination = true` :

```cpp
if (journey->currentSceneIndex + 1 < journey->scenePath.size()) {
    // Il y a une prochaine scène
    std::string nextScene = journey->scenePath[journey->currentSceneIndex + 1];

    // Préparer la transition
    transition->targetScene = nextScene;
    transition->targetPosition = connection->entryPortalPos;  // Ex: [400, 580]
    transition->isTransitioning = true;

    // SceneManager va détecter ceci et transférer l'entité
}
```

#### 5. Après la Transition

**Dans SceneManager** (après transfert) :
```cpp
journeySystem->updateTransferredEntities(registry);
```

**Ce qui se passe** :
1. `journey->currentSceneIndex++`  (maintenant à 1 = "ville")
2. Nouvelle destination définie : portail vers "magasin" à [600, 200]
3. `calculateLocalWaypointPath()` appelée automatiquement
4. Nouveau chemin de waypoints calculé dans "ville" avec les tags de Bob

**Bob traverse la ville** :
```
Position: [250, 450] (porte de sa maison)
Waypoints: [250, 450] → [400, 500] → [640, 360] → [850, 250] → [600, 200]
           (maison)      (shortcut)   (fountain)   (shortcut)   (magasin)
```

#### 6. Destination Finale

Quand Bob atteint [600, 200] dans "ville" :
- Transition vers "magasin"
- Position dans magasin : [400, 580]
- Dernière destination : [300, 250]
- Pas de waypoint graph dans le magasin → navigation directe
- Bob arrive à [300, 250]
- `journey->isOnJourney = false` → **FIN DU VOYAGE**

### Visualisation Complète

```
╔════════════════════════════════════════════════════════════════╗
║                    VOYAGE DE BOB (7h00)                        ║
╠════════════════════════════════════════════════════════════════╣
║                                                                ║
║  [MAISON_BOB]                                                 ║
║      Bob [200,300] → (waypoint) → [400,580] (porte)          ║
║                           ↓                                    ║
║  ┌──────────────── TRANSITION ─────────────────┐              ║
║                           ↓                                    ║
║  [VILLE] ⭐ LE JOUEUR VOIT BOB ICI ⭐                          ║
║      Bob apparaît à [250,450] (porte maison dans ville)      ║
║      Waypoints (avec preferredTags: ["shortcut"]):           ║
║        [250,450] → [400,500] (shortcut_sw)                    ║
║                 → [640,360] (fountain)                        ║
║                 → [850,250] (shortcut_ne)                     ║
║                 → [600,200] (porte magasin)                   ║
║                           ↓                                    ║
║  ┌──────────────── TRANSITION ─────────────────┐              ║
║                           ↓                                    ║
║  [MAGASIN]                                                    ║
║      Bob apparaît à [400,580] (entrée)                       ║
║      Direct vers [300,250] (pas de waypoints)                ║
║      ✅ ARRIVÉE !                                             ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
```

---

## Formats JSON

### 1. Définitions (Definitions)

Chargées **une fois au démarrage** par DefinitionManager.

#### Sprites.json
```json
{
  "sprites": [
    {
      "id": "player",
      "texture": "character_sheet",
      "textureRect": [0, 0, 32, 32],
      "size": [32, 32],
      "tint": [255, 255, 255, 255],
      "zOrder": 10
    },
    {
      "id": "tree",
      "texture": "environment",
      "textureRect": [64, 0, 64, 96],
      "size": [64, 96],
      "zOrder": 5
    }
  ]
}
```

#### Lights.json
```json
{
  "lights": [
    {
      "id": "torch",
      "type": "point",
      "color": [255, 200, 100, 255],
      "radius": 150.0,
      "intensity": 1.0
    },
    {
      "id": "sunlight",
      "type": "directional",
      "color": [255, 255, 200, 128],
      "direction": [1, 1],
      "intensity": 0.8
    }
  ]
}
```

#### Animations.json
```json
{
  "animations": [
    {
      "id": "player_walk_down",
      "frames": [
        [0, 0, 32, 32],
        [32, 0, 32, 32],
        [64, 0, 32, 32],
        [96, 0, 32, 32]
      ],
      "frameDuration": 0.15,
      "loop": true
    }
  ]
}
```

#### Audio.json
```json
{
  "sounds": [
    {
      "id": "footstep",
      "file": "sounds/footstep.wav",
      "loop": false,
      "volume": 80.0,
      "pitch": 1.0
    },
    {
      "id": "ambiance_forest",
      "file": "sounds/forest_ambiance.ogg",
      "loop": true,
      "volume": 50.0
    }
  ]
}
```

#### Activators.json
```json
{
  "activators": [
    {
      "id": "pressure_plate",
      "type": "proximity",
      "shape": "box",
      "size": [80, 80],
      "targetTag": "player",
      "canReactivate": true,
      "cooldownTime": 0.5,
      "onActivateEvent": "door_open",
      "onDeactivateEvent": "door_close"
    },
    {
      "id": "trigger_zone",
      "type": "proximity",
      "shape": "circle",
      "radius": 150.0,
      "targetTag": "player",
      "canReactivate": false,
      "onActivateEvent": "cutscene_start"
    }
  ]
}
```

#### NPCs.json
```json
{
  "npcs": [
    {
      "id": "merchant_bob",
      "name": "Bob le Marchand",
      "sprite": "npc_merchant",
      "homeScene": "maison_bob",
      "homePosition": [200, 300],
      "walkSpeed": 100.0,
      "preferredPathTags": ["shortcut"],
      "personality": "Pressé et efficace, Bob prend toujours les raccourcis",
      "dailySchedule": [
        {
          "startTime": 7.0,
          "activity": "travel",
          "targetScene": "magasin",
          "targetPosition": [300, 250],
          "description": "Aller au magasin"
        },
        {
          "startTime": 8.0,
          "endTime": 18.0,
          "activity": "work",
          "targetScene": "magasin",
          "targetPosition": [300, 250],
          "description": "Travailler"
        }
      ]
    },
    {
      "id": "guard_alice",
      "name": "Alice la Garde",
      "sprite": "npc_guard",
      "homeScene": "caserne",
      "homePosition": [150, 200],
      "walkSpeed": 80.0,
      "preferredPathTags": ["main_road"],
      "personality": "Disciplinée et vigilante, Alice patrouille les routes principales",
      "dailySchedule": [...]
    }
  ]
}
```

### 2. Scènes (Scenes)

Chargées **à la demande** par SceneManager.

#### ville.json (avec waypoints)
```json
{
  "name": "Ville",
  "type": "exterior",
  "backgroundColor": [120, 150, 180, 255],
  "pathfinding": {
    "waypoints": [
      {"id": "fountain", "position": [640, 360], "tags": ["landmark", "center", "main_road"]},
      {"id": "north_plaza", "position": [640, 200], "tags": ["main_road", "plaza"]},
      {"id": "maison_bob_door", "position": [250, 450], "tags": ["residential"]},
      {"id": "magasin_door", "position": [600, 200], "tags": ["commercial"]},
      {"id": "shortcut_ne", "position": [850, 250], "tags": ["shortcut"]},
      {"id": "scenic_garden", "position": [500, 360], "tags": ["scenic", "peaceful"]}
    ],
    "connections": [
      {"from": "fountain", "to": "north_plaza", "tags": ["main_road"], "bidirectional": true},
      {"from": "fountain", "to": "shortcut_ne", "tags": ["shortcut"], "bidirectional": true},
      {"from": "fountain", "to": "scenic_garden", "tags": ["scenic"], "bidirectional": true},
      {"from": "north_plaza", "to": "magasin_door", "tags": ["main_road"], "bidirectional": true},
      {"from": "shortcut_ne", "to": "magasin_door", "tags": ["shortcut"], "bidirectional": true}
    ]
  },
  "entities": [
    {
      "type": "sprite",
      "spriteID": "ground_tile",
      "position": [0, 0],
      "scale": [20, 15],
      "zOrder": -10
    },
    {
      "type": "light",
      "lightID": "torch",
      "position": [640, 360]
    },
    {
      "type": "activator",
      "activatorID": "trigger_zone",
      "position": [250, 450]
    }
  ]
}
```

#### maison_bob.json (sans waypoints)
```json
{
  "name": "Maison de Bob",
  "type": "interior",
  "backgroundColor": [80, 70, 60, 255],
  "entities": [
    {
      "type": "sprite",
      "spriteID": "floor_wood",
      "position": [0, 0],
      "scale": [10, 8],
      "zOrder": -10
    },
    {
      "type": "sprite",
      "spriteID": "door_interior",
      "position": [400, 580],
      "zOrder": 5,
      "comment": "Porte de sortie"
    },
    {
      "type": "sprite",
      "spriteID": "bed",
      "position": [200, 300],
      "zOrder": 1
    },
    {
      "type": "light",
      "lightID": "candle",
      "position": [500, 350]
    }
  ]
}
```

### 3. SceneGraph

Définit les **connexions entre scènes**.

#### scenegraph.json
```json
{
  "connections": [
    {
      "from": "ville",
      "to": "maison_bob",
      "exitPortal": [250, 450],
      "entryPortal": [400, 580],
      "travelTime": 1.0,
      "bidirectional": true
    },
    {
      "from": "ville",
      "to": "taverne",
      "exitPortal": [800, 300],
      "entryPortal": [400, 580],
      "travelTime": 1.0,
      "bidirectional": true
    },
    {
      "from": "ville",
      "to": "magasin",
      "exitPortal": [600, 200],
      "entryPortal": [400, 580],
      "travelTime": 1.0,
      "bidirectional": true
    }
  ]
}
```

---

## Guide d'Intégration

### Étape 1 : Initialisation

```cpp
// Dans votre classe Game
class Game : public NovaEngine::Application {
private:
    NovaEngine::SceneManager m_sceneManager;

public:
    void onInitialize() override {
        // 1. Initialiser le SceneManager
        if (!m_sceneManager.initialize(
            "assets/data/definitions",     // Dossier avec tous les .json de définitions
            "assets/data/scenegraph.json"  // Fichier de connexions entre scènes
        )) {
            LOG_ERROR("Failed to initialize SceneManager");
            return;
        }

        // 2. Charger les scènes
        m_sceneManager.loadScene("ville", "assets/data/scenes/ville.json");
        m_sceneManager.loadScene("maison_bob", "assets/data/scenes/maison_bob.json");
        m_sceneManager.loadScene("magasin", "assets/data/scenes/magasin.json");
        m_sceneManager.loadScene("taverne", "assets/data/scenes/taverne.json");

        // 3. Définir la scène active
        m_sceneManager.setActiveScene("ville");

        LOG_INFO("Game initialized successfully");
    }
};
```

### Étape 2 : Update et Render

```cpp
void Game::onUpdate(float deltaTime) {
    // Met à jour toutes les scènes actives
    m_sceneManager.update(deltaTime);
}

void Game::onRender() {
    // Rend la scène active
    m_sceneManager.render();
}

void Game::onShutdown() {
    // Cleanup automatique
}
```

### Étape 3 : Créer des Entités Dynamiquement

```cpp
void createPlayer(Scene* scene, const Vec2f& position) {
    EntityRegistry& registry = scene->getEntityRegistry();

    Entity* player = registry.createEntity();

    // Transform
    auto* transform = player->addComponent(std::make_unique<TransformComponent>());
    transform->position = position;

    // Sprite
    auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
    sprite->textureID = "player";
    sprite->textureHandle = RESOURCES().getTextureHandle("character_sheet");
    sprite->textureRect = IntRect{0, 0, 32, 32};
    sprite->size = Vec2f{32, 32};
    sprite->zOrder = 10;

    // Tag
    auto* tag = player->addComponent(std::make_unique<TagComponent>());
    tag->tag = "player";

    // Collider
    auto* collider = player->addComponent(std::make_unique<ColliderComponent>());
    collider->type = ColliderComponent::Type::Box;
    collider->size = Vec2f{28, 28};

    LOG_INFO("Player created at ({}, {})", position.x, position.y);
}
```

### Étape 4 : Démarrer un Voyage NPC

```cpp
void scheduleNPCJourney(SceneManager& sceneManager, u64 npcID) {
    // 1. Trouver le NPC
    Scene* currentScene = sceneManager.getScene("maison_bob");
    Entity* npc = currentScene->getEntityRegistry().getEntity(npcID);

    if (!npc) {
        LOG_ERROR("NPC {} not found", npcID);
        return;
    }

    // 2. Ajouter JourneyComponent si pas déjà présent
    auto* journey = npc->getComponent<JourneyComponent>();
    if (!journey) {
        journey = npc->addComponent(std::make_unique<JourneyComponent>());
        journey->preferredPathTags = {"shortcut"};  // Personnalité
    }

    // 3. Ajouter SceneTransitionComponent si pas déjà présent
    if (!npc->hasComponent<SceneTransitionComponent>()) {
        npc->addComponent(std::make_unique<SceneTransitionComponent>());
    }

    // 4. Récupérer le JourneySystem de la scène
    // (vous devrez stocker une référence ou le récupérer autrement)
    JourneySystem* journeySystem = /* ... */;

    // 5. Démarrer le voyage
    if (journeySystem->startJourney(
        npc,
        "maison_bob",     // Scène actuelle
        "magasin",        // Scène destination
        Vec2f{300, 250}   // Position finale
    )) {
        LOG_INFO("NPC {} journey started", npcID);

        // 6. Calculer le chemin de waypoints local
        journeySystem->calculateLocalWaypointPath(npc, currentScene);
    }
}
```

### Étape 5 : Implémenter le Mouvement NPC

Vous devez créer un système de mouvement qui lit les waypoints :

```cpp
class NPCMovementSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto npcs = registry.getEntitiesWith({
            "TransformComponent",
            "JourneyComponent"
        });

        for (Entity* npc : npcs) {
            auto* transform = npc->getComponent<TransformComponent>();
            auto* journey = npc->getComponent<JourneyComponent>();

            if (!journey->isOnJourney) continue;

            // Récupérer le waypoint actuel
            Vec2f target;
            if (!journey->localWaypointPath.empty() &&
                journey->currentLocalWaypointIndex < journey->localWaypointPath.size()) {
                target = journey->localWaypointPath[journey->currentLocalWaypointIndex];
            } else {
                target = journey->currentDestination;
            }

            // Calculer la direction
            Vec2f direction = target - transform->position;
            f32 distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 1.0f) {
                // Normaliser et déplacer
                direction.x /= distance;
                direction.y /= distance;

                f32 speed = 100.0f;  // Ou charger depuis NPCs.json
                transform->position.x += direction.x * speed * deltaTime;
                transform->position.y += direction.y * speed * deltaTime;
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "JourneyComponent"};
    }
};
```

### Étape 6 : Gérer les Transitions de Scènes

Dans SceneManager, ajouter cette logique :

```cpp
void SceneManager::handleSceneTransitions() {
    std::vector<TransferData> transfers;

    // 1. Collecter toutes les entités en transition
    for (auto& [sceneName, scene] : m_scenes) {
        auto transitioning = scene->getEntityRegistry().getEntitiesWith({
            "SceneTransitionComponent"
        });

        for (Entity* entity : transitioning) {
            auto* transition = entity->getComponent<SceneTransitionComponent>();

            if (transition->isTransitioning) {
                TransferData transfer;
                transfer.entityID = entity->getID();
                transfer.fromScene = sceneName;
                transfer.toScene = transition->targetScene;
                transfer.targetPosition = transition->targetPosition;
                transfers.push_back(transfer);

                transition->isTransitioning = false;
            }
        }
    }

    // 2. Effectuer les transferts
    for (const auto& transfer : transfers) {
        Scene* fromScene = getScene(transfer.fromScene);
        Scene* toScene = getScene(transfer.toScene);

        if (!fromScene || !toScene) continue;

        // Récupérer l'entité
        Entity* entity = fromScene->getEntityRegistry().getEntity(transfer.entityID);
        if (!entity) continue;

        // Sérialiser l'entité
        nlohmann::json entityData;
        // ... sérialisation complète de tous les composants ...

        // Créer dans la nouvelle scène
        Entity* newEntity = toScene->getEntityRegistry().createEntity();
        // ... désérialiser tous les composants ...

        // Mettre à jour la position
        auto* transform = newEntity->getComponent<TransformComponent>();
        if (transform) {
            transform->position = transfer.targetPosition;
        }

        // Supprimer de l'ancienne scène
        fromScene->getEntityRegistry().destroyEntity(transfer.entityID);

        LOG_INFO("Entity {} transferred from '{}' to '{}'",
                transfer.entityID, transfer.fromScene, transfer.toScene);
    }

    // 3. Mettre à jour les JourneyComponents après transfert
    // (appeler journeySystem->updateTransferredEntities)
}
```

---

## Exemples Complets

### Exemple 1 : NPC avec Emploi du Temps

```cpp
// Système de schedule (à implémenter)
class ScheduleSystem : public System {
private:
    f32 m_currentTime = 7.0f;  // Heure actuelle (7h00 du matin)
    std::unordered_map<std::string, nlohmann::json> m_npcSchedules;  // Chargé depuis NPCs.json

public:
    void update(float deltaTime, EntityRegistry& registry) override {
        m_currentTime += deltaTime / 3600.0f;  // Convertir en heures

        for (auto& [npcID, schedule] : m_npcSchedules) {
            // Trouver l'activité actuelle
            for (const auto& activity : schedule["dailySchedule"]) {
                f32 startTime = activity["startTime"];
                f32 endTime = activity.value("endTime", startTime + 1.0f);

                if (m_currentTime >= startTime && m_currentTime < endTime) {
                    std::string activityType = activity["activity"];

                    if (activityType == "travel") {
                        // Démarrer un voyage
                        Entity* npc = registry.getEntity(/* ... */);

                        // Vérifier si déjà en voyage
                        auto* journey = npc->getComponent<JourneyComponent>();
                        if (!journey->isOnJourney) {
                            std::string targetScene = activity["targetScene"];
                            Vec2f targetPos = {
                                activity["targetPosition"][0],
                                activity["targetPosition"][1]
                            };

                            // Démarrer le voyage
                            m_journeySystem->startJourney(npc, currentScene, targetScene, targetPos);
                        }
                    }
                }
            }
        }
    }
};
```

### Exemple 2 : Interaction Joueur-NPC

```cpp
// Système d'interaction
class InteractionSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // Récupérer le joueur
        auto players = registry.getEntitiesWith({"TagComponent"});
        Entity* player = nullptr;
        for (Entity* e : players) {
            auto* tag = e->getComponent<TagComponent>();
            if (tag->tag == "player") {
                player = e;
                break;
            }
        }

        if (!player) return;

        auto* playerTransform = player->getComponent<TransformComponent>();

        // Si le joueur appuie sur E
        if (INPUT().isKeyPressed(Key::E)) {
            // Chercher les NPCs proches
            auto npcs = registry.getEntitiesWith({"TransformComponent", "JourneyComponent"});

            for (Entity* npc : npcs) {
                auto* npcTransform = npc->getComponent<TransformComponent>();

                // Calculer la distance
                Vec2f diff = npcTransform->position - playerTransform->position;
                f32 distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (distance < 50.0f) {
                    // NPC proche ! Ouvrir dialogue
                    LOG_INFO("Interaction avec NPC {}", npc->getID());

                    // Si le NPC est en voyage, l'arrêter
                    auto* journey = npc->getComponent<JourneyComponent>();
                    if (journey->isOnJourney) {
                        m_journeySystem->cancelJourney(npc);
                        LOG_INFO("NPC voyage annulé pour dialogue");
                    }

                    // Ouvrir UI de dialogue
                    // ...
                }
            }
        }
    }
};
```

### Exemple 3 : Debug Visuel des Waypoints

```cpp
class DebugWaypointSystem : public System {
private:
    Scene* m_scene;
    bool m_showWaypoints = false;

public:
    explicit DebugWaypointSystem(Scene* scene) : m_scene(scene) {}

    void update(float deltaTime, EntityRegistry& registry) override {
        // Toggle avec F3
        if (INPUT().isKeyJustPressed(Key::F3)) {
            m_showWaypoints = !m_showWaypoints;
        }

        if (!m_showWaypoints) return;

        const WaypointGraph& graph = m_scene->getWaypointGraph();

        // Dessiner tous les waypoints
        for (const auto& waypoint : graph.getWaypoints()) {
            // Cercle bleu pour le waypoint
            GRAPHICS().drawCircle(
                waypoint.position,
                5.0f,
                Color{0, 0, 255, 200}
            );

            // Texte avec l'ID
            GRAPHICS().drawText(
                waypoint.id,
                waypoint.position + Vec2f{10, 0},
                12,
                Color::White
            );
        }

        // Dessiner toutes les connexions
        for (const auto& conn : graph.getConnections()) {
            const Waypoint* from = graph.findWaypointByID(conn.from);
            const Waypoint* to = graph.findWaypointByID(conn.to);

            if (from && to) {
                Color lineColor = Color{255, 255, 0, 100};  // Jaune

                // Couleur selon le tag
                if (!conn.tags.empty()) {
                    if (conn.tags[0] == "shortcut") {
                        lineColor = Color{255, 0, 0, 100};  // Rouge
                    } else if (conn.tags[0] == "scenic") {
                        lineColor = Color{0, 255, 0, 100};  // Vert
                    }
                }

                GRAPHICS().drawLine(from->position, to->position, 2.0f, lineColor);
            }
        }

        // Dessiner les chemins des NPCs en voyage
        auto travelers = registry.getEntitiesWith({"JourneyComponent", "TransformComponent"});

        for (Entity* npc : travelers) {
            auto* journey = npc->getComponent<JourneyComponent>();
            auto* transform = npc->getComponent<TransformComponent>();

            if (!journey->isOnJourney || journey->localWaypointPath.empty()) continue;

            // Dessiner le chemin
            for (size_t i = 0; i < journey->localWaypointPath.size() - 1; ++i) {
                GRAPHICS().drawLine(
                    journey->localWaypointPath[i],
                    journey->localWaypointPath[i + 1],
                    3.0f,
                    Color{255, 0, 255, 200}  // Magenta
                );
            }

            // Dessiner le waypoint actuel
            if (journey->currentLocalWaypointIndex < journey->localWaypointPath.size()) {
                Vec2f currentWP = journey->localWaypointPath[journey->currentLocalWaypointIndex];
                GRAPHICS().drawCircle(currentWP, 8.0f, Color{255, 0, 255, 255});
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {};
    }
};
```

---

## Conclusion

### Points Forts du Système

✅ **Architecture ECS Pure** : Séparation claire Entity/Component/System
✅ **Data-Driven** : Tout configurable via JSON
✅ **Performance** : Queries optimisées, update sélectif des scènes
✅ **Pathfinding Avancé** : Deux niveaux (SceneGraph + WaypointGraph)
✅ **Personnalité** : NPCs avec comportements différents via tags
✅ **Multi-Scène** : NPCs vivants traversant physiquement le monde
✅ **Modulaire** : Facile d'ajouter de nouveaux composants/systèmes

### Ce qui reste à implémenter (par vous)

- [ ] **Système de mouvement NPC** : Lire les waypoints et déplacer les entités
- [ ] **Système de schedule** : Déclencher les activités selon l'heure
- [ ] **Gestion des transitions** : Transférer les entités entre scènes
- [ ] **Appel de calculateLocalWaypointPath** : Après chaque transition
- [ ] **Chargement des preferredPathTags** : Depuis NPCs.json
- [ ] **UI de debug** : Visualiser waypoints, chemins, etc.
- [ ] **Sauvegarde/Chargement** : Sérialiser l'état du monde

### Structure de Dossiers Recommandée

```
client/
└── assets/
    └── data/
        ├── definitions/
        │   ├── Sprites.json
        │   ├── Lights.json
        │   ├── Animations.json
        │   ├── Audio.json
        │   ├── Activators.json
        │   └── NPCs.json
        ├── scenes/
        │   ├── ville.json
        │   ├── maison_bob.json
        │   ├── taverne.json
        │   └── magasin.json
        └── scenegraph.json
```

### Workflow de Développement

1. **Définir les données** (JSON)
2. **Charger dans le moteur** (DefinitionManager, SceneManager)
3. **Tester visuellement** (Debug system, logs)
4. **Itérer** (Ajuster les positions, waypoints, connexions)
5. **Optimiser** (Profiling, réduction des scènes actives)

---

**Félicitations !** Vous disposez maintenant d'un moteur ECS complet avec pathfinding avancé et NPCs vivants ! 🎮✨

Pour toute question ou problème, consultez les logs (LOG_INFO, LOG_DEBUG, LOG_ERROR) qui fournissent des informations détaillées sur chaque opération.
