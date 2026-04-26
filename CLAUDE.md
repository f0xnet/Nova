# CLAUDE.md — Guide de travail sur NovaEngine

> Document interne destiné à Claude pour travailler efficacement sur le moteur. Concentré sur la **partie client/moteur** (le serveur est ignoré).
>
> Pour la doc technique exhaustive (API, exemples détaillés), voir `NOVAENGINE_DOCUMENTATION.md`.

---

## 1. Vue d'ensemble

**NovaEngine** est un moteur de jeu **2D** écrit en **C++17**, reposant sur **SFML 2.6** via une couche d'abstraction backend. Le projet inclut :

- **`sdk/include/NovaEngine/`** — headers du moteur (API publique, ~70 fichiers)
- **`client/src/`** — implémentation .cpp du moteur + jeu d'exemple (RPG)
- **`client/assets/`** — données de jeu (JSON, shaders, sprites, sons, fonts)
- **`client/bin/Release/`** — binaire (`Nova.exe`) + `config/engine.ini` + `data/` (assets copiés)

### Caractéristiques clés

| Aspect | Détail |
|---|---|
| Langage | C++17, GLSL, JSON |
| Graphics / Audio / Input | SFML 2.6 (statique, via abstraction `IXxxBackend`) |
| Architecture | ECS (Entity-Component-System) + Backend abstrait + Pipeline post-process |
| Données | JSON (nlohmann::json) sur 2 tiers : définitions (templates) + scènes (instances) |
| Config | INI (`config/engine.ini`) parsé par `ConfigManager` |
| Build | `compile_client.sh` (Linux) / `client__compile.bat` (Windows + manifest DPI) |
| Plateforme cible | Windows principalement, build Linux supporté |

### Les 5 piliers architecturaux

1. **Backend abstrait** — `IGraphicsBackend`, `IAudioBackend`, etc. → impl SFML, swappable.
2. **ECS** — `Entity` (id + map de components), `Component` (data, sérialisable JSON), `System` (logique).
3. **Scene/SceneManager** — chargent JSON, instancient les entités via `DefinitionManager`.
4. **Post-processing pipeline** — chaîne d'effets shader (SSAO → Bloom → ColorGrading → DynamicLighting).
5. **UI déclaratif** — `UILoader` parse JSON → `UIManager` orchestre Buttons / Panels / Texts / Inputs.

---

## 2. Arborescence (focus client/moteur)

