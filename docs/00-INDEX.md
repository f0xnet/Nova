# Documentation NovaEngine - Index

## Vue d'ensemble

NovaEngine est un moteur de jeu 2D en C++ conçu pour créer des jeux RPG avec des effets visuels avancés. Le moteur utilise une architecture **Entity Component System (ECS)** avec une **abstraction backend** permettant de supporter plusieurs bibliothèques graphiques (actuellement SFML).

## Caractéristiques principales

### Architecture
- **ECS pur** : Entités = conteneurs de composants, logique dans les systèmes
- **Backend modulaire** : Abstraction complète permettant de changer de bibliothèque graphique
- **Gestion multi-scènes** : Support de plusieurs scènes avec transitions et pathfinding
- **Système de définitions** : Séparation entre définitions d'entités et placement dans les scènes

### Rendu avancé
- **Pipeline de post-processing** : Effets appliqués séquentiellement avec ping-pong rendering
- **Éclairage dynamique** : Point lights, directional lights, spot lights avec cycle jour/nuit
- **Effets CRT** : Simulation d'écran cathodique avec scanlines, aberration chromatique, courbure
- **SSAO** : Screen Space Ambient Occlusion pour profondeur
- **Bloom** : Effet de glow sur zones lumineuses
- **Color Grading** : Correction colorimétrique

### Gameplay
- **Système de dialogue** : NPCs interactifs avec boîtes de dialogue
- **Pathfinding** : Navigation par waypoints pour NPCs avec personnalité (tags de préférence)
- **Voyages multi-scènes** : NPCs traversant physiquement plusieurs scènes
- **Zones d'activation** : Triggers (proximité, manuel, automatique)
- **Contrôleur joueur** : Mouvement, détection de NPCs

### Système UI
- **Composants UI** : Boutons, textes, champs de saisie, sliders, images, panels
- **Gestionnaire d'événements** : System d'actions avec callbacks
- **Chargement JSON** : Définition d'interfaces en JSON

## Structure de la documentation

### 📚 Modules principaux

1. **[Architecture générale](01-ARCHITECTURE.md)**
   - Vue d'ensemble de l'architecture
   - Patterns de conception utilisés
   - Flux de données et cycle de vie

2. **[Système Core](02-CORE.md)**
   - Types de base
   - Logger
   - Application (classe de base)
   - ConfigManager

3. **[Backend Abstraction](03-BACKEND.md)**
   - BackendManager
   - Interfaces backend
   - Implémentation SFML
   - Types backend (Vec2f, Color, etc.)

4. **[Entity Component System](04-ECS.md)**
   - Architecture ECS
   - Entity, Component, System
   - EntityRegistry
   - Tous les composants (11 built-in)

5. **[Systèmes ECS](05-SYSTEMS.md)**
   - RenderSystem
   - AnimationSystem
   - LightSystem
   - PhysicsSystem
   - ActivatorSystem
   - AudioSystem
   - JourneySystem

6. **[Gestion des scènes](06-SCENE-MANAGEMENT.md)**
   - Scene
   - SceneManager
   - DefinitionManager
   - SceneGraph & WaypointGraph
   - Système de pathfinding

7. **[Système de rendu](07-RENDERING.md)**
   - PostProcessPipeline
   - PostProcessEffect (base)
   - CRTEffect
   - BloomEffect
   - SSAOEffect
   - DynamicLightingEffect
   - ColorGradingEffect

8. **[Shaders](08-SHADERS.md)**
   - dynamic_lighting.frag
   - crt.frag
   - bloom.frag
   - ssao.frag
   - color_grading.frag

9. **[Système UI](09-UI.md)**
   - UIManager
   - UIComponent (base)
   - Composants UI (Button, Text, TextInput, etc.)
   - UILoader

10. **[Système Audio](10-AUDIO.md)**
    - AudioManager
    - SoundPlayer
    - MusicPlayer
    - AudioComponent

