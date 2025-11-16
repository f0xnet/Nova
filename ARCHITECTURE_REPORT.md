# RAPPORT D'ARCHITECTURE COMPLÈTE - NovaEngine

**Date:** 2025-11-16  
**Niveau de détail:** Très exhaustif  
**Objectif:** Analyse complète pour documentation

---

## TABLE DES MATIÈRES

1. [Structure globale du projet](#structure-globale)
2. [Architecture ECS (Entity Component System)](#architecture-ecs)
3. [Architecture Backend](#architecture-backend)
4. [Système UI](#système-ui)
5. [Systèmes principaux](#systèmes-principaux)
6. [Application Framework](#application-framework)
7. [Flux d'exécution détaillé](#flux-dexécution)
8. [Format de fichiers JSON](#formats-json)

---

## Structure globale

### Répertoires du projet
```
Nova/
├── sdk/
│   ├── include/NovaEngine/
│   │   ├── Audio/               # Gestion audio
│   │   ├── Backend/             # Abstraction des backends
│   │   ├── Core/                # Noyau (Logger, ConfigManager, Application)
│   │   ├── ECS/                 # Entity Component System
│   │   ├── Events/              # Système d'événements
│   │   ├── Resources/           # Gestionnaire de ressources
│   │   ├── UI/                  # Système UI
│   │   └── Game.hpp
│   └── [dépendances externes]
├── client/
│   └── src/                     # Implémentations spécifiques au client
├── server/
├── examples/
└── [fichiers de configuration]
```

### Hiérarchie des namespaces
```
NovaEngine/
├── Components principaux
├── Systems
├── ECS (Entity Registry, Scene, SceneManager)
├── Backend (BackendManager, Interfaces, SFML implementation)
├── UI (UIManager, UILoader, UIComponent, Components UI)
├── Core (Application, Logger, ConfigManager)
├── Resources (ResourceManager)
├── Audio (AudioManager, SoundPlayer, MusicPlayer)
└── Events (Event, EventDispatcher, EventHandler)
```

---

## ARCHITECTURE ECS (Entity Component System)

### Vue d'ensemble
Le système ECS de NovaEngine suit le pattern classique :
- **Entities** : Conteneurs de composants (IDs uniques)
- **Components** : Données pures (position, sprite, lumière, etc.)
- **Systems** : Logique qui opère sur les entités avec des composants spécifiques

### Hiérarchie des classes

```cpp
// BASE CLASSES
Component (base class abstraite)
  ├── TransformComponent
  ├── SpriteComponent
  ├── AnimationComponent
  ├── LightComponent
  ├── ColliderComponent
  ├── AudioComponent
  ├── ActivatorComponent
  ├── TagComponent
  ├── SceneTransitionComponent
  └── JourneyComponent

System (base class abstraite)
  ├── RenderSystem
  ├── AnimationSystem
  ├── LightSystem
  ├── AudioSystem
  ├── PhysicsSystem
  ├── ActivatorSystem
  └── JourneySystem
```

### 1. COMPONENTS (Détaillé)

#### TransformComponent
**Responsabilité:** Position, rotation, et échelle des entités
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 11-49)

**Propriétés:**
```cpp
Vec2f position = {0.0f, 0.0f};      // Position XY dans le monde
f32 rotation = 0.0f;                 // Rotation en degrés
Vec2f scale = {1.0f, 1.0f};          // Échelle X/Y
Vec2f origin = {0.0f, 0.0f};         // Point d'origine/pivot
```

**Sérialisation:** Supporte JSON (serialize/deserialize)
**Utilité:** Requis pour TOUTE entité qui doit avoir une position

---

#### SpriteComponent
**Responsabilité:** Rendu de sprites 2D texturés
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 51-103)

**Propriétés clés:**
```cpp
ID textureID;                           // Référence aux définitions
TextureHandle textureHandle;            // Handle pour le rendu
IntRect textureRect = {0,0,0,0};       // Sous-rectangle (0,0,0,0 = full)
Vec2f size = {0,0};                    // Taille affichage (0,0 = natif)
Color tint = Color::White;              // Teinte/couleur
BlendMode blendMode = BlendMode::Alpha; // Mode de fusion
i32 zOrder = 0;                        // Profondeur (bas = derrière)
bool visible = true;
```

**Pipeline de rendu:**
1. TransformComponent fournit position/rotation/scale
2. SpriteComponent fournit texture et apparence
3. RenderSystem combine les données et appelle `GRAPHICS().drawSprite()`

---

#### AnimationComponent
**Responsabilité:** Animation basée sur les frames
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 169-215)

**Propriétés:**
```cpp
ID animationID;                  // Référence à la définition
std::vector<IntRect> frames;     // Liste des rectangles de frames
f32 frameDuration = 0.1f;        // Temps par frame (secondes)
f32 currentTime = 0.0f;          // Temps accumulé actuel
u32 currentFrame = 0;            // Index du frame actuel
bool loop = true;                // Boucle indéfiniment?
bool playing = true;             // Animation active?
```

**Fonctionnement:**
- AnimationSystem reçoit deltaTime
- Accumule currentTime
- Quand currentTime >= frameDuration: currentFrame++
- Met à jour SpriteComponent.textureRect
- Gère boucle/fin selon settings

---

#### LightComponent
**Responsabilité:** Lumières (ponctuelle, directionnelle, projecteur)
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 109-166)

**Propriétés:**
```cpp
enum class LightType { Point, Directional, Spot };

LightType type = LightType::Point;
Color color = Color::White;
f32 radius = 100.0f;           // Pour Point/Spot
f32 intensity = 1.0f;          // 0.0 à 1.0+
Vec2f direction = {0,0};       // Pour Directional/Spot
f32 angle = 45.0f;             // Pour Spot (degrés)
bool castShadows = false;
bool enabled = true;
```

**Rendu:**
- LightSystem dessine les lumières comme des rectangles semi-transparents
- Implémentation simple pour visualisation
- Pour du vrai éclairage: utiliser des shaders

---

#### ColliderComponent
**Responsabilité:** Collision physique (boîte ou cercle)
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 220-260)

**Propriétés:**
```cpp
enum class ColliderType { Box, Circle };

ColliderType type = ColliderType::Box;
Vec2f size = {0,0};            // Pour Box
f32 radius = 0.0f;             // Pour Circle
Vec2f offset = {0,0};          // Décalage depuis position entité
bool isTrigger = false;        // Trigger (pas de physique) vs solide
bool enabled = true;
```

**Détection:**
- PhysicsSystem vérifie les collisions AABB (box-box uniquement)
- Log les collisions détectées
- Framework: utiliser pour la base, intégrer Box2D pour complexe

---

#### AudioComponent
**Responsabilité:** Lecture de son/musique
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 266-296)

**Propriétés:**
```cpp
ID soundID;                         // Référence audio
SoundHandle soundHandle;            // Handle pour playback
bool playOnStart = false;           // Jouer au chargement?
bool loop = false;                  // Boucler?
f32 volume = 100.0f;                // Volume (0-100)
f32 pitch = 1.0f;                   // Pitch (1.0 = normal)
bool playing = false;               // État de lecture
```

---

#### ActivatorComponent
**Responsabilité:** Zones de déclenchement (portes, interrupteurs, etc.)
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 308-400)

**Types d'activation:**
```cpp
enum class ActivatorType {
    Proximity,      // S'active à l'entrée, se désactive à la sortie
    Manual,         // Nécessite action manuelle (touche)
    Automatic       // Actif en continu tant que dans zone
};

enum class ActivatorShape { Box, Circle };
```

**Propriétés clés:**
```cpp
ActivatorType type = ActivatorType::Proximity;
ActivatorShape shape = ActivatorShape::Box;
Vec2f size = {100,100};         // Pour Box
f32 radius = 50.0f;             // Pour Circle
Vec2f offset = {0,0};

bool isActive = false;
bool canReactivate = true;
f32 cooldownTime = 0.0f;
f32 currentCooldown = 0.0f;

std::string targetTag = "player"; // Quelle entité active?
std::string actionID;             // Action à déclencher
std::string onActivateEvent;      // Événement fire
std::string onDeactivateEvent;

bool showDebugZone = false;       // Visualisation debug
Color debugColor = {0,255,0,100};
```

---

#### TagComponent
**Responsabilité:** Identification des entités pour logique métier
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 410-425)