```
/home/user/Nova/
├── CLAUDE.md                          ← CE FICHIER (mémoire de travail)
├── NOVAENGINE_DOCUMENTATION.md        ← doc technique exhaustive (3367 lignes, FR)
├── README.md                          ← vitrine publique
├── compile_client.sh                  ← build Linux
├── client__compile.bat                ← build Windows (5 étapes : manifest, RH, compile, link, embed)
├── server__compile.bat                ← (ignoré)
│
├── sdk/
│   ├── include/NovaEngine/            ← API PUBLIQUE
│   │   ├── Game.hpp                   ← classe Game (exemple, héritée par client/main.cpp)
│   │   ├── Core/
│   │   │   ├── Application.hpp        ← classe de base, boucle principale
│   │   │   ├── ConfigManager.hpp      ← INI parser (Display/Audio/Input/Debug/Game configs)
│   │   │   ├── Logger.hpp             ← LOG_TRACE/DEBUG/INFO/WARN/ERROR/FATAL macros
│   │   │   ├── Types.hpp              ← i32, u64, f32, String, ID, Ref<T>, Unique<T>
│   │   │   └── NovaEngine.hpp
│   │   ├── Backend/
│   │   │   ├── BackendManager.hpp     ← singleton; macros BACKEND() WINDOW() INPUT() GRAPHICS() etc.
│   │   │   ├── Core/BackendTypes.hpp  ← Vec2f/Vec2i/Vec3f, Color, Rect, IntRect, KeyCode,
│   │   │   │                             InputEvent, SpriteData, RectData, TextData,
│   │   │   │                             ViewportData, Handles (TextureHandle, etc.)
│   │   │   ├── Interfaces/            ← 7 interfaces : Window / Input / Graphics / Resource /
│   │   │   │                             Audio / Font / Viewport
│   │   │   └── SFML/                  ← impl SFML des 7 interfaces + helpers/conversions
│   │   ├── ECS/
│   │   │   ├── ECS.hpp                ← header agrégateur, à inclure depuis le jeu
│   │   │   ├── Component.hpp          ← base Component + macro COMPONENT_TYPE_ID(Name)
│   │   │   ├── Entity.hpp             ← Entity (u64 id + map<TypeID, unique_ptr<Component>>)
│   │   │   ├── Components.hpp         ← 11 components built-in (voir §4)
│   │   │   ├── System.hpp             ← interface System
│   │   │   ├── Systems.hpp            ← 7 systems built-in (voir §4)
│   │   │   ├── EntityRegistry.hpp     ← container d'entities, requêtes par components
│   │   │   ├── DefinitionManager.hpp  ← charge tier 1 (templates JSON)
│   │   │   ├── Scene.hpp              ← loadFromJSON + update/render, owne EntityRegistry + WaypointGraph
│   │   │   ├── SceneManager.hpp       ← orchestre N scènes, scene active + scènes en transit
│   │   │   ├── SceneGraph.hpp         ← graphe inter-scènes (pour voyages NPC multi-scènes)
│   │   │   └── WaypointGraph.hpp      ← pathfinding intra-scène
│   │   ├── UI/
│   │   │   ├── UIManager.hpp          ← gère composants par layers/groupes/UIs, dispatch events
│   │   │   ├── UILoader.hpp           ← parse JSON UI → instancie composants
│   │   │   ├── UIComponent.hpp        ← base abstraite (hérite EventHandler)
│   │   │   └── Components/            ← Button, Text, TextInput, Panel, Image, Slider, Animation
│   │   ├── Rendering/
│   │   │   ├── PostProcessPipeline.hpp ← chaîne d'effets, ping-pong de RenderTextures
│   │   │   ├── PostProcessEffect.hpp   ← interface effet (init/apply/shutdown)
│   │   │   ├── PostProcessManager.hpp  ← (legacy, déprécié → utiliser Pipeline)
│   │   │   └── Effects/                ← CRT, Bloom, SSAO, ColorGrading, DynamicLighting,
│   │   │                                  Passthrough, LightData
│   │   ├── Events/                    ← Event (union Input/UI/Engine/Custom), EventHandler, EventDispatcher
│   │   ├── Resources/                 ← ResourceManager (textures/fonts/sons), ResourceTypes
│   │   ├── Audio/                     ← AudioManager / SoundPlayer / MusicPlayer (legacy, AUDIO() préféré)
│   │   └── Systems/LightingSystem.hpp ← collecte LightComponents → DynamicLightingEffect
│   │
│   ├── SFML-2.6-2.1/                  ← libs SFML précompilées (lib/, include/)
│   ├── MinGW/                         ← toolchain Windows (g++, windres, ld, etc.)
│   └── libs/                          ← scripts CMake (peu utilisés ici)
│
├── client/
│   ├── main.cpp                       ← entry : Logger setLevel(Trace) ; Game().run()
│   ├── src/
│   │   ├── Game.cpp                   ← orchestration : viewport letterbox, SceneManager, UI,
│   │   │                                 PostProcessPipeline, LightingSystem, dialogue, controls
│   │   ├── Core/                      ← .cpp pour ConfigManager, Logger, NovaEngine
│   │   ├── Backend/                   ← .cpp pour BackendManager + impls SFML*
│   │   ├── ECS/                       ← (vide — ECS est header-only sauf SceneManager côté SDK)
│   │   ├── UI/                        ← UIManager.cpp, UILoader.cpp, UIComponent.cpp + Components/
│   │   ├── Rendering/                 ← PostProcessPipeline.cpp + Effects/
│   │   ├── Resources/                 ← ResourceManager.cpp, ResourceTypes.cpp
│   │   ├── Events/                    ← Event.cpp, EventDispatcher.cpp, EventHandler.cpp
│   │   ├── Audio/                     ← AudioManager.cpp, MusicPlayer.cpp, SoundPlayer.cpp
│   │   ├── Systems/                   ← LightingSystem.cpp
│   │   ├── Dialogue/                  ← DialogueComponent.hpp, DialogueSystem.{hpp,cpp}
│   │   │                                  (composant CUSTOM, pas dans le SDK)
│   │   └── Player/                    ← PlayerController.{hpp,cpp}
│   │
│   ├── assets/                        ← copié vers bin/Release/data/ au runtime
│   │   ├── data/
│   │   │   ├── definitions/           ← Tier 1 — Sprites/NPCs/Lights/Animations/Audio/Activators.json
│   │   │   ├── scenes/                ← Tier 2 — ville.json, taverne.json, maison_bob.json,
│   │   │   │                              test.json, test_scene.json, activator_test_scene.json
│   │   │   └── scenegraph.json        ← connexions inter-scènes (portails)
│   │   ├── shaders/                   ← .vert/.frag : crt, ssao, bloom, color_grading,
│   │   │                                  dynamic_lighting, passthrough, blur, simple_test
│   │   ├── ui/json/                   ← loginmenu.json, dialogue.json
│   │   └── dialogs/rin/rin_care_01.json
│   │
│   ├── icon/                          ← appicon.rc / .res (Windows resource)
│   ├── app.manifest                   ← DPI-aware manifest (généré par .bat)
│   ├── bin/Release/                   ← binaire prod (gitignored)
│   │   ├── Nova.exe
│   │   ├── openal32.dll
│   │   ├── config/engine.ini
│   │   ├── data/                      ← copie de client/assets/data
│   │   └── logs/
│   └── obj/Release/                   ← .o objets (gitignored, incremental)
│
└── tools/ResourceHacker/              ← embed manifest DPI dans .exe (Windows)
```

