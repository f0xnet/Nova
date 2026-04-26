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