**Exemple d'utilisation:**
```cpp
TagComponent tag;
tag.tag = "player";  // Peut être "player", "enemy", "npc", etc.
```

**Utilité:** 
- ActivatorSystem vérifie `targetTag` contre `TagComponent`
- Permet logique de jeu flexible

---

#### SceneTransitionComponent
**Responsabilité:** Gestion de transitions entre scènes
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 436-464)

**Propriétés:**
```cpp
std::string targetScene;        // Scène de destination
Vec2f targetPosition;           // Position dans scène cible
bool isTransitioning = false;   // État transition en cours?
```

**Flux:**
1. Entité définit targetScene et targetPosition
2. JourneySystem marque comme transitioning
3. SceneManager effectue le transfert
4. Entité apparaît à targetPosition dans nouvelle scène

---

#### JourneyComponent
**Responsabilité:** Gestion de multi-scène voyages pour NPCs
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Components.hpp` (lignes 476-559)

**Paradigme unique de NovaEngine:**
Au lieu de TELEPORTER les NPCs entre scènes, ils TRAVERSENT PHYSIQUEMENT
les scènes intermédiaires. Cela permet au joueur de voir les NPCs
passer par les zones intermédiaires!

**Propriétés:**
```cpp
// Chemin multi-scènes
std::vector<std::string> scenePath;     // Ex: ["interior_1", "exterior_main", "interior_2"]
int currentSceneIndex = 0;

// Destination dans scène actuelle
Vec2f currentDestination;
bool reachedCurrentDestination = false;

// Pathfinding local (waypoints dans scène)
std::vector<Vec2f> localWaypointPath;   // Waypoints dans scène actuelle
int currentLocalWaypointIndex = 0;

// Personnalité NPC
std::vector<std::string> preferredPathTags;  // Ex: ["main_road"], ["shortcut"]

bool isOnJourney = false;
std::string finalDestinationScene;
Vec2f finalDestinationPos;
```

**Algorithme de voyage:**
```
1. NPC commence journey(A → C via B)
2. scenePath = [A, B, C]
3. Boucle:
   - Si pas all waypoints locaux: suivre waypoints dans scène actuelle
   - Si destination atteinte et scène suivante existe:
     → Transition vers scène suivante
     → Calculer waypoints locaux vers prochain portail
   - Si destination scène finale: journey terminé
```

---

### 2. SYSTEMS (Détaillé)

Les systems opèrent en séquence dans Scene::update():

```cpp
// Ordre dans Scene constructor
m_systems.push_back(std::make_unique<AnimationSystem>());
m_systems.push_back(std::make_unique<PhysicsSystem>());
m_systems.push_back(std::make_unique<ActivatorSystem>());
m_systems.push_back(std::make_unique<AudioSystem>());
m_systems.push_back(std::make_unique<LightSystem>());
m_systems.push_back(std::make_unique<RenderSystem>()); // RENDER LAST
```

**Ordre critique:** Animation avant Render (update sprites avant draw)

---

#### RenderSystem
**Responsabilité:** Dessine tous les sprites
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 20-61)

**Processus:**
```cpp
1. Récupère tous les entités avec {TransformComponent, SpriteComponent}
2. Trie par zOrder (valeurs basses d'abord = derrière)
3. Pour chaque entité:
   - Crée SpriteData depuis Transform + Sprite
   - Appelle GRAPHICS().drawSprite(spriteData)
```

**Paramètres SpriteData utilisés:**
- texture (TextureHandle)
- position, size, rotation, scale, origin (Transform)
- textureRect (Sprite)
- color (tint), blendMode (Sprite)

---

#### AnimationSystem
**Responsabilité:** Met à jour les animations frame-based
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 72-112)

**Algorithme:**
```cpp
Pour chaque entité {SpriteComponent, AnimationComponent}:
  1. anim->currentTime += deltaTime
  2. Si currentTime >= frameDuration:
     - currentTime = 0
     - currentFrame++
     - Si currentFrame >= frames.size():
       - Si loop: currentFrame = 0
       - Sinon: currentFrame = size-1, playing = false
  3. sprite->textureRect = frames[currentFrame]
```

---

#### PhysicsSystem
**Responsabilité:** Détection de collision AABB simple
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 196-245)

**Limitations:**
- Seulement box-box (pas cercle, pas SAT)
- Détecte collisions, ne résout pas
- Framework pour démarrer, intégrer Box2D pour production

**Détection:**
```cpp
Pour chaque pair d'entités avec ColliderComponent:
  - Calcule AABB de chaque collider
  - Teste intersection AABB simple
  - Log si collision détectée
```

---

#### ActivatorSystem
**Responsabilité:** Gère les zones de déclenchement
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 258-430)

**Logique complète:**
```cpp
1. Update cooldowns pour tous les activators
2. Pour chaque activator:
   - Calcule sa zone (box ou cercle)
   - Récupère toutes les entités avec TagComponent
   - Filtre par targetTag
   - Test si dans zone
   - Selon type d'activation:
     * Proximity: activate si entre + n'était pas actif
     * Automatic: reste actif si dans zone
     * Manual: log quand possible
   - Gère cooldown après activation
   - Fire événements (onActivateEvent/onDeactivateEvent)
3. Dessine debug zones si showDebugZone
```

**Événements:**
- onActivateEvent: fire quand activation
- onDeactivateEvent: fire quand deactivation

---

#### AudioSystem
**Responsabilité:** Joue les sons au démarrage
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 165-185)

**Logique simple:**
```cpp
Pour chaque entité avec AudioComponent:
  Si playOnStart && !playing:
    - AUDIO().playSound(handle, volume, 1.0, loop)
    - playing = true
```

---

#### JourneySystem
**Responsabilité:** Gère les voyages multi-scènes
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Systems.hpp` (lignes 445-694)

**Attributs:**
```cpp
class SceneGraph* m_sceneGraph;  // Pour pathfinding
std::vector<PendingTransfer> m_pendingTransfers;
```

**Processus principal (update):**
```
1. Pour chaque entité {Transform, SceneTransition, Journey}:
   - Si pas isOnJourney: skip
   - Si localWaypointPath existe:
     * Suivre waypoints
     * Quand waypoint atteint (< 5px): incrementer index
   - Sinon: navigation directe vers currentDestination
   
2. Si destination atteinte:
   - Si scenePath a scène suivante:
     * Récupère connection du SceneGraph
     * Crée PendingTransfer
     * Marque transition
   - Sinon: journey complete

3. updateTransferredEntities() appelé après transition:
   - Met à jour journey state
   - Définit prochaine destination
```

**API publiques:**
```cpp
bool startJourney(Entity*, currentScene, targetScene, targetPosition)
void calculateLocalWaypointPath(Entity*, Scene*)
void cancelJourney(Entity*)
```

---

### 3. Scene & SceneManager

#### Scene
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Scene.hpp`

**Responsabilités:**
- Contient EntityRegistry (toutes les entités)
- Contient tous les Systems
- Contient WaypointGraph (pour pathfinding local)

**Initialisation:**
```cpp
explicit Scene(const std::string& name) {
    // Crée tous les systems dans l'ordre
    m_systems.push_back(std::make_unique<AnimationSystem>());
    // ... etc
    m_systems.push_back(std::make_unique<RenderSystem>()); // Dernier
}
```

**Chargement JSON:**
```cpp
bool loadFromJSON(const nlohmann::json& sceneData, const DefinitionManager& defManager)
```

**Format scène attendu:**
```json
{
  "name": "interior_1",
  "type": "interior",
  "backgroundColor": [0, 0, 0, 255],
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
    // ... plus d'entités
  ]
}
```

**Types d'entités créés:**
- "sprite" → SpriteComponent
- "light" → LightComponent
- "animated_sprite" → SpriteComponent + AnimationComponent
- "audio" → AudioComponent
- "activator" → ActivatorComponent
- "player" → TagComponent("player") + optionnel sprite

---

#### SceneManager
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/SceneManager.hpp`