---

## 3. Build & exécution

### Linux
```bash
bash /home/user/Nova/compile_client.sh
# → produit client/bin/Release/NovaEngine (statique)
```
Étapes : compile chaque `.cpp` sous `client/src/` + `client/main.cpp` → link avec `sfml-*-s` + OpenGL/freetype/openal/FLAC/vorbis. Flags : `-DSFML_STATIC -std=c++17 -O2 -Wall`.

⚠️ Le script Linux référence des libs Windows (`-lopengl32`, `-lwinmm`, `-lgdi32`) — **il a été conçu pour cross-compiler vers Windows**. Pour un vrai build Linux natif, remplacer par `-lGL`, `-lpthread`, `-lX11`.

### Windows
```bat
client__compile.bat
```
5 étapes : (1) génère `app.manifest` DPI, (2) installe ResourceHacker si absent, (3) compile incrémental (skip si .o à jour), (4) link, (5) `ResourceHacker -addoverwrite` pour embarquer le manifest. Toolchain : `sdk/MinGW/bin/g++.exe`. Flags : `-O0 -DNDEBUG -DSFML_STATIC -std=c++17`.

### Lancement
```bash
cd /home/user/Nova/client/bin/Release
./Nova.exe   # (ou ./NovaEngine sur Linux)
```
**Le binaire DOIT être lancé depuis `bin/Release/`** : tous les chemins (config, assets, shaders) sont relatifs au cwd (`config/engine.ini`, `data/...`, etc.).

### Contrôles in-game
- WASD / flèches — déplacement
- E — interagir / avancer dialogue
- T — avancer le temps de 1h
- 1 / 2 / 3 / 4 — toggle SSAO / Bloom / ColorGrading / DynamicLighting
- ESC — quitter

---

## 4. Boucle d'initialisation (dans l'ordre)

