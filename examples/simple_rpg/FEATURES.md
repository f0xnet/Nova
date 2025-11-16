# Fonctionnalités Démontrées - Simple RPG

Ce document explique en détail les fonctionnalités de NovaEngine démontrées dans cet exemple.

## 1. Architecture ECS (Entity Component System)

### Components Utilisés

#### TransformComponent
```cpp
Vec2f position;  // Position dans le monde
f32 rotation;    // Rotation (non utilisée dans cet exemple)
Vec2f scale;     // Échelle (non utilisée dans cet exemple)
```

**Utilisé pour** : Positionner le joueur et les NPCs dans le monde.

#### SpriteComponent
```cpp
TextureHandle textureHandle;  // Référence à la texture
Vec2f size;                   // Taille du sprite
Color color;                  // Teinte de couleur
```

**Utilisé pour** : Afficher visuellement le joueur et les NPCs.

#### DialogueComponent (Personnalisé)
```cpp
std::string npcName;                    // Nom du NPC
std::vector<std::string> dialogueLines; // Lignes de dialogue
int currentLine;                        // Ligne actuelle
```

**Utilisé pour** : Stocker les dialogues des NPCs.

#### TagComponent
```cpp
std::vector<std::string> tags;  // Tags d'identification
```

**Utilisé pour** : Identifier le joueur avec le tag "player".

### Systems Utilisés

- **RenderSystem** : Dessine automatiquement tous les sprites
- **AnimationSystem** : Gère les animations (inactif dans cet exemple)
- **PhysicsSystem** : Gère les collisions (inactif dans cet exemple)

## 2. Système de Scènes

### Chargement de Scènes

Les scènes sont chargées depuis des fichiers JSON :

```cpp
m_sceneMgr.loadScene("data/scenes/village.json", "village");
m_sceneMgr.loadScene("data/scenes/taverne.json", "taverne");
```

### Commutation de Scènes

```cpp
m_sceneMgr.setActiveScene("village");  // Active le village
```

La scène active est celle qui :
- Est mise à jour chaque frame
- Est rendue à l'écran
- Contient le joueur

### Structure des Scènes

Chaque scène contient :
- **Métadonnées** : Nom, type, couleur de fond
- **Entités** : Liste des entités avec leurs composants
- **Systems** : Logique de mise à jour et rendu

## 3. Architecture Deux Niveaux

### Niveau 1 : Definitions (Chargées une fois)

`assets/data/definitions/Sprites.json` :
```json
{
  "sprites": [
    {
      "id": "npc_merchant",
      "texture": "data/npc_merchant.png",
      "size": [64, 64]
    }
  ]
}
```

**Avantages** :
- Chargement unique des textures
- Réutilisation des définitions
- Modification centralisée

### Niveau 2 : Instances (Scènes)

`assets/data/scenes/village.json` :
```json
{
  "entities": [
    {
      "id": 100,
      "components": {
        "Sprite": {
          "definitionID": "npc_merchant"  // Référence !
        }
      }
    }
  ]
}
```

**Avantages** :
- Fichiers de scène légers
- Placement rapide d'entités
- Pas de duplication de données

## 4. Système de Dialogue

### Détection de Proximité

```cpp
Entity* findNearestNPC(Entity* player, Scene* scene, float maxDistance) {
    // Parcourt toutes les entités
    // Calcule la distance au joueur
    // Retourne la plus proche avec DialogueComponent
}
```

**Algorithme** :
1. Pour chaque entité de la scène
2. Si elle a un DialogueComponent
3. Calculer distance au joueur
4. Garder la plus proche dans le rayon maxDistance (80px)

### États du Dialogue

```cpp
bool m_dialogueActive;              // Dialogue en cours ?
int m_currentDialogueLine;          // Ligne actuelle
std::vector<std::string> m_currentDialogue;  // Lignes du dialogue
```

**Machine à états** :
- **Idle** : Pas de dialogue, joueur peut bouger
- **Dialogue actif** : Dialogue affiché, joueur bloqué
- **Transition** : Passage à la ligne suivante avec E

### Rendu du Dialogue

```cpp
void drawDialogueBox() {
    // 1. Calculer position (bas de l'écran en coordonnées monde)
    // 2. Dessiner fond noir semi-transparent
    // 3. Dessiner nom du NPC en jaune
    // 4. Dessiner texte du dialogue en blanc
    // 5. Dessiner indicateur "Press E"
}
```

**Coordonnées monde** : Le dialogue suit la caméra grâce à `viewCenter`.

## 5. Système de Caméra

### Caméra qui suit le joueur

```cpp
VIEWPORT().setViewCenter(playerTransform->position);
```

La caméra est centrée sur le joueur à chaque frame, créant un effet de scrolling naturel.

### Coordonnées Monde vs Écran

**Coordonnées Monde** :
- Position absolue dans le niveau
- Exemple : player.position = (640, 360)

**Coordonnées Écran** :
- Position relative à la fenêtre
- Exemple : coin supérieur gauche = (0, 0)

Le ViewportBackend gère automatiquement la conversion.

## 6. Gestion des Événements

### Polling des Événements

```cpp
void onEvent(const Event& event) override {
    if (event.type != Event::Type::Input) return;

    const InputEvent& inputEvent = event.inputEvent;

    if (inputEvent.type == InputEventType::KeyPressed &&
        inputEvent.key.code == KeyCode::E) {
        // Traiter la touche E
    }
}
```

**Hook virtuel** : La classe `Application` appelle `onEvent()` pour chaque événement.

### Contrôles Continus

```cpp
if (INPUT().isKeyPressed(KeyCode::W)) movement.y -= 1;
```

