# INDEX DES FICHIERS CLÉS - NovaEngine

## COMPOSANTS ECS

| Fichier | Classe | Responsabilité | Ligne |
|---------|--------|-----------------|-------|
| Components.hpp | `TransformComponent` | Position, rotation, scale | 11-49 |
| Components.hpp | `SpriteComponent` | Rendu 2D texturé | 51-103 |
| Components.hpp | `AnimationComponent` | Animation frame-based | 169-215 |
| Components.hpp | `LightComponent` | Lumières (point/directionnel/spot) | 109-166 |
| Components.hpp | `ColliderComponent` | Collisions AABB/cercle | 220-260 |
| Components.hpp | `AudioComponent` | Lecture son/musique | 266-296 |
| Components.hpp | `ActivatorComponent` | Zones de déclenchement | 308-400 |
| Components.hpp | `TagComponent` | Identification entité | 410-425 |
| Components.hpp | `SceneTransitionComponent` | Transitions scène | 436-464 |
| Components.hpp | `JourneyComponent` | Multi-scène NPC travel | 476-559 |

---

## SYSTÈMES ECS

| Fichier | Classe | Responsabilité | Ligne |
|---------|--------|-----------------|-------|
| Systems.hpp | `RenderSystem` | Dessine tous les sprites | 20-61 |
| Systems.hpp | `AnimationSystem` | Mises à jour animations | 72-112 |
| Systems.hpp | `PhysicsSystem` | Détection collisions | 196-245 |
| Systems.hpp | `ActivatorSystem` | Gestion zones trigger | 258-430 |
| Systems.hpp | `AudioSystem` | Lecture audio | 165-185 |
| Systems.hpp | `LightSystem` | Rendu lumières | 123-156 |
| Systems.hpp | `JourneySystem` | Voyages multi-scènes | 445-694 |

---

## ARCHITECTURE CORE ECS

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| Entity.hpp | `Entity` | Conteneur de composants (ID + composants) |
| Component.hpp | `Component` | Base abstraite pour tous les composants |
| System.hpp | `System` | Base abstraite pour tous les systèmes |
| EntityRegistry.hpp | `EntityRegistry` | Manager d'entités (CRUD + queries) |
| Scene.hpp | `Scene` | Contient registry, systems, waypoints |
| SceneManager.hpp | `SceneManager` | Manager scènes, definitions, sceneGraph |
| DefinitionManager.hpp | `DefinitionManager` | Charge/gère définitions entities (JSON) |
| WaypointGraph.hpp | `WaypointGraph` | Pathfinding local dans scène (BFS) |
| SceneGraph.hpp | `SceneGraph` | Connections entre scènes (BFS multi-scène) |

---

## BACKEND & ABSTRACTION

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| BackendManager.hpp | `BackendManager` | Singleton - gère tous les backends |
| BackendTypes.hpp | Types/Structs | Vec2f, Color, InputEvent, SpriteData, etc. |
| IWindowBackend.hpp | `IWindowBackend` | Interface - fenêtre (abstract) |
| IGraphicsBackend.hpp | `IGraphicsBackend` | Interface - rendu (abstract) |
| IInputBackend.hpp | `IInputBackend` | Interface - input (abstract) |
| IAudioBackend.hpp | `IAudioBackend` | Interface - audio (abstract) |
| IFontBackend.hpp | `IFontBackend` | Interface - fonts (abstract) |
| IResourceBackend.hpp | `IResourceBackend` | Interface - ressources (abstract) |
| IViewportBackend.hpp | `IViewportBackend` | Interface - viewport/camera (abstract) |
| SFMLGraphicsBackend.hpp | `SFMLGraphicsBackend` | Implémentation SFML graphics |
| SFMLWindowBackend.hpp | `SFMLWindowBackend` | Implémentation SFML fenêtre |
| SFMLInputBackend.hpp | `SFMLInputBackend` | Implémentation SFML input |
| SFMLAudioBackend.hpp | `SFMLAudioBackend` | Implémentation SFML audio |
| SFMLFontBackend.hpp | `SFMLFontBackend` | Implémentation SFML fonts |
| SFMLConversions.hpp | Fonctions | Conversions NovaEngine ↔ SFML |

---

## SYSTÈME UI

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| UIManager.hpp | `UIManager` | Orchestrateur central UI (add/remove/update/render) |
| UIComponent.hpp | `UIComponent` | Base abstraite composants UI |
| UILoader.hpp | `UILoader` | Parse JSON UI et crée composants |
| Button.hpp | `Button` | Bouton interactif (normal/hover/pressed) |
| Text.hpp | `Text` | Texte statique/dynamique |
| Image.hpp | `Image` | Image/texture UI |
| Panel.hpp | `Panel` | Boîte colorée (conteneur visuel) |
| Slider.hpp | `Slider` | Curseur (0.0-1.0) |
| TextInput.hpp | `TextInput` | Champ texte interactif |
| Animation.hpp | `Animation` | Animation UI (spritesheet) |