```
client/main.cpp
  └─ Logger::setLogLevel(Trace)
  └─ Game game;                              ← Game::Game() (Game.cpp:7)
       ├─ createConfig() : charge config/engine.ini via ConfigManager → Application::Config
       └─ alloue DialogueSystem, PlayerController, LightingSystem (ptrs nuls pour effets)
  └─ game.run()                              ← Application::run() (Application.hpp:54)
       ├─ initializeEngine() : BACKEND().initialize(SFML, w, h, title, fullscreen)
       │                       + WINDOW().setVSync + setFramerateLimit
       ├─ onInitialize()                     ← Game::onInitialize() (Game.cpp:26)
       │   1. Calcule viewport letterbox/pillarbox (rapport logique vs fenêtre)
       │   2. VIEWPORT().setView(viewData)
       │   3. SceneManager::initialize("data/definitions/", "data/scenegraph.json")
       │      → DefinitionManager::loadDefinitions() (charge Sprites/NPCs/Lights/...)
       │      → SceneGraph::loadFromJSON()
       │   4. SceneManager::loadScene("data/scenes/test.json", "test")
       │   5. SceneManager::setActiveScene("test")
       │   6. Cherche entité avec TagComponent.tag == "player" → PlayerController::setPlayerID()
       │   7. UIManager::setActionCallback(handleUIAction)
       │   8. UILoader::loadFromFile("data/ui/json/dialogue.json", uiManager)
       │   9. DialogueSystem::initialize(&uiManager)
       │  10. PostProcessPipeline::initialize(logicalW, logicalH)
       │      → addEffect<SSAOEffect>(), <BloomEffect>, <ColorGradingEffect>, <DynamicLightingEffect>
       │      → m_lightingSystem->setLightingEffect(dynamicLighting), setTimeOfDay(0.25 = 6h)
       │
       ├─ runMainLoop() :  while WINDOW().isOpen()
       │   ├─ processEvents() → onEvent() (UI dispatch + game keys)
       │   ├─ onUpdate(dt)   ← Game::onUpdate
       │   │   - PlayerController::updateMovement(scene, dt, !dialogue.isActive())
       │   │   - PlayerController::updateNPCDetection(scene)
       │   │   - VIEWPORT().setViewCenter(playerPos)   ← caméra suit joueur
       │   │   - LightingSystem::update(dt, registry)  ← ECS lights → DynamicLightingEffect
       │   │   - DialogueSystem::showNPCIndicator(nearestNPC && !dialogue.isActive())
       │   │   - SceneManager::update(dt)              ← active + scènes sur path actif
       │   │   - UIManager::update(dt)
       │   ├─ WINDOW().clear(clearColor)
       │   └─ onRender()     ← Game::onRender
       │       - PostProcessPipeline::beginSceneRender()  (bind RT)
       │       - SceneManager::render()                   (active scene → systems → RenderSystem)
       │       - PostProcessPipeline::endSceneRender(0.016f)  ← /!\ HARDCODED, voir §10
       │       - VIEWPORT().resetView()                    ← UI sans offset caméra
       │       - UIManager::render()
       │
       └─ onShutdown() → BACKEND().shutdown()
```

---

## 5. ECS — Entity-Component-System

### Modèle conceptuel
- **Entity** = `u64 id` + `unordered_map<ComponentTypeID, unique_ptr<Component>>`. Pas de logique.
- **Component** = data pure + `serialize/deserialize(json)`. Identifié par une string (le nom de la classe via `COMPONENT_TYPE_ID(MyComp)`).
- **System** = logique stateless (idéalement) qui itère sur des entités via `registry.getEntitiesWith({"TransformComponent", "..."})`.
- **EntityRegistry** = dictionnaire d'entités, requêtable par set de components.
- **Scene** = owne 1 EntityRegistry + N Systems + 1 WaypointGraph (pathfinding intra-scène).
- **SceneManager** = owne N Scenes + 1 DefinitionManager + 1 SceneGraph (inter-scènes).

### Components built-in (`sdk/include/NovaEngine/ECS/Components.hpp`)