**Responsabilités:**
- Charge/décharge scènes
- Gère DefinitionManager (definitions globales)
- Gère SceneGraph (pour multi-scène pathfinding)
- Maintient scène active
- Update/render tous les scènes actives

**Hiérarchie de chargement:**
```
1. initialize():
   - Charge DefinitionManager (Sprites.json, Lights.json, etc.)
   - Charge SceneGraph (scenegraph.json)

2. loadScene(path, name):
   - Parse JSON depuis path
   - Crée Scene
   - Scene.loadFromJSON() utilise DefinitionManager

3. setActiveScene(name):
   - Change m_activeScene
```

**Update logic complexe:**
```cpp
void update(float deltaTime) {
  // Collector les scènes avec des NPCs en voyage
  std::unordered_set<std::string> scenesOnActivePaths;
  
  for (chaque scène) {
    Cherche entités avec JourneyComponent
    Si isOnJourney: ajoute tous les scènes du parcours
  }
  
  // Update scènes actives
  for (chaque scène) {
    Si scène rendue: full update
    Si sur parcours actif: update aussi (NPCs traversent)
    Sinon: dormant, pas d'update
  }
}
```

---

### 4. EntityRegistry & Entity

#### Entity
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/Entity.hpp`

**Conteneur générique:**
```cpp
class Entity {
  u64 m_id;
  std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;
  
public:
  template<typename T> T* addComponent(std::unique_ptr<T>);
  template<typename T> T* getComponent();
  template<typename T> bool hasComponent();
  std::vector<ComponentTypeID> getComponentTypes();
};
```

**ComponentTypeID:** `using ComponentTypeID = std::string` (nom de classe)

**Macro de type ID:**
```cpp
#define COMPONENT_TYPE_ID(TypeName) \
  ComponentTypeID getTypeID() const override { return #TypeName; }
```

---

#### EntityRegistry
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/ECS/EntityRegistry.hpp`

**Manager d'entités:**
```cpp
class EntityRegistry {
  std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
  u64 m_nextID = 1;
  
public:
  Entity* createEntity();
  void destroyEntity(u64 entityID);
  Entity* getEntity(u64 entityID);
  
  // Requête - c'est MAGIQUE
  std::vector<Entity*> getEntitiesWith(
    const std::vector<ComponentTypeID>& componentTypes
  );
  
  std::vector<Entity*> getAllEntities();
  size_t getEntityCount();
};
```

**Requête ECS (getEntitiesWith):**
```cpp
// Exemple:
auto entities = registry.getEntitiesWith({
  "TransformComponent", "SpriteComponent"
});
// Retourne tous les entités qui ONT CES DEUX composants
```

---

## ARCHITECTURE BACKEND

**Objectif:** Abstraire les détails graphiques/audio/fenêtre

### BackendManager (Singleton)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/BackendManager.hpp`

**Gestion unique des backends:**
```cpp
class BackendManager {
  static BackendManager& get();  // Singleton
  
  bool initialize(
    BackendType type = BackendType::SFML,
    u32 windowWidth = 800, u32 windowHeight = 600,
    const String& windowTitle = "NovaEngine",
    bool fullscreen = false
  );
  
  // Accès aux interfaces
  IWindowBackend& window();
  IInputBackend& input();
  IGraphicsBackend& graphics();
  IResourceBackend& resources();
  IAudioBackend& audio();
  IFontBackend& fonts();
  IViewportBackend& viewport();
};

// Macros de confort
#define WINDOW()    BackendManager::get().window()
#define INPUT()     BackendManager::get().input()
#define GRAPHICS()  BackendManager::get().graphics()
#define RESOURCES() BackendManager::get().resources()
#define AUDIO()     BackendManager::get().audio()
#define FONTS()     BackendManager::get().fonts()
#define VIEWPORT()  BackendManager::get().viewport()
```

**Exemple d'utilisation:**
```cpp
// Au lieu de:
// someObject.window->drawSprite(data);

// On utilise:
GRAPHICS().drawSprite(data);  // Clean!
AUDIO().playSound(handle);
```

---

### Types de base (BackendTypes.hpp)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/Core/BackendTypes.hpp`

**Types d'alias:**
```cpp
using i8/i16/i32/i64 = signed integers
using u8/u16/u32/u64 = unsigned integers
using f32/f64 = float/double
using String = std::string

using TextureHandle = u64;     // Opaque texture ID
using FontHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
using ShaderHandle = u64;

constexpr u64 INVALID_HANDLE = 0;
```

**Structs principaux:**

```cpp
struct Vec2f {
  f32 x, y;
  // Opérateurs: +, -, *, /, +=, -=, *=, /=
};

struct Color {
  u8 r, g, b, a;
  static const Color Black, White, Red, Green, Blue, Yellow, Transparent;
};

enum class KeyCode { A, B, ..., Escape, LControl, ..., Space, Enter, ... };
enum class MouseButton { Left, Right, Middle };

struct InputEvent {
  InputEventType type;  // Closed, Resized, KeyPressed, ...
  union {
    struct { u32 width, height; } size;
    struct { KeyCode code; bool alt, control, shift, system; } key;
    struct { MouseButton button; i32 x, y; } mouseButton;
    // ...
  };
};

struct SpriteData {
  TextureHandle texture;
  Vec2f position, size;
  f32 rotation;
  Vec2f scale, origin;
  IntRect textureRect;
  Color color;
  BlendMode blendMode;
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
```

---

### Interfaces abstraites

**Fichier pattern:** `/home/user/Nova/sdk/include/NovaEngine/Backend/Interfaces/I*Backend.hpp`

#### IWindowBackend
```cpp
class IWindowBackend {
public:
  virtual bool initialize(u32 width, u32 height, ...) = 0;
  virtual void close() = 0;
  virtual bool isOpen() const = 0;
  virtual void setTitle(const String& title) = 0;
  virtual void clear(const Color& color) = 0;
  virtual void display() = 0;
  virtual void setVSync(bool enabled) = 0;
  virtual void setFramerateLimit(u32 limit) = 0;
  // ... plus de méthodes
};
```

#### IGraphicsBackend
```cpp
class IGraphicsBackend {
public:
  virtual bool initialize(void* windowHandle) = 0;
  
  // Texture management
  virtual TextureHandle loadTexture(const String& path) = 0;
  virtual TextureHandle createTexture(u32 width, u32 height) = 0;
  virtual void unloadTexture(TextureHandle handle) = 0;
  
  // Drawing
  virtual void drawSprite(const SpriteData& sprite) = 0;
  virtual void drawRect(const RectData& rect) = 0;
  virtual void drawText(const TextData& text) = 0;
  
  // Shaders
  virtual ShaderHandle loadShader(const String& vertexPath, const String& fragmentPath) = 0;
  virtual void bindShader(ShaderHandle handle) = 0;
};
```

#### IInputBackend
```cpp
class IInputBackend {
public:
  virtual bool initialize() = 0;
  virtual bool pollEvent(InputEvent& event) = 0;
  virtual bool isKeyPressed(KeyCode code) const = 0;
  virtual bool isMouseButtonPressed(MouseButton button) const = 0;
  virtual Vec2i getMousePosition() const = 0;
};
```

#### IAudioBackend
```cpp
class IAudioBackend {
public:
  virtual bool initialize() = 0;
  virtual SoundHandle loadSound(const String& path) = 0;
  virtual void playSound(SoundHandle handle, f32 volume, f32 pitch, bool loop) = 0;
  // ... music, volume control, etc.
};
```

#### IFontBackend
```cpp
class IFontBackend {
public:
  virtual bool initialize() = 0;
  virtual FontHandle loadFont(const String& path) = 0;
  virtual void getTextMetrics(FontHandle font, const String& text, 
                              u32 charSize, TextMetrics& out) = 0;
};
```

#### IViewportBackend
```cpp
class IViewportBackend {
public:
  virtual void setViewport(const ViewportData& viewport) = 0;
  virtual ViewportData getViewport() const = 0;
  // Camera/viewport control
};
```

---

### SFML Implementation

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp`

**Exemple d'implémentation concrète:**
```cpp
class SFMLGraphicsBackend : public IGraphicsBackend {
private:
  sf::RenderWindow* m_window;
  std::unordered_map<TextureHandle, std::unique_ptr<sf::Texture>> m_textures;
  std::unordered_map<ShaderHandle, std::unique_ptr<sf::Shader>> m_shaders;
  std::unordered_map<FontHandle, sf::Font*> m_fonts;  // Borrowed
  u64 m_nextTextureHandle = 1;
  
public:
  TextureHandle loadTexture(const String& path) override {
    auto texture = std::make_unique<sf::Texture>();
    if (texture->loadFromFile(path)) {
      u64 handle = m_nextTextureHandle++;
      m_textures[handle] = std::move(texture);
      return handle;
    }
    return INVALID_HANDLE;
  }
  
  void drawSprite(const SpriteData& sprite) override {
    auto* sfmlTexture = getSFMLTexture(sprite.texture);
    if (!sfmlTexture) return;
    
    sf::Sprite sfmlSprite(*sfmlTexture);
    sfmlSprite.setPosition(sprite.position.x, sprite.position.y);
    sfmlSprite.setRotation(sprite.rotation);
    sfmlSprite.setScale(sprite.scale.x, sprite.scale.y);
    // ... etc
    m_window->draw(sfmlSprite);
  }
};
```

**Autres fichiers SFML:**
- `SFMLWindowBackend.hpp` - Fenêtre SFML
- `SFMLInputBackend.hpp` - Gestion input SFML
- `SFMLAudioBackend.hpp` - Audio SFML
- `SFMLFontBackend.hpp` - Fonts SFML
- `SFMLConversions.hpp` - Conversions entre types NovaEngine ↔ SFML

---

## SYSTÈME UI

**Hiérarchie:**
```
UIManager
  ├── Gère les composants UI
  ├── Dispatche les événements
  └── Organise par groupes/layers

UIComponent (abstract base)
  ├── Button
  ├── Text
  ├── Image
  ├── Panel
  ├── Slider
  ├── TextInput
  └── Animation

UILoader
  └── Parse JSON et crée composants
```

---

### UIManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIManager.hpp`

**Rôle:** Orchestrateur central de l'UI

```cpp
class UIManager {
private:
  struct ComponentInfo {
    std::shared_ptr<UIComponent> component;
    ID uiID;              // Quel UI? (écran de menu, HUD, etc.)
    ID groupID;           // Quel groupe? (groupe de boutons, etc.)
    i32 layer;            // Profondeur de rendu
  };
  
  std::unordered_map<ID, ComponentInfo> m_components;
  ActionCallback m_actionCallback;
  // Cache de rendu trié par layer
  mutable std::vector<std::pair<i32, std::shared_ptr<UIComponent>>> m_renderCache;
  mutable bool m_renderCacheDirty;
  
public:
  void addComponent(const std::shared_ptr<UIComponent>& component);
  void removeComponent(const ID& id);
  void removeUI(const ID& uiID);                    // Suppr tout un UI
  void removeGroup(const ID& groupID);              // Suppr un groupe
  
  void setGroupActive(const ID& groupID, bool active);
  void setUIActive(const ID& uiID, bool active);
  void switchToGroup(const ID& uiID, const ID& newGroupID);
  void setLayerActive(i32 layer, bool active);
  
  void update(float deltaTime);
  void render() const;
  void dispatchEvent(const Event& event);
  
  void setActionCallback(ActionCallback callback);
  void handleAction(const std::string& action, const std::string& value, const ID& componentID);
  
  // Queries
  std::shared_ptr<UIComponent> getComponent(const ID& id);
  std::vector<std::shared_ptr<UIComponent>> getGroup(const ID& groupID);
  std::vector<std::shared_ptr<UIComponent>> getUI(const ID& uiID);
  i32 getMaxLayers() const;
};
```

**Organisation mentale:**
- Multiple UIs peuvent exister (main menu, HUD, pause menu, etc.)
- Chaque UI contient plusieurs groupes (groupe de boutons, groupe de texte, etc.)
- Chaque composant a une couche (layer) pour le z-order
- Cache de rendu optimisé trié par layer

---

### UIComponent (Base abstraite)

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UIComponent.hpp`

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
  std::string m_effect;              // Nom effet (fade, slide, etc.)
  std::string m_description;         // Tooltip ou description
  
public:
  virtual void update(f32 deltaTime);
  virtual void render() const = 0;
  virtual void onEvent(const Event& event) = 0;
  
  // Setters
  void setPosition(const Vec2f& pos);
  virtual void setSize(const Vec2f& size);
  void setVisible(bool visible);
  void setActive(bool active);
  void setLayer(i32 layer);
  
  // Getters
  virtual Rect getBounds() const = 0;
  const ID& getID() const;
  bool isVisible() const;
  bool isActive() const;
};
```

---

### Composants UI

#### Button
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Button.hpp`

```cpp
enum class ButtonState { NOT_HOVER = 0, HOVER = 1, PRESSED = 2 };

class Button : public UIComponent {
private:
  RectData m_shape;
  SpriteData m_sprite;
  TextData m_text;
  
  ButtonState m_currentState;
  bool m_haveText;
  bool m_buttonPressed;
  std::string m_action;              // Action ID
  std::string m_value;               // Valeur associée
  
  std::unordered_map<ButtonState, TextureHandle> m_textures;
  std::function<void()> m_callback;
  ActionCallback m_actionCallback;
  
public:
  void setText(const std::string& text);
  void setFont(FontHandle font);
  void setFontSize(i32 fontSize);
  void setTextColor(const Color& color);
  void setTextures(TextureHandle normal, TextureHandle hover = INVALID_HANDLE, 
                  TextureHandle pressed = INVALID_HANDLE);
  void setAction(const std::string& action);
  void setValue(const std::string& value);
  void setOnClick(std::function<void()> callback);
  void setOnClickWithAction(ActionCallback callback);
  
  void onEvent(const Event& event) override;
  void update(float deltaTime) override;
  void render() const override;
  Rect getBounds() const override;
};
```

**Fonctionnement:**
1. Détecte position souris et état survol
2. Change texture selon état (normal/hover/pressed)
3. Au clic: appelle callback ou actionCallback
4. Optionnel: affiche texte par-dessus

---

#### Text
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Text.hpp`

```cpp
class Text : public UIComponent {
private:
  TextData m_textData;
  std::string m_content;
  
public:
  void setString(const std::string& str);
  void setFont(FontHandle font);
  void setCharacterSize(u32 size);
  void setTextColor(const Color& color);
  void setTextStyle(TextStyle style);
  
  const std::string& getString() const;
  const Color& getTextColor() const;
  u32 getCharacterSize() const;
  TextStyle getTextStyle() const;
  
  void render() const override;
  Rect getBounds() const override;
};
```

---

#### Image
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Image.hpp`

```cpp
class Image : public UIComponent {
private:
  SpriteData m_sprite;
  
public:
  void setTexture(TextureHandle texture);
  void setTexture(TextureHandle texture, bool resetRect);
  void setTextureRect(const IntRect& rect);
  void setColor(const Color& color);
  
  TextureHandle getTexture() const;
  const IntRect& getTextureRect() const;
  const Color& getColor() const;
  
  void render() const override;
  Rect getBounds() const override;
};
```

---

#### Panel
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Panel.hpp`

```cpp
class Panel : public UIComponent {
private:
  RectData m_background;
  
public:
  void setColor(const Color& color);
  
  void render() const override;
  Rect getBounds() const override;
};
```

**Utilité:** Boîte colorée simple pour conteneur visuel

---

#### Slider
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Slider.hpp`

```cpp
class Slider : public UIComponent {
private:
  RectData m_track;       // Barre
  RectData m_handle;      // Curseur
  f32 m_value;            // 0.0 à 1.0
  bool m_dragging;
  std::function<void(f32)> m_onValueChanged;
  
public:
  void setValue(f32 value);
  f32 getValue() const;
  void setOnValueChanged(std::function<void(f32)> callback);
  
  void onEvent(const Event& event) override;
  void render() const override;
  Rect getBounds() const override;
};
```

**Interactif:** Drag souris met à jour value

---

#### TextInput
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/TextInput.hpp`

```cpp
class TextInput : public UIComponent {
private:
  RectData m_box;
  TextData m_text;
  std::string m_buffer;
  bool m_focused;
  bool m_cursorVisible;
  f32 m_cursorTimer;
  std::function<void(const std::string&)> m_onTextChanged;
  
public:
  void setFont(FontHandle font);
  const std::string& getText() const;
  void setText(const std::string& text);
  void setOnTextChanged(std::function<void(const std::string&)> callback);
  
  void onEvent(const Event& event) override;
  void update(f32 deltaTime) override;
  void render() const override;
  Rect getBounds() const override;
};
```

**Interactif:** 
- Click pour focus
- Tape du texte dans m_buffer
- Curseur clignote
- Appelle callback sur changement

---

#### Animation (UI)
**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/Components/Animation.hpp`

```cpp
class Animation : public UIComponent {
private:
  SpriteData m_sprite;
  Vec2u m_frameSize;        // Taille d'une frame
  f32 m_frameTime;
  f32 m_elapsed;
  bool m_playing;
  u32 m_currentFrame;
  u32 m_totalFrames;
  
public:
  void setTexture(TextureHandle texture);
  void setFrameSize(const Vec2u& size);
  void setFrameTime(f32 seconds);
  void setPlaying(bool playing);
  
  void update(f32 deltaTime) override;
  void render() const override;
  Rect getBounds() const override;
};
```

**Différent de ECS AnimationComponent:**
- Basé sur taille de frame fixe (pas frame rectangles custom)
- Texture est spritesheet simple (n frames x m lignes)
- Calcule rect automatiquement

---

### UILoader

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/UI/UILoader.hpp`

```cpp
struct UILayoutData {
  std::string name;
  ID uiID;
  std::string language;
  std::string description;
  std::string background;
  i32 layers;
};

class UILoader {
public:
  bool loadFromFile(const std::string& path, UIManager& uiManager);
  bool loadUI(const ID& uiID, UIManager& uiManager);
  bool loadFromData(const nlohmann::json& jsonData, UIManager& uiManager);
  
private:
  void parseButtons(const nlohmann::json& buttonsJson, UIManager& uiManager, const UILayoutData& layoutData);
  void parseImages(const nlohmann::json& imagesJson, UIManager& uiManager, const UILayoutData& layoutData);
  void parseTexts(const nlohmann::json& textsJson, UIManager& uiManager, const UILayoutData& layoutData);
  void parseInputs(const nlohmann::json& inputsJson, UIManager& uiManager, const UILayoutData& layoutData);
  
  // Utilitaires
  Color parseColor(const std::string& colorStr) const;
  Vec2f applyRescale(f32 x, f32 y) const;  // Résolution adaptive
  u32 applyRescaleFontSize(u32 fontSize) const;
};
```

**Format JSON UI:**
```json
{
  "name": "main_menu",
  "uiID": "main_menu_ui",
  "language": "en",
  "layers": 3,
  "buttons": [
    {
      "id": "btn_start",
      "position": [500, 300],
      "size": [200, 50],
      "text": "Start Game",
      "action": "start_game"
    }
  ],
  "texts": [
    {
      "id": "title",
      "position": [400, 100],
      "text": "Main Menu",
      "fontSize": 48
    }
  ],
  "images": [
    {
      "id": "background",
      "position": [0, 0],
      "size": [1920, 1080],
      "texture": "menu_bg"
    }
  ]
}
```

---

## SYSTÈMES PRINCIPAUX

### ResourceManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Resources/ResourceManager.hpp`

```cpp
class ResourceManager {
private:
  std::unordered_map<ID, std::unique_ptr<sf::Texture>> m_textures;
  std::unordered_map<ID, std::unique_ptr<sf::Font>> m_fonts;
  std::unordered_map<ID, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
  std::unordered_map<ID, std::string> m_musicPaths;  // Musiques chargées à la demande
  
public:
  bool loadFromJSON(const std::string& path);
  
  // Textures
  bool loadTexture(const ID& id, const std::string& path);
  sf::Texture& getTexture(const ID& id);
  
  // Fonts
  bool loadFont(const ID& id, const std::string& path);
  sf::Font& getFont(const ID& id);
  
  // Sons
  bool loadSoundBuffer(const ID& id, const std::string& path);
  sf::SoundBuffer& getSoundBuffer(const ID& id);
  
  // Musiques
  bool loadMusic(const ID& id, const std::string& path);
  std::string getMusicPath(const ID& id) const;
  
  void clear();  // Vide tout
};
```

**Utilisation typique:**
```cpp
// Dans Application::onInitialize()
ResourceManager rm;
rm.loadTexture("wall_01", "assets/textures/walls/wall_01.png");
rm.loadFont("ui_font", "assets/fonts/Arial.ttf");

// Plus tard:
auto& texture = rm.getTexture("wall_01");
auto handle = RESOURCES().loadTexture("wall_01");
```

---

### ConfigManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Core/ConfigManager.hpp`

```cpp
struct DisplayConfig {
  u32 width = 1920, height = 1080;
  bool fullscreen = false, vsync = true;
  u32 frameRateLimit = 60;
  u32 nativeWidth = 3840, nativeHeight = 2160;  // Pour rescaling
};

struct AudioConfig {
  f32 masterVolume = 100.0f;
  f32 musicVolume = 80.0f;
  f32 soundVolume = 90.0f;
  bool muteAll = false;
  std::string audioDevice = "default";
};

struct InputConfig {
  struct KeyBinding {
    std::string action;
    std::vector<std::string> keys;
  };
  std::vector<KeyBinding> keyBindings;
  f32 mouseSensitivity = 1.0f;
  bool invertMouse = false;
};

struct DebugConfig {
  bool enableLogging = true;
  std::string logLevel = "INFO";
  std::string logFile = "logs/nova.log";
  bool showFPS = false;
  bool showDebugInfo = false;
  bool enableProfiler = false;
};

struct GameConfig {
  std::string language = "en";
  std::string playerName = "Player";
  bool autoSave = true;
  u32 autoSaveInterval = 300;
  std::string savePath = "saves/";
};

class ConfigManager {
private:
  DisplayConfig m_displayConfig;
  AudioConfig m_audioConfig;
  InputConfig m_inputConfig;
  DebugConfig m_debugConfig;
  GameConfig m_gameConfig;
  
public:
  static ConfigManager& getInstance();
  
  bool loadFromFile(const std::string& configPath = "config/engine.ini");
  bool saveToFile(const std::string& configPath = "config/engine.ini") const;
  bool createDefaultConfig(const std::string& configPath = "config/engine.ini") const;
  
  // Accès
  const DisplayConfig& getDisplayConfig() const;
  // ... etc pour autres
};

// Macros de confort
#define DISPLAY_CONFIG ::NovaEngine::ConfigManager::getInstance().getDisplayConfig()
#define AUDIO_CONFIG ::NovaEngine::ConfigManager::getInstance().getAudioConfig()
// ...
```

**Format INI:**
```ini
[Display]
width=1920
height=1080
fullscreen=false
vsync=true
frameRateLimit=60

[Audio]
masterVolume=100.0
musicVolume=80.0
soundVolume=90.0

[Debug]
enableLogging=true
logLevel=INFO
showFPS=false
```

---

### Logger

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Core/Logger.hpp`

```cpp
enum class LogLevel { Trace, Debug, Info, Warning, Error, Fatal };

class Logger {
public:
  static Logger& getInstance();  // Singleton thread-safe
  
  void setLogFile(const std::string& filepath);
  void setLogLevel(LogLevel level);
  void enableAnsiColors(bool enable);
  
  void log(LogLevel level, std::string_view module, std::string_view message);
  
  template<typename... Args>
  void logf(LogLevel level, std::string_view module, std::string_view format, Args&&... args) {
    std::string formatted = formatString(format, std::forward<Args>(args)...);
    log(level, module, formatted);
  }
};

// Macros - incluent nom du fichier automatiquement
#define LOG_TRACE(format, ...) ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Trace, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FATAL(format, ...)
```

**Utilisation:**
```cpp
LOG_INFO("Scene '{}' loaded successfully ({} entities)", scene_name, entity_count);
LOG_DEBUG("Entity {}: Playing sound (handle: {})", entity_id, sound_handle);
LOG_ERROR("Failed to load texture: {}", texture_path);
```

**Sortie:** 
- Console (avec couleurs ANSI optionnelles)
- Fichier log
- Thread-safe (mutex interne)

---

### AudioManager

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Audio/AudioManager.hpp`

```cpp
class AudioManager {
private:
  ResourceManager& m_resourceManager;
  SoundPlayer m_soundPlayer;      // Gère les effets sonores
  MusicPlayer m_musicPlayer;      // Gère la musique de fond
  
public:
  explicit AudioManager(ResourceManager& resourceManager);
  
  void playSound(const ID& id);
  void playMusic(const ID& id, bool loop = true);
  void stopMusic();
  
  void setSoundVolume(float volume);
  void setMusicVolume(float volume);
};
```

**Conception:**
- Sons = effets court terme (tirs, explosions, UI)
- Musiques = bande sonore, boucle longue
- Volume séparé par catégorie

---

### EventSystem

**Fichier pattern:** `/home/user/Nova/sdk/include/NovaEngine/Events/Event*.hpp`

**Event:**
```cpp
enum class EventType { Unknown, Input, UI, Engine, Custom };

struct Event {
  EventType type = EventType::Unknown;
  InputEvent inputEvent;      // Si type == Input
  std::string name;           // Nom custom
  std::string payload;        // Données JSON ou string
  
  Event() = default;
  Event(const InputEvent& evt);
  Event(EventType type, const std::string& name, const std::string& payload = "");
};
```

**EventHandler (abstract base):**
```cpp
class EventHandler {
public:
  virtual ~EventHandler() = default;
  virtual void onEvent(const Event& event) = 0;
};
```

**EventDispatcher:**
```cpp
class EventDispatcher {
private:
  std::vector<EventHandler*> m_handlers;
  
public:
  void subscribe(EventHandler* handler);
  void unsubscribe(EventHandler* handler);
  void dispatch(const Event& event);
  void clear();
};
```

**Pattern d'utilisation:**
```cpp
class MyComponent : public EventHandler {
  void onEvent(const Event& event) override {
    if (event.type == EventType::Input) {
      if (event.inputEvent.type == InputEventType::KeyPressed) {
        // Gère touche
      }
    }
    else if (event.type == EventType::UI) {
      if (event.name == "button_clicked") {
        // Gère clic
      }
    }
  }
};
```

---

## APPLICATION FRAMEWORK

### Application Class

**Fichier:** `/home/user/Nova/sdk/include/NovaEngine/Core/Application.hpp`

```cpp
class Application {
public:
  struct Config {
    String windowTitle = "NovaEngine Application";
    u32 windowWidth = 1920, windowHeight = 1080;
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
  
  int run();  // Boucle principale
  void quit();
  float getDeltaTime() const;
  const Config& getConfig() const;
  bool isInitialized() const;
  
protected:
  // Hooks implémentés par sous-classes
  virtual bool onInitialize() = 0;      // Appel une fois au démarrage
  virtual void onUpdate(float deltaTime) = 0;    // Appel chaque frame
  virtual void onRender() = 0;         // Appel chaque frame
  virtual void onEvent(const Event& event) { }    // Optionnel
  virtual void onShutdown() { }        // Optionnel
  
private:
  bool initializeEngine();
  void runMainLoop();
  void processEvents();
  void shutdownEngine();
};
```

**Template for Créer une App:**
```cpp
class MyGame : public Application {
public:
  MyGame() : Application(Config{
    "My Game",
    1920, 1080,
    false, 60, true, Color::Black
  }) {}
  
  bool onInitialize() override {
    // Charge ressources, initalise systèmes
    sceneManager.initialize();
    sceneManager.loadScene("assets/scenes/level1.json", "level1");
    sceneManager.setActiveScene("level1");
    return true;
  }
  
  void onUpdate(float deltaTime) override {
    // Update game logic
    sceneManager.update(deltaTime);
    player.update(deltaTime);
  }
  
  void onRender() override {
    // Render
    sceneManager.render();
  }
  
  void onEvent(const Event& event) override {
    if (event.type == EventType::Input) {
      if (event.inputEvent.key.code == KeyCode::Escape) {
        quit();
      }
    }
  }
};

int main() {
  MyGame app;
  return app.run();  // Lance boucle principale
}
```

**Flux run():**
```
1. initializeEngine():
   - BackendManager::initialize(SFML, ...)
   - setVSync, frameRateLimit
2. onInitialize():
   - Votre logique d'initialisation
3. runMainLoop():
   Tant que fenêtre ouverte:
     - processEvents()
     - onUpdate(deltaTime)
     - clear()
     - onRender()
     - display()
4. onShutdown()
5. shutdownEngine()
```

---

## FLUX D'EXÉCUTION DÉTAILLÉ

### Frame Principal
```
Application::run()
  └─ initializeEngine()
     └─ BACKEND().initialize(SFML, width, height, ...)
        └─ Crée SFMLWindowBackend, SFMLGraphicsBackend, etc.
  
  └─ onInitialize()  [Implémentation user]
     └─ SceneManager::initialize()
        └─ Charge DefinitionManager (Sprites.json, etc.)
        └─ Charge SceneGraph (scenegraph.json)
     └─ SceneManager::loadScene()
        └─ Scene::loadFromJSON()
           └─ Pour chaque entité JSON:
              └─ createEntity() → EntityRegistry
              └─ Ajoute composants selon type
     └─ SceneManager::setActiveScene()

  └─ runMainLoop()  [Boucle principale]
     Tant que WINDOW().isOpen():
       ├─ processEvents()
       │  └─ INPUT().pollEvent()
       │     └─ Pour chaque InputEvent:
       │        └─ onEvent(Event(inputEvent))
       │
       ├─ onUpdate(deltaTime)  [User]
       │  └─ Logique jeu
       │  └─ SceneManager::update(deltaTime)
       │     └─ Pour chaque scène active:
       │        └─ Scene::update(deltaTime)
       │           └─ Pour chaque System en ordre:
       │              └─ System::update(deltaTime, registry)
       │                 ├─ RenderSystem: GRAPHICS().drawSprite()
       │                 ├─ AnimationSystem: update frames
       │                 ├─ ActivatorSystem: check zones
       │                 ├─ AudioSystem: play sounds
       │                 ├─ JourneySystem: move NPCs
       │                 └─ ...
       │
       ├─ WINDOW().clear(clearColor)
       │
       ├─ onRender()  [User]
       │  └─ SceneManager::render()
       │     └─ activeScene->render()
       │        └─ WINDOW().clear() [redondant]
       │
       ├─ UIManager::render()
       │  └─ Sort par layer
       │  └─ Pour chaque component:
       │     └─ component->render()
       │        └─ GRAPHICS().drawRect/drawText/etc
       │
       └─ WINDOW().display()
  
  └─ onShutdown()  [User]
  
  └─ shutdownEngine()
     └─ BACKEND().shutdown()
```

### Cycle Entité-Composant-Système

**Exemple: Sprite animé qui se déplace**

```cpp
// Definition (asset)
Entity entity_0 {
  TransformComponent { position: (100, 200), rotation: 0, scale: (1, 1) },
  SpriteComponent { textureID: "player_idle", tint: White, zOrder: 10 },
  AnimationComponent { animationID: "player_walk", playing: true, loop: true },
  ColliderComponent { type: Box, size: (32, 32) }
}

// Frame N
RenderSystem:
  - Récupère {Transform, Sprite} de entity_0
  - Crée SpriteData(texture_handle, position: 100,200, rotation: 0, ...)
  - Appelle GRAPHICS().drawSprite(spriteData)
  - → Texture dessinée à l'écran

AnimationSystem:
  - Récupère {Sprite, Animation}
  - anim->currentTime += deltaTime
  - Si frameTime dépassé:
    - currentFrame++
    - sprite->textureRect = frames[currentFrame]

PhysicsSystem:
  - Récupère {Transform, Collider}
  - Teste collisions avec autres entités
  - Log collisions

// Entre frames:
// Player input déplace entity:
entity_0.getComponent<TransformComponent>()->position += velocity * deltaTime;

// Frame N+1
RenderSystem:
  - SpriteData(position: 105, 210, ...)  // Nouvelle position!
  - GRAPHICS().drawSprite()
```

---

## FORMATS JSON DÉTAILLÉS

### Format Définition de Sprite

**Fichier:** `assets/data/definitions/Sprites.json`

```json
{
  "sprites": [
    {
      "id": "wall_01",
      "texture": "wall_01_texture",
      "texturePath": "assets/textures/walls/wall_01.png",
      "textureRect": [0, 0, 64, 64],
      "size": [64, 64],
      "origin": [32, 32],
      "zOrder": 0
    },
    {
      "id": "player_idle",
      "texture": "player_texture",
      "texturePath": "assets/textures/player.png",
      "textureRect": [0, 0, 32, 48],
      "size": [32, 48],
      "origin": [16, 24],
      "zOrder": 20
    }
  ]
}
```

### Format Définition d'Animation

**Fichier:** `assets/data/definitions/Animations.json`

```json
{
  "animations": [
    {
      "id": "player_walk",
      "frames": [
        [0, 0, 32, 48],      // Frame 0: rect (left, top, width, height)
        [32, 0, 32, 48],     // Frame 1
        [64, 0, 32, 48],     // Frame 2
        [96, 0, 32, 48]      // Frame 3
      ],
      "frameDuration": 0.1,
      "loop": true
    }
  ]
}
```

### Format Scène

**Fichier:** `assets/data/scenes/interior_1.json`

```json
{
  "name": "interior_1",
  "type": "interior",
  "backgroundColor": [32, 32, 64, 255],
  
  "pathfinding": {
    "waypoints": [
      {"id": "entrance", "position": [100, 200], "tags": ["entry"]},
      {"id": "fountain", "position": [640, 360], "tags": ["landmark"]},
      {"id": "exit", "position": [1200, 200], "tags": ["exit"]}
    ],
    "connections": [
      {"from": "entrance", "to": "fountain", "bidirectional": true, "tags": ["main_path"]},
      {"from": "fountain", "to": "exit", "bidirectional": true, "tags": ["main_path"]}
    ]
  },
  
  "entities": [
    {
      "type": "sprite",
      "position": [0, 0],
      "spriteID": "wall_01",
      "scale": [2, 2]
    },
    {
      "type": "animated_sprite",
      "position": [640, 200],
      "spriteID": "player_idle",
      "animationID": "player_walk"
    },
    {
      "type": "activator",
      "position": [100, 200],
      "activatorID": "door_1",
      "targetTag": "player"
    },
    {
      "type": "player",
      "position": [500, 300],
      "spriteID": "player_idle"
    }
  ]
}
```

### Format SceneGraph

**Fichier:** `assets/data/scenegraph.json`

```json
{
  "connections": [
    {
      "from": "interior_1",
      "to": "exterior_main",
      "exitPortal": [100, 200],
      "entryPortal": [500, 500],
      "travelTime": 5.0,
      "bidirectional": true
    },
    {
      "from": "exterior_main",
      "to": "interior_2",
      "exitPortal": [1200, 400],
      "entryPortal": [100, 400],
      "travelTime": 3.0,
      "bidirectional": true
    }
  ]
}
```

### Format UI

**Fichier:** `assets/data/ui/main_menu.json`

```json
{
  "name": "main_menu",
  "uiID": "main_menu_ui",
  "language": "en",
  "layers": 3,
  
  "buttons": [
    {
      "id": "btn_start",
      "position": [960, 300],
      "size": [300, 80],
      "text": "Start Game",
      "fontSize": 32,
      "textColor": [255, 255, 255, 255],
      "action": "start_game",
      "layer": 2
    },
    {
      "id": "btn_quit",
      "position": [960, 500],
      "size": [300, 80],
      "text": "Quit",
      "fontSize": 32,
      "action": "quit_game",
      "layer": 2
    }
  ],
  
  "texts": [
    {
      "id": "title",
      "position": [960, 100],
      "text": "Main Menu",
      "fontSize": 72,
      "textColor": [255, 200, 100, 255],
      "layer": 2
    }
  ],
  
  "images": [
    {
      "id": "bg",
      "position": [0, 0],
      "size": [1920, 1080],
      "texture": "menu_background",
      "layer": 0
    }
  ]
}
```

### Format Configuration

**Fichier:** `config/engine.ini`

```ini
[Display]
width=1920
height=1080
fullscreen=false
vsync=true
frameRateLimit=60
antialiasingLevel=0
nativeWidth=3840
nativeHeight=2160

[Audio]
masterVolume=100.0
musicVolume=80.0
soundVolume=90.0
muteAll=false
audioDevice=default

[Input]
mouseSensitivity=1.0
invertMouse=false
keyBindings=move_left:A:Left,move_right:D:Right

[Debug]
enableLogging=true
logLevel=INFO
logFile=logs/nova.log
showFPS=false
showDebugInfo=false
enableProfiler=false

[Game]
language=en
playerName=Player
autoSave=true
autoSaveInterval=300
savePath=saves/
```

---

## POINTS CLÉS D'ARCHITECTURE

### 1. Abstraction Backend
- **Benefit:** Portable (SFML maintenant, SDL plus tard)
- **Pattern:** Singleton + interfaces abstraites
- **Utilisation:** Macros GRAPHICS(), WINDOW(), etc.

### 2. ECS Pattern
- **Flexibility:** Composants découplés, logique dans systems
- **Queryable:** `getEntitiesWith()` pour ECS queries
- **Extensible:** Ajouter nouveau component/system facilement

### 3. Two-Tier JSON System
- **Tier 1:** Definitions (load une fois au démarrage)
- **Tier 2:** Scenes (ref definitions par ID)
- **Benefit:** Réutilisabilité, pas duplication

### 4. Multi-Scene Travel
- **Unique:** NPCs traversent physiquement scènes
- **Via:** JourneySystem + SceneGraph
- **Permet:** Joueur voit NPCs traverser

### 5. Organized Rendering
- **Z-Order:** Sprites triés avant render
- **Layers:** UI avec layer system
- **Systems:** Animation avant render (ordering importanteur)

### 6. Configuration Centralisée
- **ConfigManager:** Singleton gère toutes les settings
- **Macros:** DISPLAY_CONFIG, AUDIO_CONFIG, etc.
- **File-based:** INI persiste entre runs

---

## EXTENSIONS FUTURES

Cette architecture permet facile :
- Ajouter nouveau Component type
- Ajouter nouveau System
- Swapper SFML pour SDL
- Ajouter dialogue system
- Ajouter save/load
- Ajouter network multiplayer
- Ajouter particle systems
- Intégrer Box2D physics
- Ajouter shader support avancé

