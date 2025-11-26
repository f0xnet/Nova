# Gestion des Scènes - NovaEngine

La gestion des scènes gère le chargement, l'organisation et les transitions entre différentes zones du jeu.

## Architecture

```
SceneManager
 ├── DefinitionManager (définitions réutilisables)
 ├── SceneGraph (connexions inter-scènes)
 └── Scenes
      ├── Scene 1 (EntityRegistry + Systems + WaypointGraph)
      ├── Scene 2
      └── Scene 3
```

## SceneManager

**Rôle** : Gestionnaire central de toutes les scènes

```cpp
class SceneManager {
public:
    // Initialisation (charge définitions + scene graph)
    bool initialize(const std::string& definitionsPath,
                   const std::string& sceneGraphPath);

    // Chargement scènes
    bool loadScene(const std::string& scenePath, const std::string& sceneName);
    void unloadScene(const std::string& sceneName);

    // Activation
    void setActiveScene(const std::string& sceneName);
    Scene* getActiveScene();

    // Update/Render
    void update(float deltaTime);
    void render();

    // Accès
    Scene* getScene(const std::string& name);
    DefinitionManager& getDefinitionManager();
    SceneGraph& getSceneGraph();

    void shutdown();
};
```

**Utilisation** :
```cpp
SceneManager sceneManager;

// 1. Init (charge définitions)
sceneManager.initialize("data/definitions/", "data/scenegraph.json");

// 2. Charger scènes
sceneManager.loadScene("data/scenes/ville.json", "ville");
sceneManager.loadScene("data/scenes/taverne.json", "taverne");

// 3. Activer scène
sceneManager.setActiveScene("ville");

// 4. Game loop
while (running) {
    sceneManager.update(deltaTime);
    sceneManager.render();
}
```

## DefinitionManager

**Rôle** : Charge et stocke définitions d'entités réutilisables

```cpp
class DefinitionManager {
public:
    bool loadDefinitions(const std::string& path);

    const nlohmann::json* getSpriteDefinition(const ID& id) const;
    const nlohmann::json* getLightDefinition(const ID& id) const;
    const nlohmann::json* getAnimationDefinition(const ID& id) const;
    const nlohmann::json* getAudioDefinition(const ID& id) const;
    const nlohmann::json* getNPCDefinition(const ID& id) const;
    const nlohmann::json* getActivatorDefinition(const ID& id) const;
};
```

**Fichiers chargés** :
- `Sprites.json` : Définitions de sprites
- `Lights.json` : Définitions de lumières
- `Animations.json` : Définitions d'animations
- `Audio.json` : Définitions de sons
- `NPCs.json` : Définitions de NPCs
- `Activators.json` : Définitions d'activateurs

**Format définition** (`Sprites.json`) :
```json
{
  "torch": {
    "texture": "assets/textures/torch.png",
    "size": [32, 64],
    "origin": [16, 32],
    "zOrder": 5
  },
  "player": {
    "texture": "assets/textures/player.png",
    "size": [64, 64],
    "origin": [32, 32],
    "zOrder": 10
  }
}
```

**Format scène** (`ville.json`) :
```json
{
  "name": "ville",
  "type": "exterior",
  "backgroundColor": [50, 50, 60, 255],
  "entities": [
    {
      "type": "sprite",
      "spriteID": "torch",
      "position": [100, 200]
    },
    {
      "type": "player",
      "spriteID": "player",
      "position": [400, 300]
    }
  ],
  "pathfinding": {
    "waypoints": [
      {"id": "wp1", "position": [100, 100], "tags": ["main_road"]},
      {"id": "wp2", "position": [200, 100], "tags": ["main_road"]},
      {"id": "wp3", "position": [300, 100], "tags": ["shortcut"]}
    ],
    "connections": [
      {"from": "wp1", "to": "wp2", "tags": ["main_road"]},
      {"from": "wp2", "to": "wp3", "tags": ["shortcut"]}
    ]
  }
}
```

## SceneGraph

**Rôle** : Pathfinding inter-scènes

```cpp
struct SceneConnection {
    std::string fromScene;
    std::string toScene;
    Vec2f exitPortalPos;   // Position portail sortie dans fromScene
    Vec2f entryPortalPos;  // Position entrée dans toScene
};

class SceneGraph {
public:
    bool loadFromJSON(const std::string& path);

    // Pathfinding
    std::vector<std::string> findPath(const std::string& start,
                                     const std::string& end) const;

    const SceneConnection* getConnection(const std::string& from,
                                        const std::string& to) const;
};
```

**Format** (`scenegraph.json`) :
```json
{
  "connections": [
    {
      "from": "ville",
      "to": "taverne",
      "exitPortal": [500, 300],
      "entryPortal": [50, 400]
    },
    {
      "from": "ville",
      "to": "maison_bob",
      "exitPortal": [800, 200],
      "entryPortal": [30, 350]
    }
  ]
}
```

## WaypointGraph

**Rôle** : Pathfinding intra-scène

```cpp
struct Waypoint {
    std::string id;
    Vec2f position;
    std::vector<std::string> tags;  // Ex: ["main_road"], ["scenic"]
};

struct WaypointConnection {
    std::string from;
    std::string to;
    std::vector<std::string> tags;
};

class WaypointGraph {
public:
    bool loadFromJSON(const nlohmann::json& data);

    // Pathfinding
    std::vector<Vec2f> findPath(const Vec2f& start,
                               const Vec2f& end,
                               const std::vector<std::string>& preferredTags = {}) const;

    const std::vector<Waypoint>& getWaypoints() const;
    bool isEmpty() const;
};
```

**Algorithme** : A* avec préférence de tags
- NPCs avec `preferredPathTags = ["main_road"]` privilégient chemins "main_road"
- NPCs avec `preferredPathTags = ["shortcut"]` privilégient raccourcis

## Transitions multi-scènes

**Exemple** : NPC voyage de "ville" à "maison_bob"

```cpp
// 1. Démarrer voyage
JourneySystem* journeySystem = scene->getSystem<JourneySystem>();
journeySystem->startJourney(npc, "ville", "maison_bob", Vec2f{100, 200});

// 2. JourneySystem calcule chemin
scenePath = ["ville", "maison_bob"]

// 3. Pour chaque scène :
//    - Calcule waypoint path local (WaypointGraph)
//    - NPC suit waypoints
//    - Atteint portail de sortie
//    - SceneManager transfère NPC vers scène suivante
//    - NPC apparaît au portail d'entrée
//    - Répète

// 4. Arrivée finale
NPC atteint position cible dans "maison_bob"
```

**Code SceneManager pour transitions** :
```cpp
void SceneManager::update(float deltaTime) {
    // Update toutes scènes actives
    for (const auto& sceneName : m_activeScenesForUpdate) {
        Scene* scene = m_scenes[sceneName].get();
        if (scene) {
            scene->update(deltaTime);
        }
    }

    // Traiter transitions d'entités
    processSceneTransitions();
}

void SceneManager::processSceneTransitions() {
    for (auto& [sceneName, scene] : m_scenes) {
        auto entities = scene->getEntityRegistry().getEntitiesWith({"SceneTransitionComponent"});

        for (Entity* entity : entities) {
            auto* transition = entity->getComponent<SceneTransitionComponent>();

            if (transition->isTransitioning) {
                // Transférer entité vers scène cible
                transferEntityToScene(entity, sceneName, transition->targetScene, transition->targetPosition);
                transition->isTransitioning = false;
            }
        }
    }
}
```

---

**Prochaine section** : [Système de Rendu](07-RENDERING.md)