| Component | Champs principaux | Notes |
|---|---|---|
| `TransformComponent` | `position`, `rotation`, `scale`, `origin` | requis pour tout ce qui est dans le monde |
| `SpriteComponent` | `textureID` (ID définition) + `textureHandle` (runtime), `textureRect`, `size`, `tint`, `blendMode`, `zOrder`, `visible` | sortie sort par zOrder dans `RenderSystem` |
| `LightComponent` | `type` (Point/Directional/Spot), `color`, `radius`, `intensity`, `direction`, `angle`, `castShadows`, `enabled` | collecté par `LightingSystem` → `DynamicLightingEffect` |
| `AnimationComponent` | `animationID`, `frames` (vector<IntRect>), `frameDuration`, `currentFrame`, `loop`, `playing` | met à jour `SpriteComponent::textureRect` |
| `ColliderComponent` | `type` (Box/Circle), `size` ou `radius`, `offset`, `isTrigger`, `enabled` | `PhysicsSystem` fait AABB pair-à-pair (O(n²), simple) |
| `AudioComponent` | `soundID`, `soundHandle`, `playOnStart`, `loop`, `volume`, `pitch`, `playing` | `AudioSystem` joue via `AUDIO()` |
| `ActivatorComponent` | `type` (Proximity/Manual/Automatic), `shape` (Box/Circle), `size`/`radius`, `targetTag`, `actionID`, `cooldownTime`, `onActivateEvent`/`onDeactivateEvent`, `showDebugZone` | détection par tag |
| `TagComponent` | `tag` (string) | identification — ex `"player"` |
| `SceneTransitionComponent` | `targetScene`, `targetPosition`, `isTransitioning` | utilisé par `JourneySystem` pour transferts |
| `ShaderComponent` | `shader` (ShaderHandle), `enabled` | shader custom par entité dans `RenderSystem::update` |
| `JourneyComponent` | `scenePath`, `currentSceneIndex`, `currentDestination`, `localWaypointPath`, `currentLocalWaypointIndex`, `preferredPathTags`, `isOnJourney`, `finalDestinationScene`/`Pos` | voyages multi-scènes pour NPC |

### Components custom (jeu, pas dans le SDK)
- `DialogueComponent` (`client/src/Dialogue/DialogueComponent.hpp`) — `npcName` + `dialogueLines` + `currentLine`. Construit en code (pas via JSON `definitions/`).

### Pour ajouter un nouveau component custom
```cpp
class MyComponent : public NovaEngine::Component {
public:
    int data = 0;
    COMPONENT_TYPE_ID(MyComponent)   // ← obligatoire (identifiant pour registry.getEntitiesWith)
    void serialize(nlohmann::json& j) const override   { j["data"] = data; }
    void deserialize(const nlohmann::json& j) override { if (j.contains("data")) data = j["data"]; }
};
```
Puis l'attacher : `entity->addComponent(std::make_unique<MyComponent>())`.

### Systems built-in (`sdk/include/NovaEngine/ECS/Systems.hpp`)

| System | Composants requis | Rôle |
|---|---|---|
| `RenderSystem` | Transform + Sprite (+Shader optionnel) | tri zOrder + `GRAPHICS().drawSprite(SpriteData)` |
| `AnimationSystem` | Sprite + Animation | avance `currentFrame`, met à jour `Sprite::textureRect` |
| `PhysicsSystem` | Transform + Collider | AABB box-box O(n²), log only (pas de résolution) |
| `ActivatorSystem` | Transform + Activator (+ Tag pour triggers) | détection zone, cooldown, événements |
| `AudioSystem` | Audio | `playOnStart` → `AUDIO().playSound` |
| `LightSystem` | Transform + Light | rendu de cercle semi-transparent (visualisation simple, pas le vrai lighting) |
| `JourneySystem` | Transform + SceneTransition + Journey | pathfinding multi-scènes, suivi waypoints, prépare transferts |

⚠️ Le `JourneySystem` n'est **PAS** dans la liste par défaut de `Scene::Scene()` — il faut l'ajouter manuellement et lui passer un `SceneGraph*` (constructeur explicite).

