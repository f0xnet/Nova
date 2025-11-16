# NovaEngine - Vue d'Ensemble Rapide

## En 5 minutes: Qu'est-ce que NovaEngine?

**NovaEngine** est un moteur de jeu 2D écrit en C++17 avec une architecture modulaire utilisant le pattern **ECS (Entity Component System)**.

```
Entité = Conteneur d'ID
   ├─ Components = Données pures (position, sprite, santé, etc.)
   └─ Systems = Logique (animation, render, collision, etc.)
```

---

## Architecture 5 Piliers

### 1. ECS - Logique Jeu
```
Entités + Composants + Systèmes = Logique flexible et découplée

Exemple: Sprite animé qui se déplace
  Entity {
    TransformComponent { position: (100, 200), rotation: 0 }
    SpriteComponent { texture, zOrder, visible }
    AnimationComponent { frames, duration, playing }
    ColliderComponent { type: Box, size: (32, 32) }
  }
  
  Systems exécutés en ordre:
  1. AnimationSystem → met à jour frames
  2. PhysicsSystem → détecte collisions
  3. RenderSystem → dessine (LAST!)
```

### 2. Backend - Abstraction Graphique
```
Pas de SFML directement dans le code jeu!

GRAPHICS().drawSprite(data);   // Appel backend via singleton
WINDOW().clear(Color::Black);  // Fenêtre
INPUT().pollEvent(event);      // Input
AUDIO().playSound(handle);     // Audio

Backend implémentation = SFML (mais SDL possible plus tard)
```

### 3. JSON Deux-Tiers - Data Driven
```
Tier 1: Définitions (chargées une fois)
  assets/data/definitions/Sprites.json
  assets/data/definitions/Animations.json
  → DefinitionManager charge tout en mémoire

Tier 2: Scènes (référencent définitions)
  assets/data/scenes/interior_1.json
  {
    "entities": [
      { "type": "sprite", "spriteID": "wall_01" }  // Ref à définition!
    ]
  }
```

### 4. UI Indépendant - UIManager
```
UIManager gère composants UI séparément du ECS

UIManager.addComponent(Button...)
UIManager.addComponent(Text...)
UIManager.render()  // Rendu séparé, organisé par layer

Composants UI: Button, Text, Image, Panel, Slider, TextInput
```

### 5. Application Framework - Boucle Principale
```cpp
class MyGame : public Application {
  bool onInitialize() override {
    sceneManager.loadScene("level1.json", "level1");
    return true;
  }
  
  void onUpdate(float dt) override {
    sceneManager.update(dt);
  }
  
  void onRender() override {
    sceneManager.render();
  }
};

int main() {
  MyGame game;
  return game.run();  // Boucle principale s'exécute ici
}
```

---

## Composants ECS Clés

| Composant | Rôle |
|-----------|------|
| **TransformComponent** | Position, rotation, scale |
| **SpriteComponent** | Texture, couleur, zOrder |
| **AnimationComponent** | Frames, duration, animation state |
| **ColliderComponent** | Hitbox (box ou cercle) |
| **AudioComponent** | Lecture son |
| **ActivatorComponent** | Zone trigger (porte, switch, etc.) |
| **TagComponent** | Label ("player", "enemy", etc.) |
| **JourneyComponent** | Voyage multi-scène (NPCs) |

---

## Systèmes ECS Clés

| Système | Rôle |
|---------|------|
| **AnimationSystem** | Met à jour frames des animations |
| **PhysicsSystem** | Détecte collisions AABB |
| **ActivatorSystem** | Gère zones trigger et événements |
| **AudioSystem** | Joue sons au démarrage |
| **RenderSystem** | Dessine tous les sprites (EN DERNIER!) |
| **JourneySystem** | Gère voyages multi-scènes NPCs |

---

## Flux Une Frame

```
Application::runMainLoop()
  ├─ INPUT().pollEvent()
  │  └─ onEvent(Event)
  │
  ├─ onUpdate(deltaTime)
  │  └─ SceneManager::update(deltaTime)
  │     └─ Scene::update(deltaTime)
  │        └─ Pour chaque System:
  │           ├─ AnimationSystem::update()
  │           ├─ PhysicsSystem::update()
  │           ├─ ActivatorSystem::update()
  │           ├─ AudioSystem::update()
  │           ├─ LightSystem::update()
  │           └─ RenderSystem::update()
  │              └─ GRAPHICS().drawSprite() pour chaque sprite
  │
  ├─ WINDOW().clear()
  │
  ├─ onRender()
  │  └─ SceneManager::render()
  │
  ├─ UIManager::render()
  │  └─ Dessine UI components par layer
  │
  └─ WINDOW().display()
```

---

## Caractéristiques Uniques

### 1. Multi-Scène Travel (UNIQUE!)
Au lieu de TELEPORTER les NPCs:
```
NPC voyage: InterieurA → ExteriorMain → InterieurB

SceneManager active les 3 scènes:
- InterieurA: Player est ici (full render)
- ExteriorMain: NPC traverse (update + render si visible)
- InterieurB: Vide (pas update)

Player voit le NPC passer par ExteriorMain!
```