**État instantané** : Vérifie si la touche est enfoncée maintenant (pour mouvement continu).

**vs Événements** : `KeyPressed` se déclenche une fois à l'appui (pour actions ponctuelles).

## 7. Gestion des Ressources

### Chargement de Textures

```cpp
TextureHandle texture = RESOURCES().loadTexture("data/player.png");
```

**Cache automatique** : Si la même texture est chargée deux fois, le backend retourne le même handle.

### Chargement de Polices

```cpp
FontHandle font = RESOURCES().loadFont("data/arial.ttf");
```

**Utilisation** :
```cpp
TextData text;
text.font = font;
text.characterSize = 24;
GRAPHICS().drawText(text);
```

## 8. Backend Abstraction

### Macros d'Accès

```cpp
RESOURCES()   // ResourceBackend
GRAPHICS()    // GraphicsBackend
INPUT()       // InputBackend
VIEWPORT()    // ViewportBackend
```

**Avantage** : Le code est indépendant de SFML. On pourrait remplacer par SDL sans toucher au code du jeu.

### Backend SFML

Actuellement utilisé :
- **SFML Graphics** : Rendu 2D
- **SFML Window** : Fenêtrage
- **SFML Audio** : Sons (non utilisé dans cet exemple)

## 9. Application Framework

### Hooks Virtuels

```cpp
class SimpleRPG : public Application {
    bool onInitialize() override;       // Initialisation
    void onUpdate(float deltaTime) override;  // Mise à jour logique
    void onRender() override;           // Rendu supplémentaire
    void onEvent(const Event&) override;  // Traitement événements
    void onShutdown() override;         // Nettoyage
};
```

**Flux** :
1. Application::run() lance la boucle
2. Appelle onInitialize() une fois
3. Boucle : onUpdate() → onRender() → display()
4. Appelle onShutdown() à la fin

### Delta Time

```cpp
void onUpdate(float deltaTime) {
    player.position += velocity * speed * deltaTime;
}
```

**Indépendance du framerate** : Le mouvement est constant quelle que soit la vitesse de rendu.

## 10. Logging

### Niveaux de Log

```cpp
LOG_INFO("Game initialized successfully!");
LOG_WARN("Texture not found, using fallback");
LOG_ERROR("Failed to load scene");
```

**Sortie console** :
```
[2025-01-16 15:30:45] [INFO] [main.cpp] Game initialized successfully!
```

## Fonctionnalités NON Démontrées (Mais Disponibles)

Pour garder l'exemple simple, ces fonctionnalités ne sont pas utilisées :

- ❌ Animations (sprite sheets)
- ❌ Lumières 2D
- ❌ Effets audio
- ❌ Collisions physiques
- ❌ Système de particules
- ❌ Système UI complet
- ❌ Sauvegarde/chargement
- ❌ Navigation multi-scène des NPCs
- ❌ Waypoint pathfinding
- ❌ Système d'inventaire

Ces fonctionnalités sont disponibles dans NovaEngine, voir la documentation principale pour les utiliser.

## Extensions Suggérées

### Niveau 1 : Facile

1. **Ajouter plus de NPCs** : Dupliquer les NPCs existants avec de nouveaux dialogues
2. **Plus de scènes** : Créer une maison, une boutique
3. **Changer les couleurs** : Modifier backgroundColor pour différentes ambiances

### Niveau 2 : Intermédiaire

1. **Système d'inventaire simple** : Ajouter un InventoryComponent
2. **Transitions de scène** : Téléporter le joueur entre scènes via des portails
3. **Animations de marche** : Animer le sprite du joueur

### Niveau 3 : Avancé

1. **Combat au tour par tour** : Système de combat simple
2. **Système de quêtes** : QuestComponent avec objectifs
3. **Sauvegarde** : Sérialiser l'état du joueur en JSON
4. **IA des NPCs** : Patrouilles, routines journalières

## Analyse du Code

### Séparation des Responsabilités

- **main.cpp** : Logique du jeu
- **JSON** : Données de configuration
- **SDK** : Fonctionnalités du moteur

**Principe** : Le code du jeu ne connaît que l'API du moteur, pas l'implémentation.

### Pattern Composition

Au lieu d'héritage :
```cpp
// ❌ Mauvais
class NPC : public GameObject, public Talkable, public Renderable {}

// ✅ Bon (ECS)
Entity npc;
npc.addComponent<TransformComponent>();
npc.addComponent<SpriteComponent>();
npc.addComponent<DialogueComponent>();
```

### Data-Driven Design

Les dialogues sont dans JSON, pas en dur dans le code :
```cpp
// ❌ Mauvais
if (npcID == 100) {
    showDialogue("Hello traveler!");
}

// ✅ Bon
auto* dialogue = npc->getComponent<DialogueComponent>();
showDialogue(dialogue->dialogueLines);
```

## Performance

### Optimisations Appliquées

1. **Cache de proximité** : NPC le plus proche calculé une fois par frame
2. **Rendu conditionnel** : Indicateur "E" seulement si NPC proche
3. **Dialogue bloque mouvement** : Pas de calculs inutiles pendant dialogue

### Métriques Estimées

Pour cet exemple simple :
- **FPS** : 60 (VSync activé)
- **Entités** : ~3 (joueur + 2 NPCs)
- **Draw calls** : ~4 par frame
- **RAM** : ~50 MB

## Conclusion

Cet exemple démontre les **fondamentaux** de NovaEngine :
- ECS modulaire
- Chargement de scènes
- Système de dialogue
- Gestion ressources
- Backend abstraction

C'est une base solide pour créer des jeux 2D plus complexes !