Ordre d'init des systems d'une `Scene` (`Scene.hpp:41`) :
```
Animation → Physics → Activator → Audio → Light → Render
```

### Format JSON — Définitions (Tier 1)

`client/assets/data/definitions/Sprites.json` :
```json
{ "sprites": [
  { "id": "player", "texture": "data/sprites/player.png",
    "width": 114, "height": 225, "scale": 2.0, "zOrder": 10 },
  { "id": "floor_wood", "texture": "tileset_floors",
    "textureRect": [0,0,32,32], "origin": [0,0], "size": [32,32],
    "scale": [1.0,1.0], "zOrder": -10 }
]}
```
Mêmes structures pour `NPCs.json` (avec dialogues), `Lights.json`, `Animations.json`, `Audio.json`, `Activators.json`. Chargé une seule fois au démarrage par `DefinitionManager`.

### Format JSON — Scènes (Tier 2)

`client/assets/data/scenes/test.json` :
```json
{
  "name": "Test Scene",
  "type": "interior",
  "backgroundColor": [0, 0, 0, 255],
  "entities": [
    { "type": "sprite", "spriteID": "floor", "x": 1000, "y": 1100, "zOrder": 0 },
    { "type": "player", "spriteID": "player", "x": 1000, "y": 1080, "zOrder": 10 },
    { "components": {
        "transform": { "position": [1000, 800] },
        "light": { "type": "point", "color": [255,200,100,255],
                   "radius": 520.0, "intensity": 0.8, "enabled": true } } }
  ]
}
```

Deux formats coexistent dans `entities[]` :
- **Forme courte** (héritée) : `{ "type": "sprite|player|npc|...", "spriteID": "...", "x": ..., "y": ..., "zOrder": ... }` — résolue via `DefinitionManager`.
- **Forme étendue** : `{ "components": { "transform": {...}, "light": {...}, ... } }` — clés en lowercase mappées au composant correspondant.

### Format JSON — SceneGraph (inter-scènes)

`client/assets/data/scenegraph.json` :
```json
{ "connections": [
  { "from": "ville", "to": "taverne",
    "exitPortal": [800, 300], "entryPortal": [400, 580],
    "travelTime": 1.0, "bidirectional": true }
]}
```
Utilisé par `JourneySystem` pour planifier les voyages multi-scènes des NPC.

---

## 6. Backend — couche d'abstraction graphique/audio/IO

`BackendManager` est un singleton (`BACKEND()`) qui owne 7 sous-backends, accessibles via macros :

| Macro | Type | Quoi |
|---|---|---|
| `WINDOW()` | `IWindowBackend` | open/close/resize/setVSync/setFramerateLimit/clear/display |
| `INPUT()` | `IInputBackend` | `pollEvent(InputEvent&)`, état clavier/souris |
| `GRAPHICS()` | `IGraphicsBackend` | `loadTexture`, `drawSprite/Rect/Text`, shaders, RenderTextures |
| `RESOURCES()` | `IResourceBackend` | (gestion handles) |
| `AUDIO()` | `IAudioBackend` | `loadSound`, `playSound`, music |
| `FONTS()` | `IFontBackend` | `loadFont`, mesures texte |
| `VIEWPORT()` | `IViewportBackend` | `setView(ViewportData)`, `setViewCenter`, `resetView` |

Init :
```cpp
BACKEND().initialize(BackendType::SFML, width, height, "Title", fullscreen);
```
Une seule impl à ce jour (`SFML*Backend`). L'enum `BackendType` prévoit `SDL` et `Custom` mais ils ne sont pas implémentés.

### Types graphiques (`Backend/Core/BackendTypes.hpp`)