11. **[Système d'événements](11-EVENTS.md)**
    - Event
    - EventDispatcher
    - EventHandler

12. **[Gestion des ressources](12-RESOURCES.md)**
    - ResourceManager
    - ResourceTypes (handles)
    - Chargement depuis JSON

13. **[Classe Game](13-GAME.md)**
    - Structure de Game
    - Initialisation
    - Game loop
    - Intégration de tous les systèmes

14. **[Guides pratiques](14-GUIDES.md)**
    - Créer une nouvelle entité
    - Créer un nouveau composant
    - Créer un nouveau système
    - Ajouter un effet de post-processing
    - Créer une scène
    - Configurer le pathfinding

## Conventions de code

### Naming
- **Classes** : PascalCase (`TransformComponent`, `RenderSystem`)
- **Fonctions/méthodes** : camelCase (`getComponent()`, `update()`)
- **Variables membres** : m_camelCase (`m_position`, `m_entities`)
- **Variables locales** : camelCase (`deltaTime`, `entity`)
- **Constantes** : UPPER_SNAKE_CASE (`MAX_LIGHTS`, `INVALID_HANDLE`)

### Types
- `u8, u16, u32, u64` : unsigned integers
- `i8, i16, i32, i64` : signed integers
- `f32, f64` : floats
- `String` : std::string
- `ID` : std::string (identifiants)
- `Handle` types : u64 (TextureHandle, ShaderHandle, etc.)

### Organisation des fichiers
```
sdk/include/NovaEngine/          # Headers publics
├── Core/                         # Système core
├── Backend/                      # Abstraction backend
├── ECS/                          # Entity Component System
├── Rendering/                    # Post-processing
├── UI/                           # Interface utilisateur
├── Audio/                        # Système audio
├── Events/                       # Événements
└── Resources/                    # Gestion ressources

client/src/                       # Implémentation client
├── Game.cpp                      # Classe Game principale
├── Dialogue/                     # Système de dialogue
├── Player/                       # Contrôleur joueur
└── Systems/                      # Systèmes custom

client/assets/                    # Assets du jeu
├── data/                         # Données JSON
│   ├── definitions/              # Définitions d'entités
│   └── scenes/                   # Fichiers de scènes
├── shaders/                      # Shaders GLSL
└── ui/                           # Ressources UI
```

## Démarrage rapide

### Compilation

**Linux :**
```bash
cd /home/user/Nova
./compile_client.sh
```

**Windows :**
```bash
client__compile.bat
```

### Exécution
```bash
./client/bin/Release/NovaEngine
```

### Contrôles par défaut
- **WASD / Flèches** : Déplacement
- **E** : Interagir avec NPCs / Avancer dialogue
- **T** : Avancer le temps (1 heure)
- **1-4** : Toggle effets post-processing
- **ESC** : Quitter

## Dépendances

### Bibliothèques externes
- **SFML 2.x** : Graphics, Window, Audio, System
- **nlohmann/json** : Parsing JSON
- **OpenGL** : Shaders et rendu

### Compilateur
- **GCC 7+** ou **MinGW-w64** (Windows)
- Support C++17

## Statistiques du projet

- **66 fichiers header** (.hpp)
- **41 fichiers source** (.cpp)
- **11 shaders GLSL** (.frag, .vert)
- **11 composants built-in**
- **7 systèmes ECS**
- **5 effets de post-processing**
- **10 composants UI**

## Architecture en bref

```
┌─────────────────────────────────────────┐
│          APPLICATION (Game)              │
│  - Orchestration générale                │
│  - Intégration de tous les systèmes      │
└─────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│         SCENE MANAGER + ECS              │
│  - Gestion des scènes                    │
│  - Entity/Component/System               │
│  - Pathfinding multi-scène               │
└─────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│       POST-PROCESSING PIPELINE           │
│  - Chaîne d'effets                       │
│  - CRT, Bloom, SSAO, Lighting            │
└─────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│        BACKEND ABSTRACTION               │
│  - Graphics, Window, Input, Audio        │
│  - Implémentation SFML                   │
└─────────────────────────────────────────┘
```

## Prochaines étapes

1. **Lisez [Architecture](01-ARCHITECTURE.md)** pour comprendre la structure globale
2. **Consultez [ECS](04-ECS.md)** pour comprendre le cœur du moteur
3. **Explorez [Rendering](07-RENDERING.md)** pour les effets visuels
4. **Suivez [Guides pratiques](14-GUIDES.md)** pour créer votre contenu

## Ressources additionnelles

- **Code source** : `/home/user/Nova/`
- **Exemples** : `/home/user/Nova/examples/`
- **Assets de test** : `/home/user/Nova/client/assets/`

---

*Documentation générée automatiquement à partir de l'analyse complète du codebase NovaEngine*
*Dernière mise à jour : 2025-11-26*