### 2. Waypoint Pathfinding
```
Scene contient WaypointGraph (pathfinding local BFS)
SceneManager contient SceneGraph (pathfinding multi-scène BFS)

NPC suit waypoints dans scène + transitions entre scènes physiquement
```

### 3. Activators (Zones Trigger Flexibles)
```
ActivatorComponent types:
  - Proximity: Active à l'entrée, désactive à la sortie
  - Automatic: Actif tant que entity dans zone
  - Manual: Nécessite action (key press)

Formes: Box ou Circle
Actions: Peut fire événements custom
```

---

## Fichiers de Configuration Clés

### 1. Definition Files (Tier 1)
```json
// assets/data/definitions/Sprites.json
{
  "sprites": [
    {
      "id": "wall_01",
      "texturePath": "assets/textures/wall_01.png",
      "textureRect": [0, 0, 64, 64],
      "size": [64, 64]
    }
  ]
}
```

### 2. Scene Files (Tier 2)
```json
// assets/data/scenes/interior_1.json
{
  "name": "interior_1",
  "backgroundColor": [32, 32, 64, 255],
  "pathfinding": {
    "waypoints": [
      {"id": "entrance", "position": [100, 200]},
      {"id": "exit", "position": [1200, 200]}
    ],
    "connections": [
      {"from": "entrance", "to": "exit", "bidirectional": true}
    ]
  },
  "entities": [
    {
      "type": "sprite",
      "position": [500, 300],
      "spriteID": "wall_01"
    }
  ]
}
```

### 3. SceneGraph (Multi-Scène)
```json
// assets/data/scenegraph.json
{
  "connections": [
    {
      "from": "interior_1",
      "to": "exterior_main",
      "exitPortal": [100, 200],
      "entryPortal": [500, 500]
    }
  ]
}
```

### 4. Configuration (engine.ini)
```ini
[Display]
width=1920
height=1080
vsync=true

[Audio]
masterVolume=100.0
soundVolume=90.0

[Debug]
enableLogging=true
logLevel=INFO
```

---

## Macros de Confort

```cpp
// Backend access
GRAPHICS().drawSprite(data);
WINDOW().clear(Color::Black);
INPUT().pollEvent(event);
AUDIO().playSound(handle);

// Configuration
auto& displayConfig = DISPLAY_CONFIG;
auto& audioConfig = AUDIO_CONFIG;

// Logging (auto-inclut filename)
LOG_INFO("Message: {}", variable);
LOG_WARN("Warning: {}", data);
LOG_ERROR("Error: {}", error);
```

---

## Workflow Typique

### 1. Créer Définition d'Entité
```json
// assets/data/definitions/Sprites.json
{
  "sprites": [
    {
      "id": "player_walk",
      "texturePath": "assets/textures/player.png",
      "size": [32, 48],
      "zOrder": 20
    }
  ]
}
```

### 2. Créer Scene
```json
// assets/data/scenes/level1.json
{
  "name": "level1",
  "backgroundColor": [50, 50, 50, 255],
  "entities": [
    {
      "type": "sprite",
      "position": [640, 360],
      "spriteID": "player_walk"
    }
  ]
}
```

### 3. Code Jeu
```cpp
class Game : public Application {
  SceneManager sceneManager;
  
  bool onInitialize() override {
    sceneManager.initialize();
    sceneManager.loadScene("assets/data/scenes/level1.json", "level1");
    sceneManager.setActiveScene("level1");
    return true;
  }
  
  void onUpdate(float dt) override {
    sceneManager.update(dt);
  }
  
  void onRender() override {
    sceneManager.render();
  }
};
```

---

## Singletons Clés

```cpp
BackendManager::get()        // Accès backends
Logger::getInstance()         // Logging
ConfigManager::getInstance()  // Configuration
```

---

## Points À Retenir

1. **ECS est tout** - Entities + Components + Systems
2. **JSON two-tier** - Définitions séparées des scènes
3. **Backend abstrait** - GRAPHICS() au lieu de SFML direct
4. **Multi-scène est réel** - NPCs traversent physiquement
5. **UI indépendant** - UIManager != ECS
6. **Order matters** - Systems exécutés dans l'ordre
7. **Config centralisé** - ConfigManager gère tout
8. **Logging partout** - LOG_INFO(), LOG_ERROR(), etc.

---

## Fichiers pour Démarrer Docs

1. **ARCHITECTURE_INDEX.md** - Tables des fichiers + hiérarchies
2. **ARCHITECTURE_REPORT.md** - Détails complets (2125 lignes!)
3. **ARCHITECTURE_QUICK_START.md** - Ce fichier

## Cheminement Recommandé

```
Pas de détails:
  1. QUICK_START (ce fichier) = 5 min
  2. INDEX (tables) = 10 min
  
Détails complets:
  3. REPORT (détails) = 1-2 heures
  
Implémentation:
  4. Lire fichiers SDK correspondants
  5. Créer première application
```

---

Generated: 2025-11-16  
Architecture Level: Very Thorough  
Documentation Status: Ready for Full Documentation Writing