- **Math** : `Vec2f`, `Vec2i`, `Vec2u`, `Vec3f` (avec opérateurs), `Rect`, `IntRect`.
- **Color** : `r/g/b/a` u8 + statics `Black/White/Red/Green/Blue/Yellow/Transparent`.
- **Handles** : `TextureHandle`, `FontHandle`, `SoundHandle`, `MusicHandle`, `ShaderHandle`, `RenderTextureHandle` (tous u64). `INVALID_HANDLE = 0`.
- **Drawing** : `SpriteData`, `RectData`, `TextData`, `TextMetrics`, `ViewportData`.
- **Input** : `KeyCode` (A-Z, Num0-9, Escape, Modifiers, Arrow keys, Space, Enter), `MouseButton`, `InputEventType`, `InputEvent` (union par type).
- **Style** : `BlendMode` (Alpha/Add/Multiply/None), `TextStyle` (Regular/Bold/Italic/Underlined/StrikeThrough, flags combinables).

### Aliases de types (`Core/Types.hpp`)
`i8/16/32/64`, `u8/16/32/64`, `f32/64`, `String = std::string`, `ID = std::string`, `Ref<T>` = `shared_ptr`, `Unique<T>` = `unique_ptr`, `Weak<T>` = `weak_ptr`.

⚠️ Il y a une **double déclaration** de ces types : à la fois dans `Core/Types.hpp` ET dans `Backend/Core/BackendTypes.hpp`. C'est inoffensif tant que les déclarations sont identiques, mais à surveiller.

---

## 7. Rendering — pipeline post-processing

### Architecture
```
SceneRender → RenderTexture A (m_renderTexture)
                    ↓
              Effect 1 (SSAO)        si enabled  → A → B
                    ↓
              Effect 2 (Bloom)       si enabled  → B → A
                    ↓
              Effect 3 (ColorGrading)si enabled  → A → B
                    ↓
              Effect 4 (DynamicLighting) si enabled → B → A
                    ↓
              Final blit → écran
```

Ping-pong entre `m_renderTexture` et `m_tempTexture` (`PostProcessPipeline.cpp`). Si **aucun** effet activé → blit direct.

### API
```cpp
m_pipeline = std::make_unique<PostProcessPipeline>(&GRAPHICS());
m_pipeline->initialize(width, height);
auto* ssao  = m_pipeline->addEffect<SSAOEffect>();      // T* retourné, owne par pipeline
auto* bloom = m_pipeline->addEffect<BloomEffect>();
ssao->setEnabled(true);
ssao->setStrength(0.4f);

// Dans onRender :
m_pipeline->beginSceneRender();   // bind RT
sceneManager.render();
m_pipeline->endSceneRender(dt);   // applique chaîne, blit final
```

### Effets disponibles (`Rendering/Effects/`)

| Effet | Shader | Paramètres clés |
|---|---|---|
| `SSAOEffect` | `ssao.vert/frag` | `radius`, `bias`, `strength`, `samples` |
| `BloomEffect` | `bloom.frag` | `intensity`, `threshold`, `blurRadius` |
| `ColorGradingEffect` | `color_grading.frag` | `saturation`, `contrast`, `brightness`, `gamma`, color balance |
| `DynamicLightingEffect` | `dynamic_lighting.frag` | jusqu'à N lights (LightData), `ambientDarkness`, camera (world↔screen) |
| `CRTEffect` | `crt.vert/frag` | scanlines, distortion (TODO bind AO secondary, voir `CRTEffect.cpp:183`) |
| `PassthroughEffect` | `passthrough.vert/frag` | identité, debug |

### Créer un effet custom
Hériter de `PostProcessEffect`, implémenter :
```cpp
bool initialize(IGraphicsBackend* g, u32 w, u32 h) override;  // load shader, create uniforms
void apply(RenderTextureHandle in, RenderTextureHandle out, f32 dt) override;
void shutdown() override;
const char* getName() const override { return "MyEffect"; }
```
Ajouter via `pipeline.addEffect<MyEffect>()`.

### Lighting (Dynamic, via ECS)

`Systems/LightingSystem` parcourt les entités avec `LightComponent` à chaque frame, les pousse vers `DynamicLightingEffect`. La caméra est fournie par `m_dynamicLightingEffect->setCamera(viewportCenter, viewportSize)` (depuis `Game::onUpdate`) pour la conversion world→screen dans le shader.