---

## SYSTÈMES PRINCIPAUX

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| ResourceManager.hpp | `ResourceManager` | Manager ressources (textures, fonts, sons, musiques) |
| ConfigManager.hpp | `ConfigManager` | Singleton - gestion configuration INI |
| Logger.hpp | `Logger` | Singleton - logging console + fichier |
| AudioManager.hpp | `AudioManager` | Manager audio (sons + musique) |
| SoundPlayer.hpp | `SoundPlayer` | Gestion effets sonores |
| MusicPlayer.hpp | `MusicPlayer` | Gestion musique de fond |

---

## ÉVÉNEMENTS

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| Event.hpp | `Event` | Structure événement (type + données) |
| EventHandler.hpp | `EventHandler` | Base abstraite - onEvent() |
| EventDispatcher.hpp | `EventDispatcher` | Dispatche événements aux handlers |

---

## APPLICATION FRAMEWORK

| Fichier | Classe | Responsabilité |
|---------|--------|-----------------|
| Application.hpp | `Application` | Base pour appli - implémente boucle principale |
| Game.hpp | `Game` | ? (à explorer) |
| NovaEngine.hpp | Exports | Exports principaux du moteur |
| Types.hpp | Types | Types personnalisés du moteur |

---

## STRUCTURES DE DONNÉES CLÉS

### Vec2f (2D Vector)
```cpp
struct Vec2f {
  f32 x, y;
  // Opérateurs: +, -, *, /, +=, -=, *=, /=
};
```

### Color
```cpp
struct Color {
  u8 r, g, b, a;
  static const Color Black, White, Red, Green, Blue, Yellow, Transparent;
};
```

### SpriteData (pour rendu)
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
```

### TextData (pour texte)
```cpp
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

### InputEvent (input)
```cpp
struct InputEvent {
  InputEventType type;  // Closed, Resized, KeyPressed, MouseMoved, etc.
  union {
    struct { u32 width, height; } size;
    struct { KeyCode code; bool alt, control, shift, system; } key;
    struct { MouseButton button; i32 x, y; } mouseButton;
    struct { i32 x, y; } mouseMove;
  };
};
```

---

## MACROS IMPORTANTES

### Backend Access Macros
```cpp
#define WINDOW()    BackendManager::get().window()
#define INPUT()     BackendManager::get().input()
#define GRAPHICS()  BackendManager::get().graphics()
#define RESOURCES() BackendManager::get().resources()
#define AUDIO()     BackendManager::get().audio()
#define FONTS()     BackendManager::get().fonts()
#define VIEWPORT()  BackendManager::get().viewport()
```

### Configuration Access Macros
```cpp
#define DISPLAY_CONFIG ConfigManager::getInstance().getDisplayConfig()
#define AUDIO_CONFIG   ConfigManager::getInstance().getAudioConfig()
#define INPUT_CONFIG   ConfigManager::getInstance().getInputConfig()
#define DEBUG_CONFIG   ConfigManager::getInstance().getDebugConfig()
#define GAME_CONFIG    ConfigManager::getInstance().getGameConfig()
```

### Logging Macros
```cpp
#define LOG_TRACE(format, ...)  Logger::getInstance().logf(LogLevel::Trace, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FATAL(format, ...)
```

### Component Type ID
```cpp
#define COMPONENT_TYPE_ID(TypeName) \
  ComponentTypeID getTypeID() const override { return #TypeName; }
```

---

## FICHIERS DE CONFIGURATION / ASSETS

### Definition Files (Chargés une fois au démarrage)
- `assets/data/definitions/Sprites.json` - Définitions sprites
- `assets/data/definitions/Lights.json` - Définitions lumières
- `assets/data/definitions/Animations.json` - Définitions animations
- `assets/data/definitions/Audio.json` - Définitions audio
- `assets/data/definitions/Activators.json` - Définitions activators

### Scene Files
- `assets/data/scenes/[scene_name].json` - Fichiers scènes

### Meta Configs
- `assets/data/scenegraph.json` - Graphe de connexions scènes
- `config/engine.ini` - Configuration moteur (display, audio, input, debug, game)

### UI Files
- `assets/data/ui/[ui_name].json` - Définitions UI (menus, HUD, etc.)

---

## HIÉRARCHIE D'HÉRITAGE

