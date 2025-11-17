# NovaEngine - Architecture du Jeu RPG

## Vue d'ensemble

Le jeu RPG intégré utilise une architecture modulaire basée sur le système ECS (Entity-Component-System) de NovaEngine. Le code est divisé en modules indépendants pour faciliter la maintenance et l'extension.

## Structure des Modules

```
client/
├── main.cpp                      # Point d'entrée du jeu
├── src/
│   ├── Game.cpp                  # Classe principale du jeu (orchestration)
│   ├── Dialogue/
│   │   ├── DialogueComponent.hpp # Composant personnalisé pour les dialogues NPC
│   │   ├── DialogueSystem.hpp    # Système de gestion des dialogues
│   │   └── DialogueSystem.cpp
│   └── Player/
│       ├── PlayerController.hpp  # Contrôleur du joueur (mouvement, détection)
│       └── PlayerController.cpp
└── assets/
    └── data/
        ├── sprites/              # Sprites du jeu (joueur, NPCs)
        ├── fonts/                # Polices de caractères
        ├── definitions/          # Définitions des entités (Sprites.json, NPCs.json, etc.)
        ├── scenes/               # Scènes du jeu (ville.json, taverne.json, etc.)
        └── scenegraph.json       # Graphe de connexions entre scènes
```

## Modules Principaux

### 1. **Game.cpp** - Orchestrateur Principal

**Responsabilités:**
- Initialisation du moteur et des systèmes
- Gestion de la boucle de jeu (update/render/events)
- Coordination entre les modules (PlayerController, DialogueSystem, SceneManager)
- Rendu de la scène et des UI

**Méthodes clés:**
- `onInitialize()` - Initialise tous les systèmes et crée le joueur
- `onUpdate(deltaTime)` - Met à jour la logique du jeu
- `onRender()` - Rendu de tous les éléments visuels
- `onEvent(event)` - Gestion des entrées utilisateur
- `renderScene()` - Rendu des entités de la scène
- `renderNPCIndicator()` - Affiche l'indicateur "[E]" près des NPCs

**Configuration:**
Utilise le `ConfigManager` pour charger les paramètres depuis `config.json`:
- Dimensions de la fenêtre
- Mode plein écran
- VSync
- FPS limit
- Couleur de fond

### 2. **DialogueSystem** - Gestion des Dialogues

**Responsabilités:**
- Gestion de l'état des dialogues (actif/inactif)
- Avancement des lignes de dialogue
- Rendu de la boîte de dialogue avec mise en forme

**Interface publique:**
```cpp
void initialize(FontHandle font);          // Init avec une police
void startDialogue(Entity* npc);           // Démarre un dialogue avec un NPC
bool advanceDialogue();                    // Passe à la ligne suivante
bool isActive() const;                     // Vérifie si un dialogue est actif
void render(Vec2f viewCenter, Vec2f viewSize); // Rendu de la boîte de dialogue
void reset();                              // Réinitialise l'état
```

**Fonctionnement:**
1. Détecte quand le joueur appuie sur "E" près d'un NPC
2. Récupère les lignes de dialogue du `DialogueComponent` de l'NPC
3. Affiche une boîte de dialogue avec le nom du NPC et la ligne actuelle
4. Permet d'avancer avec "E"
5. Termine automatiquement à la fin du dialogue

### 3. **PlayerController** - Contrôle du Joueur

**Responsabilités:**
- Gestion des déplacements du joueur (WASD/flèches)
- Détection des NPCs à proximité
- Suivi de la position du joueur

**Interface publique:**
```cpp
void setPlayerID(u64 id);                  // Enregistre l'ID du joueur
void updateMovement(Scene*, float dt, bool allowed); // Maj mouvement
void updateNPCDetection(Scene*);           // Détecte NPCs proches
Entity* getNearestNPC() const;             // Retourne NPC le plus proche
Vec2f getPlayerPosition(Scene*) const;     // Position du joueur
void setMoveSpeed(float speed);            // Ajuste la vitesse
void setDetectionRadius(float radius);     // Ajuste rayon de détection
```

**Paramètres:**
- Vitesse de déplacement: 200 unités/seconde
- Rayon de détection NPC: 80 pixels
- Normalisation des mouvements diagonaux

### 4. **DialogueComponent** - Composant Personnalisé

**Extension ECS:**
Ce composant personnalisé étend le système ECS pour supporter les dialogues.

```cpp
class DialogueComponent : public Component {
public:
    std::string npcName;                   // Nom du NPC
    std::vector<std::string> dialogueLines; // Lignes de dialogue
    int currentLine = 0;                   // Ligne actuelle

    // Serialization pour JSON
    void serialize(nlohmann::json&) const override;
    void deserialize(const nlohmann::json&) override;
};
```

## Flux de Données