Time-of-day : `LightingSystem::setTimeOfDay(0.0..1.0)` (0 = minuit, 0.5 = midi). Touche `T` ajoute 1h dans le jeu.

---

## 8. UI System

### Modèle
- `UIManager` owne tous les composants. Chaque composant porte 4 "axes" : `id` (unique), `uiID` (ex `"loginmenu"`), `groupID` (ex `"buttons"`), `layer` (i32, ordre de rendu).
- `setUIActive(uiID, bool)`, `setGroupActive(groupID, bool)`, `setLayerActive(layer, bool)` permettent d'activer/désactiver des sous-ensembles.
- `dispatchEvent(Event)` route vers tous les composants actifs.
- Cache de rendu interne (`m_renderCache`, invalidé sur changement) → tri par layer.

### Composants UI (`UI/Components/`)
`Button`, `Text`, `TextInput`, `Panel`, `Image`, `Slider`, `Animation`. Tous héritent de `UIComponent` (lui-même `EventHandler`).

Méthodes obligatoires : `render() const`, `onEvent(Event&)`, `getBounds()`. Le `update(dt)` est virtuel non-pure (override possible).

### Action callback
```cpp
m_uiManager.setActionCallback([this](const std::string& action,
                                     const std::string& value,
                                     const ID& componentID) {
    if (action == "connect") toggleConnectionState();
});
```
Les boutons configurent un `action` dans le JSON, transmis quand cliqués.

### Format JSON UI
`client/assets/ui/json/dialogue.json`, `loginmenu.json`. Sections : `buttons`, `images`, `text`, `userInput`. Chaque entrée porte `id`, `position`, `size`, `groupID`, `layer`, etc. — voir `UILoader.cpp:50` (`parseButtons`, `parseImages`, etc.).

Charger :
```cpp
m_uiLoader.loadFromFile("data/ui/json/dialogue.json", m_uiManager);
// ou
m_uiLoader.loadUI("dialogue", m_uiManager);  // résout en data/ui/json/dialogue.json
```

---

## 9. Events

`Event` (`Events/Event.hpp`) est une struct simple :
```cpp
EventType type;        // Unknown / Input / UI / Engine / Custom
InputEvent inputEvent; // pour Input
std::string name;      // pour UI/Custom
std::string payload;
```

Source : `Application::processEvents` poll `INPUT().pollEvent(InputEvent)`, wrap dans un `Event`, appelle `onEvent(event)`. Le ESC / Closed sont court-circuités vers `quit()` avant.

`EventDispatcher` / `EventHandler` (`Events/`) : pattern observer pour propagation interne (utilisé par UIComponent).

---

## 10. Configuration (`engine.ini`)

Localisation : `client/bin/Release/config/engine.ini` (relatif au cwd). Chargé par `ConfigManager::initializeGlobalConfig`.

Sections et clés :

```ini
[Display]
width=3840           ; résolution fenêtre
height=2160
fullscreen=false
vsync=true
frameRateLimit=60
antialiasingLevel=0
nativeWidth=3840     ; résolution logique (viewport letterbox)
nativeHeight=2160

[Audio]
masterVolume=100     ; 0-100
musicVolume=80
soundVolume=100
muteAll=false
audioDevice=default

[Input]
mouseSensitivity=1
invertMouse=false
moveUp=W,Up          ; bindings, séparateur virgule
moveDown=S,Down
moveLeft=A,Left
moveRight=D,Right
interact=E,Enter,Space
menu=Escape,M

[Debug]
enableLogging=false
logLevel=FATAL       ; TRACE/DEBUG/INFO/WARN/ERROR/FATAL
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

Accès en code via macros : `DISPLAY_CONFIG`, `AUDIO_CONFIG`, `INPUT_CONFIG`, `DEBUG_CONFIG`, `GAME_CONFIG`. Singleton via `ConfigManager::getInstance()`.