### ECS Hierarchy
```
Component (abstract)
  ├─ TransformComponent
  ├─ SpriteComponent
  ├─ AnimationComponent
  ├─ LightComponent
  ├─ ColliderComponent
  ├─ AudioComponent
  ├─ ActivatorComponent
  ├─ TagComponent
  ├─ SceneTransitionComponent
  └─ JourneyComponent

System (abstract)
  ├─ RenderSystem
  ├─ AnimationSystem
  ├─ PhysicsSystem
  ├─ ActivatorSystem
  ├─ AudioSystem
  ├─ LightSystem
  └─ JourneySystem
```

### Backend Hierarchy
```
IWindowBackend (abstract)
  └─ SFMLWindowBackend

IGraphicsBackend (abstract)
  └─ SFMLGraphicsBackend

IInputBackend (abstract)
  └─ SFMLInputBackend

IAudioBackend (abstract)
  └─ SFMLAudioBackend

IFontBackend (abstract)
  └─ SFMLFontBackend

IResourceBackend (abstract)
  └─ [Implementation depends on ResourceManager]

IViewportBackend (abstract)
  └─ [Implementation depends on camera system]
```

### UI Hierarchy
```
EventHandler (abstract)
  ├─ UIComponent (abstract)
  │   ├─ Button
  │   ├─ Text
  │   ├─ Image
  │   ├─ Panel
  │   ├─ Slider
  │   ├─ TextInput
  │   └─ Animation
  └─ [Other event handlers]
```

### Application Hierarchy
```
Application (abstract)
  └─ [User-defined game class]
       ├─ onInitialize()
       ├─ onUpdate()
       ├─ onRender()
       ├─ onEvent()
       └─ onShutdown()
```

---

## SINGLETONS DU MOTEUR

1. **BackendManager** - Accès unique aux backends (GRAPHICS(), WINDOW(), etc.)
2. **Logger** - Logging centralisé
3. **ConfigManager** - Configuration moteur
4. **Application** - Base de l'application (dérivée)

---

## PATTERNS DE CONCEPTION UTILISÉS

1. **Singleton** - BackendManager, Logger, ConfigManager
2. **Factory** - DefinitionManager, UILoader (création d'objets complexes)
3. **Strategy** - Backends abstraits (SFML impl., SDL impl. possible)
4. **Observer** - EventDispatcher/EventHandler
5. **Component** - ECS (Entity-Component-System)
6. **Data-Driven** - JSON pour définitions et configurations

---

## CHEMINS CLÉS DU CODE

### ECS Query (Fondamental!)
```cpp
auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});
// Retourne toutes les entités avec BOTH components
```

### Backend Access
```cpp
GRAPHICS().drawSprite(spriteData);  // Appel backend
AUDIO().playSound(soundHandle);
```

### Component Access
```cpp
auto* transform = entity->getComponent<TransformComponent>();
transform->position += velocity;
```

### Scene Update Loop
```
SceneManager::update(deltaTime)
  ├─ Scene::update(deltaTime) [pour chaque scène active]
  │   ├─ AnimationSystem::update()
  │   ├─ PhysicsSystem::update()
  │   ├─ ActivatorSystem::update()
  │   ├─ AudioSystem::update()
  │   ├─ LightSystem::update()
  │   └─ RenderSystem::update()
  └─ JourneySystem::updateTransferredEntities() [après transitions]
```

---

## POINTS IMPORTANTS À RETENIR

1. **ECS est la base** - Tout est Entity + Components + Systems
2. **JSON deux-tiers** - Définitions (tier 1) + Scènes (tier 2) qui réfèrent définitions
3. **Backend abstrait** - Facile de passer SFML → SDL
4. **Multi-scène travel** - NPCs traversent PHYSIQUEMENT les scènes (pas teleport)
5. **Waypoints** - Pathfinding local dans scènes + SceneGraph pour multi-scène
6. **UI séparé** - UIManager gère UI indépendamment du ECS
7. **Rendering order** - Systems exécutés en ordre (Animation avant Render!)
8. **Configuration centralisée** - ConfigManager pour tous les settings

---

## POUR DÉMARRER UNE DOCUMENTATION

**Suggestion d'ordre de lecture :**
1. Lire ce fichier (INDEX) - Vue d'ensemble
2. Lire ARCHITECTURE_REPORT.md - Détails complets
3. Puis par catégorie :
   - ECS : Entity, Component, System, EntityRegistry, Scene, SceneManager
   - Backend : BackendManager, BackendTypes, IGraphicsBackend, SFMLGraphicsBackend
   - UI : UIManager, UIComponent, Button, Text, etc.
   - Core : Application, Logger, ConfigManager, ResourceManager
   - Systèmes : AnimationSystem, RenderSystem, JourneySystem, etc.