### Initialisation
```
main()
  → Game::Game()
    → createConfig() (charge config.json)
    → new DialogueSystem()
    → new PlayerController()
  → Game::run()
    → onInitialize()
      → Load font
      → DialogueSystem::initialize()
      → SceneManager::initialize()
      → Load scenes
      → Create player entity
      → PlayerController::setPlayerID()
```

### Boucle de Jeu
```
Game::onUpdate(deltaTime)
  → SceneManager::update()
  → PlayerController::updateMovement() [si pas de dialogue]
  → PlayerController::updateNPCDetection()
  → Update camera (suit le joueur)
  → UIManager::update()

Game::onRender()
  → renderScene() [dessine tous les sprites]
  → renderNPCIndicator() [si NPC proche]
  → DialogueSystem::render() [si dialogue actif]
  → UIManager::render()

Game::onEvent(event)
  → ESC: quit()
  → E:
    → Si dialogue actif: DialogueSystem::advanceDialogue()
    → Sinon: DialogueSystem::startDialogue(nearestNPC)
```

## Assets et Configuration

### Structure des Assets
```
client/assets/data/
├── sprites/          # Images PNG pour entités
│   ├── player.png
│   ├── npc_merchant.png
│   └── npc_innkeeper.png
├── fonts/            # Polices TTF
│   └── arial.ttf
├── definitions/      # Définitions JSON
│   ├── Sprites.json
│   ├── NPCs.json
│   ├── Lights.json
│   ├── Animations.json
│   └── Audio.json
├── scenes/           # Scènes JSON
│   ├── ville.json
│   └── taverne.json
└── scenegraph.json   # Connexions entre scènes
```

### Format JSON - Scène

```json
{
  "name": "Village",
  "backgroundColor": [50, 100, 50, 255],
  "entities": [
    {
      "id": 100,
      "components": {
        "Transform": {"position": [400, 300]},
        "Sprite": {
          "definitionID": "npc_merchant",
          "tint": [50, 200, 100, 255]
        },
        "Dialogue": {
          "npcName": "Marcus the Merchant",
          "dialogueLines": [
            "Welcome traveler!",
            "What can I do for you?"
          ]
        }
      }
    }
  ]
}
```

## Extending the Game

### Ajouter un Nouveau Composant Personnalisé

1. Créer le header du composant dans `client/src/` :
```cpp
class MyComponent : public Component {
public:
    // Vos données ici

    COMPONENT_TYPE_ID(MyComponent)

    void serialize(nlohmann::json& json) const override;
    void deserialize(const nlohmann::json& json) override;
};
```

2. Inclure dans `Game.cpp` et l'utiliser

3. Mettre à jour `client__compile.bat` si nécessaire

### Ajouter un Nouveau Module

1. Créer le répertoire: `client/src/MonModule/`
2. Créer header (.hpp) et implémentation (.cpp)
3. Ajouter à `client__compile.bat`:
   ```batch
   set "SOURCE_FILES=%SOURCE_FILES% src\MonModule\MonModule.cpp"
   ```
4. Inclure et utiliser dans `Game.cpp`

### Ajouter une Nouvelle Scène

1. Créer le fichier JSON: `client/assets/data/scenes/ma_scene.json`
2. Ajouter les définitions d'entités si nécessaire
3. Charger dans `Game::onInitialize()`:
```cpp
m_sceneManager.loadScene("data/scenes/ma_scene.json", "ma_scene");
```
4. Optionnel: Ajouter connexion dans `scenegraph.json`

## Compilation

Pour compiler le jeu:
```batch
cd C:\Nova
client__compile.bat
```

Le script compile automatiquement:
- `Game.cpp`
- `DialogueSystem.cpp`
- `PlayerController.cpp`
- Tous les autres modules du moteur

L'exécutable est créé dans: `client/bin/Release/NovaGame.exe`

## Contrôles par Défaut

- **WASD** ou **Flèches** : Déplacer le joueur
- **E** : Parler aux NPCs / Avancer le dialogue
- **ESC** : Quitter le jeu

## Bonnes Pratiques

1. **Modularité** : Chaque système dans son propre module
2. **Séparation des responsabilités** : Game.cpp orchestre, les modules implémentent
3. **Configuration externe** : Utiliser config.json pour les paramètres
4. **Assets organisés** : Structure claire par type de ressource
5. **Composants réutilisables** : Étendre ECS plutôt que hard-coder
6. **Logging** : Utiliser LOG_INFO/LOG_ERROR pour le debug
7. **Gestion d'erreurs** : Vérifier les retours et handle invalides

## Performance

- Rendu optimisé: Un seul parcours des entités par frame
- Detection NPCs: Calculée une fois par frame, pas à chaque input
- Assets: Chargés une fois, réutilisés via handles
- Police: Chargée une fois, partagée entre systèmes
