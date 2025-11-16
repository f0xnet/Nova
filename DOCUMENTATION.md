# Documentation Complète - NovaEngine

**Version du moteur** : 1.0
**Dernière mise à jour** : Janvier 2025

---

## Table des Matières

### Partie I : Introduction et Concepts Fondamentaux
1. [À propos de NovaEngine](#1-à-propos-de-novaengine)
2. [Architecture Globale](#2-architecture-globale)
3. [Philosophie de Design](#3-philosophie-de-design)
4. [Types de Base et Structures](#4-types-de-base-et-structures)

### Partie II : Système Backend (Abstraction)
5. [Comprendre le Backend](#5-comprendre-le-backend)
6. [Backend Manager](#6-backend-manager)
7. [Window Backend](#7-window-backend)
8. [Graphics Backend](#8-graphics-backend)
9. [Input Backend](#9-input-backend)
10. [Resource Backend](#10-resource-backend)
11. [Audio Backend](#11-audio-backend)
12. [Font Backend](#12-font-backend)
13. [Viewport Backend](#13-viewport-backend)

### Partie III : Systèmes Core
14. [Logger - Système de Logging](#14-logger---système-de-logging)
15. [ConfigManager - Gestion de Configuration](#15-configmanager---gestion-de-configuration)
16. [NovaEngine - Classe Principale](#16-novaengine---classe-principale)
17. [Application - Créer un Jeu](#17-application---créer-un-jeu)

### Partie IV : Entity Component System (ECS)
18. [Introduction à l'ECS](#18-introduction-à-lecs)
19. [Composants (Components)](#19-composants-components)
20. [Entités (Entities)](#20-entités-entities)
21. [Systèmes (Systems)](#21-systèmes-systems)
22. [Entity Registry](#22-entity-registry)

### Partie V : Gestion de Scènes
23. [Scènes et SceneManager](#23-scènes-et-scenemanager)
24. [Definition Manager](#24-definition-manager)
25. [SceneGraph - Connexions](#25-scenegraph---connexions)
26. [WaypointGraph - Pathfinding](#26-waypointgraph---pathfinding)

### Partie VI : Systèmes Avancés
27. [Système de Journey Multi-Scènes](#27-système-de-journey-multi-scènes)
28. [Système UI](#28-système-ui)
29. [Système d'Événements](#29-système-dévénements)
30. [Resource Management](#30-resource-management)

### Partie VII : Guides Pratiques
31. [Créer Votre Premier Jeu](#31-créer-votre-premier-jeu)
32. [Formats JSON](#32-formats-json)
33. [Workflows de Développement](#33-workflows-de-développement)
34. [Debugging et Optimisation](#34-debugging-et-optimisation)

---

# Partie I : Introduction et Concepts Fondamentaux

## 1. À propos de NovaEngine

### 1.1 Qu'est-ce que NovaEngine ?

NovaEngine est un **moteur de jeu 2D moderne** écrit en C++17, conçu pour créer des jeux 2D riches et complexes avec une architecture propre et extensible. Le moteur utilise actuellement **SFML (Simple and Fast Multimedia Library)** comme backend graphique, mais son architecture permet de changer facilement de bibliothèque sans modifier le code du jeu.

### 1.2 Caractéristiques Principales

**Architecture ECS (Entity Component System)**
NovaEngine implémente une architecture ECS complète qui sépare clairement les données (Components) de la logique (Systems). Cette approche favorise la composition plutôt que l'héritage, rendant le code plus modulaire et maintenable.

**Abstraction Backend Complète**
Tous les appels à SFML passent par des interfaces abstraites. Cela signifie que :
- Vous pouvez changer de bibliothèque graphique (SDL, OpenGL, Vulkan) sans toucher au code du jeu
- Le code est plus testable (on peut mocker les backends)
- L'API est cohérente et simplifiée

**Système de Scènes Avancé**
Le moteur gère plusieurs scènes simultanément avec :
- Chargement/déchargement dynamique
- Transitions fluides entre scènes
- NPCs vivant en temps réel à travers plusieurs scènes
- Pathfinding intelligent avec waypoints personnalisables

**Data-Driven Design**
Presque tout est configurable via JSON :
- Définitions d'entités (sprites, lumières, animations, sons)
- Scènes et leur contenu
- Connexions entre scènes
- Configuration du moteur
- Interfaces utilisateur

**Performance Optimisée**
- Mise à jour sélective des scènes (seules les scènes actives sont mises à jour)
- Cache de ressources pour éviter les chargements multiples
- Queries ECS optimisées
- Tri automatique par z-order pour le rendu

### 1.3 Pour Qui Est Ce Moteur ?

NovaEngine est idéal pour :
- **Jeux 2D top-down** (RPG, aventure, stratégie)
- **Jeux avec monde ouvert** (plusieurs zones/scènes connectées)
- **Jeux avec NPCs complexes** (routines quotidiennes, déplacements réalistes)
- **Développeurs C++** qui veulent une base solide et extensible

### 1.4 Prérequis

**Connaissances requises :**
- C++17 (concepts de base : classes, templates, pointeurs intelligents)
- Notion de JSON
- Compréhension basique de la programmation orientée objet

**Logiciels nécessaires :**
- Compilateur C++17 (GCC 7+, Clang 5+, MSVC 2017+)
- SFML 2.5+ (pour le backend actuel)
- nlohmann/json (inclus dans le moteur)
- CMake ou système de build compatible

---

## 2. Architecture Globale

### 2.1 Vue d'Ensemble en Couches

NovaEngine est organisé en **couches d'abstraction successives**, chaque couche s'appuyant sur la précédente :

```
┌─────────────────────────────────────────────────────────┐
│  COUCHE 5 : VOTRE JEU                                   │
│  (Game class, logique spécifique du jeu)                │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│  COUCHE 4 : ECS ET SCÈNES                               │
│  (Entity, Component, System, Scene, SceneManager)       │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│  COUCHE 3 : SYSTÈMES DE HAUT NIVEAU                     │
│  (UI, Events, Resources, Audio)                         │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│  COUCHE 2 : CORE ET APPLICATION                         │
│  (Logger, ConfigManager, NovaEngine, Application)       │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│  COUCHE 1 : BACKEND (Abstraction SFML)                  │
│  (Window, Graphics, Input, Audio, Resource, etc.)       │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│  COUCHE 0 : SFML (Bibliothèque externe)                 │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Flux d'Exécution Principal

Voici ce qui se passe lorsque vous lancez un jeu NovaEngine :

**1. Initialisation**
```
main() appelle NovaEngine::initialize()
    ↓
BackendManager crée tous les backends (Window, Graphics, Input, etc.)
    ↓
ConfigManager charge config.json
    ↓
Logger configure la sortie console/fichier
    ↓
Application::onInitialize() est appelé (votre code)
    ↓
Le jeu charge ses ressources, scènes, etc.
```

**2. Boucle Principale** (se répète à chaque frame)
```
INPUT().pollEvent() récupère les événements système
    ↓
Application::onEvent() traite les événements
    ↓
Application::onUpdate(deltaTime) met à jour la logique
    ├─ SceneManager::update() met à jour les scènes actives
    │   ├─ Chaque System::update() traite les entités
    │   └─ NPCs se déplacent, animations jouent, etc.
    └─ UIManager::update() met à jour l'interface
    ↓
WINDOW().clear() efface l'écran
    ↓
Application::onRender() dessine tout
    ├─ SceneManager::render() dessine les scènes
    │   └─ RenderSystem dessine les sprites triés par z-order
    └─ UIManager::render() dessine l'interface
    ↓
WINDOW().display() affiche le frame à l'écran
```

**3. Arrêt**
```
Application::onShutdown() nettoie vos ressources
    ↓
BackendManager::shutdown() libère les backends
    ↓
return 0; sortie du programme
```

### 2.3 Organisation des Fichiers

```
NovaEngine/
│
├── sdk/
│   ├── include/NovaEngine/      # Headers publics
│   │   ├── Backend/             # Interfaces backend
│   │   ├── Core/                # Logger, Config, Application
│   │   ├── ECS/                 # Entity Component System
│   │   ├── Events/              # Système d'événements
│   │   ├── Resources/           # Gestion ressources
│   │   ├── UI/                  # Système UI
│   │   └── Audio/               # Système audio
│   │
│   └── libs/                    # Bibliothèques externes (SFML, etc.)
│
├── client/                      # Votre jeu
│   ├── src/                     # Code source
│   │   └── game.cpp            # Votre classe Game
│   │
│   └── assets/                  # Assets du jeu
│       ├── textures/
│       ├── sounds/
│       ├── music/
│       ├── fonts/
│       ├── data/
│       │   ├── definitions/     # JSONs de définitions
│       │   ├── scenes/          # JSONs de scènes
│       │   └── scenegraph.json  # Connexions entre scènes
│       └── ui/                  # JSONs d'interfaces
│
├── config.json                  # Configuration du moteur
└── game.log                     # Fichier de log
```

---

## 3. Philosophie de Design

### 3.1 Composition plutôt qu'Héritage

NovaEngine favorise la **composition** via l'ECS. Au lieu de créer des classes dérivées complexes (`class Enemy : public Character : public Sprite`), on compose des entités avec des composants :

```cpp
// ❌ Mauvaise approche (héritage)
class Character { ... };
class Enemy : public Character { ... };
class FlyingEnemy : public Enemy { ... };

// ✅ Bonne approche (composition)
Entity* enemy = registry.createEntity();
enemy->addComponent<TransformComponent>();
enemy->addComponent<SpriteComponent>();
enemy->addComponent<AIComponent>();
enemy->addComponent<HealthComponent>();
```

**Avantages :**
- Flexibilité totale (on peut ajouter/retirer des composants à la volée)
- Pas de hiérarchie complexe à gérer
- Réutilisation maximale du code
- Facile à sérialiser/désérialiser

### 3.2 Data-Driven Development

Le moteur sépare clairement **code** (comment) et **données** (quoi). Presque tout est configurable via JSON, ce qui permet :

**Itération Rapide**
Modifiez un fichier JSON, relancez le jeu → changements instantanés. Pas besoin de recompiler.

**Accessibilité**
Les designers, artistes et game designers peuvent ajuster le jeu sans toucher au code C++.

**Modding**
Les joueurs peuvent modifier le jeu en éditant les JSONs.

**Exemple concret :**
```json
// Définir un nouvel ennemi dans NPCs.json
{
  "id": "goblin",
  "sprite": "goblin_sprite",
  "health": 50,
  "speed": 80.0,
  "attackDamage": 10
}
```

Pas de code à écrire, juste un JSON à éditer !

### 3.3 Séparation des Préoccupations

Chaque système a **une seule responsabilité** :

- **RenderSystem** : Afficher les sprites (et rien d'autre)
- **PhysicsSystem** : Gérer les collisions (et rien d'autre)
- **AnimationSystem** : Mettre à jour les animations (et rien d'autre)

Cela rend le code :
- Plus facile à comprendre
- Plus facile à tester
- Plus facile à débugger
- Plus facile à étendre

### 3.4 Abstraction et Portabilité

Le backend est **complètement abstrait**. Votre code ne parle jamais directement à SFML, mais utilise des interfaces :

```cpp
// Votre code utilise l'interface abstraite
GRAPHICS().drawSprite(spriteData);

// En interne, ça peut être SFML, SDL, OpenGL, Vulkan...
// Vous ne le savez pas et vous n'avez pas à le savoir !
```

**Bénéfices :**
- Changer de bibliothèque graphique = modifier une seule classe
- Tester le jeu sans fenêtre (backend mock)
- Supporter plusieurs plateformes facilement

### 3.5 Performance Consciente

Le moteur est conçu pour être **performant par défaut** :

**Optimisations Automatiques :**
- Cache de ressources (une texture chargée une seule fois)
- Update sélectif (seules les scènes avec NPCs actifs sont mises à jour)
- Tri par z-order (rendu optimisé)
- Queries ECS optimisées (hash maps pour lookup rapide)

**Contrôle Fin :**
- Vous pouvez désactiver des systèmes inutiles
- Activer/désactiver des scènes dynamiquement
- Définir des priorities de mise à jour

---

## 4. Types de Base et Structures

### 4.1 Types Numériques

NovaEngine utilise des **types explicites** pour éviter les ambiguïtés :

```cpp
// Types entiers non signés (toujours positifs)
u8  myByte;       // 0 à 255
u16 myShort;      // 0 à 65,535
u32 myInt;        // 0 à 4,294,967,295
u64 myLong;       // 0 à 18,446,744,073,709,551,615

// Types entiers signés (peuvent être négatifs)
i8  mySignedByte;   // -128 à 127
i16 mySignedShort;  // -32,768 à 32,767
i32 mySignedInt;    // -2,147,483,648 à 2,147,483,647
i64 mySignedLong;   // Très grand

// Types flottants
f32 myFloat;      // Précision simple (float)
f64 myDouble;     // Précision double
```

**Pourquoi utiliser ces types ?**

1. **Clarté** : `u32 playerHealth;` est plus explicite que `unsigned int playerHealth;`
2. **Portabilité** : Taille garantie sur toutes les plateformes
3. **Prévention d'erreurs** : Impossible de stocker -1 dans un `u32`

### 4.2 Vecteurs 2D

Les vecteurs sont utilisés partout pour représenter positions, directions, tailles, etc.

```cpp
template<typename T>
struct Vec2 {
    T x, y;

    Vec2() : x(0), y(0) {}
    Vec2(T x_, T y_) : x(x_), y(y_) {}

    // Opérateurs arithmétiques
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(T scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(T scalar) const {
        return Vec2(x / scalar, y / scalar);
    }

    // Fonctions utilitaires
    T length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalized() const {
        T len = length();
        return (len > 0) ? (*this / len) : Vec2(0, 0);
    }

    T dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }
};

// Types prédéfinis
using Vec2f = Vec2<f32>;   // Vecteur de floats
using Vec2i = Vec2<i32>;   // Vecteur d'entiers signés
using Vec2u = Vec2<u32>;   // Vecteur d'entiers non signés
```

**Utilisation typique :**

```cpp
// Position d'un joueur
Vec2f playerPosition(100.0f, 200.0f);

// Déplacement
Vec2f velocity(50.0f, 0.0f);
playerPosition = playerPosition + velocity * deltaTime;

// Direction normalisée
Vec2f toTarget = target - playerPosition;
Vec2f direction = toTarget.normalized();

// Taille d'un rectangle
Vec2f size(128.0f, 64.0f);
```

### 4.3 Rectangles

Représentent des zones rectangulaires (collisions, textures, UI).

```cpp
template<typename T>
struct Rect {
    T left, top, width, height;

    Rect() : left(0), top(0), width(0), height(0) {}
    Rect(T l, T t, T w, T h) : left(l), top(t), width(w), height(h) {}

    // Tests de collision
    bool contains(T x, T y) const {
        return (x >= left) && (x < left + width) &&
               (y >= top) && (y < top + height);
    }

    bool contains(const Vec2<T>& point) const {
        return contains(point.x, point.y);
    }

    bool intersects(const Rect& other) const {
        return !(left + width < other.left ||
                 other.left + other.width < left ||
                 top + height < other.top ||
                 other.top + other.height < top);
    }
};

using IntRect = Rect<i32>;     // Rectangle d'entiers
using FloatRect = Rect<f32>;   // Rectangle de floats
```

**Utilisation :**

```cpp
// Zone de collision d'un personnage
IntRect collisionBox(100, 200, 32, 64);

// Vérifier si la souris est dans un bouton
Vec2i mousePos = INPUT().getMousePosition();
if (buttonRect.contains(mousePos)) {
    // Clic sur le bouton !
}

// Rectangle de texture (sprite sheet)
IntRect spriteRect(0, 0, 32, 32);  // Premier sprite 32x32
```

### 4.4 Couleurs

```cpp
struct Color {
    u8 r, g, b, a;  // Rouge, Vert, Bleu, Alpha (0-255)

    Color() : r(0), g(0), b(0), a(255) {}
    Color(u8 r_, u8 g_, u8 b_, u8 a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}

    // Couleurs prédéfinies
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;
};

// Définitions
const Color Color::Black(0, 0, 0);
const Color Color::White(255, 255, 255);
const Color Color::Red(255, 0, 0);
// etc...
```

**Utilisation :**

```cpp
// Couleur unie
Color backgroundColor = Color::Black;

// Couleur personnalisée
Color customColor(128, 64, 200);  // Violet

// Couleur semi-transparente
Color semiTransparent(255, 0, 0, 128);  // Rouge à 50%

// Teinte d'un sprite
sprite.tint = Color(255, 128, 128);  // Teinte rougeâtre
```

### 4.5 Handles de Ressources

Les ressources (textures, sons, musique, fonts) sont référencées par des **handles** (identifiants uniques de type `u64`) :

```cpp
using TextureHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
using FontHandle = u64;
using ShaderHandle = u64;

constexpr u64 INVALID_HANDLE = 0;
```

**Pourquoi des handles plutôt que des pointeurs ?**

1. **Sécurité** : Un handle invalide ne cause pas de crash
2. **Sérialisation** : Facile à sauvegarder dans un fichier
3. **Cache** : Le backend peut gérer le cache en interne
4. **Abstraction** : Vous ne gérez pas la mémoire directement

**Utilisation :**

```cpp
// Charger une texture
TextureHandle playerTexture = RESOURCES().loadTexture("assets/player.png");

// Vérifier la validité
if (playerTexture != INVALID_HANDLE) {
    // Utiliser la texture
    spriteData.textureHandle = playerTexture;
}

// Le backend SFML gère la vraie sf::Texture en interne
```

### 4.6 Transform

Représente une transformation 2D complète (position, rotation, échelle, origine).

```cpp
struct Transform {
    Vec2f translation = {0, 0};   // Position
    f32 rotation = 0.0f;          // Rotation en degrés
    Vec2f scale = {1, 1};         // Échelle (1 = taille normale)
    Vec2f origin = {0, 0};        // Point d'origine pour rotation

    // Combiner des transformations
    Transform combine(const Transform& other) const;

    // Transformer un point
    Vec2f transformPoint(const Vec2f& point) const;
};
```

**Exemple d'utilisation :**

```cpp
Transform playerTransform;
playerTransform.translation = Vec2f(100, 200);  // Position
playerTransform.rotation = 45.0f;               // Rotation de 45°
playerTransform.scale = Vec2f(2.0f, 2.0f);     // 2x plus grand
playerTransform.origin = Vec2f(16, 16);        // Centre de rotation

// Appliquer lors du rendu
GRAPHICS().drawSprite(spriteData);  // Le backend applique la transform
```

---

# Partie II : Système Backend (Abstraction)

## 5. Comprendre le Backend

### 5.1 Qu'est-ce que le Backend ?

Le **backend** est la couche d'abstraction qui isole votre code de jeu de la bibliothèque graphique sous-jacente (actuellement SFML). C'est une série d'**interfaces C++** (classes abstraites) qui définissent ce qu'un backend doit pouvoir faire, sans spécifier comment.

### 5.2 Pourquoi Abstraire SFML ?

**Problème sans abstraction :**

Imaginons que vous écrivez directement contre SFML dans tout votre code :

```cpp
// Partout dans votre code
sf::RenderWindow window;
sf::Sprite sprite;
sf::Texture texture;
// Des centaines de fichiers utilisent directement SFML
```

Si vous voulez changer pour SDL ou OpenGL → **impossible** sans réécrire tout !

**Solution avec abstraction :**

```cpp
// Votre code utilise des interfaces
IWindowBackend& window = WINDOW();
IGraphicsBackend& graphics = GRAPHICS();

// L'implémentation peut être SFML, SDL, OpenGL...
// Votre code reste inchangé !
```

### 5.3 Architecture Backend

```
Votre Code
     ↓
Interfaces (IWindowBackend, IGraphicsBackend, etc.)
     ↓
Implémentation SFML (SFMLWindowBackend, SFMLGraphicsBackend, etc.)
     ↓
Bibliothèque SFML
```

**Avantages :**

1. **Portabilité** : Changer de bibliothèque = changer une implémentation
2. **Testabilité** : Créer des mocks pour tester sans fenêtre
3. **Clarté** : API simplifiée et cohérente
4. **Maintenance** : Bugs SFML isolés dans une seule couche

### 5.4 Les 7 Backends

NovaEngine décompose SFML en **7 backends spécialisés** :

1. **WindowBackend** : Gestion de la fenêtre (création, événements, affichage)
2. **GraphicsBackend** : Rendu (dessiner sprites, formes, texte)
3. **InputBackend** : Entrées (clavier, souris)
4. **ResourceBackend** : Chargement de ressources (textures, sons, fonts)
5. **AudioBackend** : Audio (jouer sons et musique)
6. **FontBackend** : Polices de caractères (charger, mesurer texte)
7. **ViewportBackend** : Caméra/vue (zoom, déplacement, conversions écran↔monde)

Chacun a **une seule responsabilité**, ce qui facilite la maintenance.

---

## 6. Backend Manager

### 6.1 Rôle du BackendManager

Le **BackendManager** est le **singleton central** qui :
- Crée et initialise tous les backends
- Fournit l'accès global à chaque backend
- Gère le cycle de vie (init/shutdown)

### 6.2 Utilisation

```cpp
// Initialiser tous les backends
BackendManager::get().initialize(
    BackendType::SFML,
    1920,                    // Largeur fenêtre
    1080,                    // Hauteur fenêtre
    "Mon Jeu NovaEngine",    // Titre
    false                    // Fullscreen
);

// Accéder aux backends
IWindowBackend& window = BackendManager::get().window();
IGraphicsBackend& graphics = BackendManager::get().graphics();

// Ou via les macros pratiques
WINDOW().display();
GRAPHICS().drawSprite(spriteData);
INPUT().isKeyPressed(KeyCode::W);
```

### 6.3 Macros d'Accès Global

Pour simplifier le code, NovaEngine fournit des macros :

```cpp
#define BACKEND()       NovaEngine::BackendManager::get()
#define WINDOW()        NovaEngine::BackendManager::get().window()
#define INPUT()         NovaEngine::BackendManager::get().input()
#define GRAPHICS()      NovaEngine::BackendManager::get().graphics()
#define RESOURCES()     NovaEngine::BackendManager::get().resources()
#define AUDIO()         NovaEngine::BackendManager::get().audio()
#define FONTS()         NovaEngine::BackendManager::get().fonts()
#define VIEWPORT()      NovaEngine::BackendManager::get().viewport()
```

**Exemple d'utilisation :**

```cpp
// Au lieu de :
BackendManager::get().window().clear(Color::Black);
BackendManager::get().window().display();

// On écrit simplement :
WINDOW().clear(Color::Black);
WINDOW().display();
```

---

## 7. Window Backend

### 7.1 Responsabilités

Le **WindowBackend** gère la fenêtre principale de votre jeu :
- Créer/fermer la fenêtre
- Gérer les événements système (fermeture, redimensionnement)
- Rafraîchir l'affichage (clear/display)
- Paramètres (VSync, frame rate limit, fullscreen)

### 7.2 Interface Complète

```cpp
class IWindowBackend {
public:
    // Création/Destruction
    virtual bool create(u32 width, u32 height, const String& title,
                       bool fullscreen = false) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // Rendu
    virtual void display() = 0;              // Afficher le frame
    virtual void clear(const Color& color) = 0;  // Effacer l'écran

    // Configuration
    virtual void setTitle(const String& title) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual void setFramerateLimit(u32 fps) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;

    // Propriétés
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;
    virtual bool hasFocus() const = 0;
    virtual void requestFocus() = 0;

    // Avancé
    virtual void* getNativeHandle() = 0;     // Handle natif (HWND sur Windows)
    virtual void setIcon(u32 width, u32 height, const u8* pixels) = 0;
    virtual void setMouseCursorVisible(bool visible) = 0;
    virtual void setMouseCursorGrabbed(bool grabbed) = 0;
};
```

### 7.3 Cycle d'Utilisation Typique

```cpp
// 1. Créer la fenêtre
WINDOW().create(1920, 1080, "Mon Jeu", false);
WINDOW().setVSync(true);
WINDOW().setFramerateLimit(60);

// 2. Boucle principale
while (WINDOW().isOpen()) {
    // Gérer les événements (voir Input Backend)

    // ... logique du jeu ...

    // 3. Rendu
    WINDOW().clear(Color::Black);

    // Dessiner tout ici

    WINDOW().display();  // Afficher
}

// 4. Fermeture automatique
```

### 7.4 Exemples Pratiques

**Basculer en fullscreen :**

```cpp
bool isFullscreen = false;

if (INPUT().isKeyJustPressed(KeyCode::F11)) {
    isFullscreen = !isFullscreen;
    WINDOW().setFullscreen(isFullscreen);
}
```

**Cacher le curseur de la souris :**

```cpp
// Pour un jeu FPS ou avec curseur custom
WINDOW().setMouseCursorVisible(false);
```

**Définir une icône personnalisée :**

```cpp
// Charger l'image de l'icône (32x32 pixels typiquement)
TextureHandle iconTexture = RESOURCES().loadTexture("assets/icon.png");
Vec2u iconSize = RESOURCES().getTextureSize(iconTexture);

// Extraire les pixels (vous devez implémenter getPixelData)
const u8* pixels = getPixelData(iconTexture);

WINDOW().setIcon(iconSize.x, iconSize.y, pixels);
```

---

## 8. Graphics Backend

### 8.1 Responsabilités

Le **GraphicsBackend** gère tout ce qui concerne le **rendu visuel** :
- Charger et gérer les textures
- Dessiner des sprites
- Dessiner des formes (rectangles, cercles, lignes)
- Dessiner du texte
- Gérer les shaders (optionnel)

### 8.2 Structures de Données

Avant de voir les méthodes, comprenons les structures utilisées :

**SpriteData** - Données complètes pour dessiner un sprite :

```cpp
struct SpriteData {
    TextureHandle textureHandle;        // Texture à utiliser
    Vec2f position = {0, 0};           // Position à l'écran
    Vec2f size = {0, 0};               // Taille (0 = taille native)
    f32 rotation = 0.0f;               // Rotation en degrés
    Vec2f scale = {1, 1};              // Échelle
    Vec2f origin = {0, 0};             // Point d'origine
    IntRect textureRect = {0,0,0,0};   // Rectangle dans la texture (0 = toute)
    Color color = Color::White;        // Teinte
    BlendMode blendMode = BlendMode::Alpha;
};
```

**RectData** - Données pour dessiner un rectangle :

```cpp
struct RectData {
    Vec2f position = {0, 0};
    Vec2f size = {100, 100};
    Color fillColor = Color::White;
    Color outlineColor = Color::Transparent;
    f32 outlineThickness = 0.0f;
    f32 rotation = 0.0f;
    Vec2f origin = {0, 0};
};
```

**TextData** - Données pour dessiner du texte :

```cpp
struct TextData {
    String text;
    FontHandle font;
    u32 characterSize = 30;
    Color fillColor = Color::White;
    Color outlineColor = Color::Transparent;
    f32 outlineThickness = 0.0f;
    TextStyle style = TextStyle::Regular;
    Vec2f position = {0, 0};
    f32 rotation = 0.0f;
    Vec2f scale = {1, 1};
    Vec2f origin = {0, 0};
};
```

### 8.3 Interface Complète

```cpp
class IGraphicsBackend {
public:
    virtual bool initialize(void* windowHandle) = 0;
    virtual void shutdown() = 0;

    // Gestion des textures
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual TextureHandle createTexture(u32 width, u32 height) = 0;
    virtual void updateTexture(TextureHandle handle, const u8* pixels,
                              u32 width, u32 height, u32 x = 0, u32 y = 0) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;
    virtual void setTextureSmooth(TextureHandle handle, bool smooth) = 0;
    virtual void setTextureRepeated(TextureHandle handle, bool repeated) = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;

    // Rendu
    virtual void drawSprite(const SpriteData& sprite) = 0;
    virtual void drawRect(const RectData& rect) = 0;
    virtual void drawText(const TextData& text) = 0;

    // Shaders (optionnel)
    virtual ShaderHandle loadShader(const String& vertexPath,
                                   const String& fragmentPath) = 0;
    virtual void bindShader(ShaderHandle handle) = 0;
    virtual void unloadShader(ShaderHandle handle) = 0;
};
```

### 8.4 Exemples Pratiques

**Dessiner un sprite simple :**

```cpp
// Charger la texture
TextureHandle playerTex = GRAPHICS().loadTexture("assets/player.png");

// Créer les données du sprite
SpriteData sprite;
sprite.textureHandle = playerTex;
sprite.position = Vec2f(100, 200);
sprite.size = Vec2f(32, 32);  // Ou (0,0) pour taille native

// Dessiner
GRAPHICS().drawSprite(sprite);
```

**Dessiner avec rotation et échelle :**

```cpp
SpriteData sprite;
sprite.textureHandle = texture;
sprite.position = Vec2f(400, 300);
sprite.rotation = 45.0f;           // Rotation de 45°
sprite.scale = Vec2f(2.0f, 2.0f);  // 2x plus grand
sprite.origin = Vec2f(16, 16);     // Centre de rotation

GRAPHICS().drawSprite(sprite);
```

**Extraire un sprite d'une spritesheet :**

```cpp
// Spritesheet 256x256 avec sprites 32x32
TextureHandle sheet = GRAPHICS().loadTexture("assets/spritesheet.png");

// Premier sprite (coin haut-gauche)
SpriteData sprite1;
sprite1.textureHandle = sheet;
sprite1.position = Vec2f(100, 100);
sprite1.textureRect = IntRect(0, 0, 32, 32);

// Deuxième sprite (à droite du premier)
SpriteData sprite2;
sprite2.textureHandle = sheet;
sprite2.position = Vec2f(200, 100);
sprite2.textureRect = IntRect(32, 0, 32, 32);

GRAPHICS().drawSprite(sprite1);
GRAPHICS().drawSprite(sprite2);
```

**Dessiner un rectangle coloré :**

```cpp
RectData rect;
rect.position = Vec2f(100, 100);
rect.size = Vec2f(200, 150);
rect.fillColor = Color::Red;
rect.outlineColor = Color::Black;
rect.outlineThickness = 2.0f;

GRAPHICS().drawRect(rect);
```

**Dessiner du texte :**

```cpp
// Charger la police
FontHandle font = FONTS().loadFont("assets/arial.ttf");

// Créer les données texte
TextData text;
text.text = "Score: 1000";
text.font = font;
text.characterSize = 24;
text.fillColor = Color::White;
text.outlineColor = Color::Black;
text.outlineThickness = 1.0f;
text.style = TextStyle::Bold;
text.position = Vec2f(10, 10);

// Dessiner
GRAPHICS().drawText(text);
```

**Teinte d'un sprite (effet de couleur) :**

```cpp
SpriteData sprite;
sprite.textureHandle = texture;
sprite.position = Vec2f(100, 100);
sprite.color = Color(255, 128, 128);  // Teinte rougeâtre

GRAPHICS().drawSprite(sprite);
```

**Sprite semi-transparent :**

```cpp
sprite.color = Color(255, 255, 255, 128);  // Alpha à 50%
GRAPHICS().drawSprite(sprite);
```

---

## 9. InputBackend - Gestion des Entrées

Le `InputBackend` gère toutes les entrées utilisateur : clavier, souris, et événements fenêtre. Il fournit deux modes d'interrogation : **polling** (vérification active) et **waiting** (attente bloquante). L'architecture distingue entre la **vérification d'état instantané** (est-ce que la touche X est pressée maintenant ?) et les **événements discrets** (l'utilisateur a-t-il appuyé sur X depuis le dernier frame ?).

**Interface complète :**

```cpp
class IInputBackend {
public:
    // Récupération d'événements
    virtual bool pollEvent(InputEvent& event) = 0;        // Non-bloquant
    virtual bool waitEvent(InputEvent& event) = 0;        // Bloquant

    // État instantané
    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual Vec2i getMousePosition() const = 0;
    virtual void setMousePosition(const Vec2i& position) = 0;

    // Configuration
    virtual void setKeyRepeatEnabled(bool enabled) = 0;
};
```

**Explications détaillées :**

- **`pollEvent()`** : Récupère le prochain événement dans la file d'attente. Retourne `false` si la file est vide. C'est la méthode principale pour la boucle de jeu car elle ne bloque jamais.

- **`waitEvent()`** : Bloque le thread jusqu'à ce qu'un événement arrive. Utile pour les menus ou interfaces statiques qui ne doivent se réveiller que sur interaction.

- **`isKeyPressed()` / `isMouseButtonPressed()`** : Interroge l'état **actuel** d'une touche/bouton. Différent des événements KeyPressed qui sont déclenchés une seule fois lors de l'appui. Utilisez l'état instantané pour les contrôles continus (déplacement de personnage), et les événements pour les actions ponctuelles (saut, tir).

- **`setKeyRepeatEnabled()`** : Active/désactive la répétition des touches. Quand activée, maintenir une touche génère plusieurs événements `KeyPressed`. Utile pour la saisie de texte, mais généralement désactivé pour les jeux (on utilise `isKeyPressed()` pour les contrôles continus).

**Structure InputEvent :**

```cpp
enum class InputEventType {
    Closed, Resized,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved,
    TextEntered
};

struct InputEvent {
    InputEventType type;
    union {
        struct { u32 width, height; } size;              // Pour Resized
        struct { u32 unicode; } text;                    // Pour TextEntered
        struct { KeyCode code; bool alt, control, shift, system; } key; // Pour Key*
        struct { MouseButton button; i32 x, y; } mouseButton; // Pour MouseButton*
        struct { i32 x, y; } mouseMove;                  // Pour MouseMoved
    };
};
```

L'événement utilise une **union** pour économiser la mémoire : un seul champ est actif selon le type. Vérifiez toujours `type` avant d'accéder aux données.

**Exemple d'utilisation réelle dans Application.hpp (lignes 164-180) :**

```cpp
void processEvents() {
    InputEvent inputEvent;

    while (INPUT().pollEvent(inputEvent)) {
        if (inputEvent.type == InputEventType::Closed) {
            quit();
            continue;
        }

        if (inputEvent.type == InputEventType::KeyPressed) {
            if (inputEvent.key.code == KeyCode::Escape) {
                LOG_INFO("Escape pressed - quitting application");
                quit();
                continue;
            }
        }

        Event novaEvent(inputEvent);
        onEvent(novaEvent);
    }
}
```

**Codes de touche :**

L'enum `KeyCode` (BackendTypes.hpp lignes 71-77) définit toutes les touches supportées :

```cpp
enum class KeyCode {
    Unknown = -1,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, LControl, LShift, LAlt, LSystem, RControl, RShift, RAlt, RSystem,
    Space, Enter, Backspace, Tab, Left, Right, Up, Down
};
```

**Pattern typique pour contrôles de jeu :**

```cpp
// Dans votre boucle de mise à jour
Vec2f movement(0, 0);

if (INPUT().isKeyPressed(KeyCode::W)) movement.y -= 1;
if (INPUT().isKeyPressed(KeyCode::S)) movement.y += 1;
if (INPUT().isKeyPressed(KeyCode::A)) movement.x -= 1;
if (INPUT().isKeyPressed(KeyCode::D)) movement.x += 1;

// Normaliser et appliquer
if (movement.x != 0 || movement.y != 0) {
    float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    movement.x /= length;
    movement.y /= length;
    player.position += movement * speed * deltaTime;
}
```

**Gestion de la souris :**

```cpp
// Position souris en coordonnées écran
Vec2i mousePos = INPUT().getMousePosition();

// Vérifier clics
if (INPUT().isMouseButtonPressed(MouseButton::Left)) {
    // Bouton gauche enfoncé en ce moment
}

// Via événements (pour détecter le moment exact du clic)
if (event.type == InputEventType::MouseButtonPressed) {
    if (event.mouseButton.button == MouseButton::Left) {
        Vec2i clickPos(event.mouseButton.x, event.mouseButton.y);
        // Traiter le clic à cette position
    }
}
```

---

## 10. ResourceBackend - Chargement de Ressources

Le `ResourceBackend` centralise le chargement et la gestion de toutes les ressources : textures, polices, sons, musiques. Il utilise un système de **handles** (identifiants numériques u64) pour référencer les ressources de manière backend-agnostique. Le backend implémente un cache interne pour éviter de charger la même ressource plusieurs fois.

**Interface complète :**

```cpp
class IResourceBackend {
public:
    // Textures
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;
    virtual bool isTextureLoaded(TextureHandle handle) const = 0;

    // Polices
    virtual FontHandle loadFont(const String& path) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual bool isFontLoaded(FontHandle handle) const = 0;

    // Sons (effets courts joués plusieurs fois simultanément)
    virtual SoundHandle loadSound(const String& path) = 0;
    virtual void unloadSound(SoundHandle handle) = 0;
    virtual bool isSoundLoaded(SoundHandle handle) const = 0;

    // Musiques (fichiers longs streamés depuis le disque)
    virtual MusicHandle loadMusic(const String& path) = 0;
    virtual void unloadMusic(MusicHandle handle) = 0;
    virtual bool isMusicLoaded(MusicHandle handle) const = 0;

    // Gestion cache
    virtual void clearCache() = 0;
};
```

**Système de handles :**

Un handle est simplement un `u64` (entier 64 bits non signé) qui identifie une ressource de manière unique. `INVALID_HANDLE` (valeur 0) indique une ressource invalide ou non chargée.

```cpp
using TextureHandle = u64;
using FontHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
constexpr u64 INVALID_HANDLE = 0;
```

**Pourquoi des handles ?** Ils découplent le code du moteur de l'implémentation backend. Avec SFML, un handle pourrait mapper vers `sf::Texture*`. Avec SDL, vers `SDL_Texture*`. Votre code utilise toujours un `TextureHandle`.

**Chargement avec cache automatique :**

Le backend implémente un cache interne. Si vous chargez deux fois le même fichier, le backend retourne le même handle :

```cpp
TextureHandle tex1 = RESOURCES().loadTexture("assets/player.png");
TextureHandle tex2 = RESOURCES().loadTexture("assets/player.png");
// tex1 == tex2 : le fichier n'est chargé qu'une seule fois en mémoire
```

**Gestion de la durée de vie :**

Les ressources restent en mémoire jusqu'à ce que vous appeliez explicitement `unloadTexture()` / `unloadFont()` / etc. ou `clearCache()`. Le backend maintient un compteur de références interne, donc si vous chargez la même ressource 3 fois, il faut 3 appels à `unload` pour vraiment la libérer.

**Différence Sons vs Musiques :**

- **Sons (`SoundHandle`)** : Fichiers courts (< 10 secondes typiquement). Chargés entièrement en RAM. Peuvent être joués simultanément plusieurs fois (ex: bruits de pas, tirs). Formats: WAV, OGG.

- **Musiques (`MusicHandle`)** : Fichiers longs (musiques de fond). **Streamés** depuis le disque, ne chargent pas tout en RAM. Une seule musique peut jouer à la fois par défaut. Formats: OGG, FLAC, MP3.

**Exemple d'utilisation :**

```cpp
// Charger une texture pour un sprite
TextureHandle playerTexture = RESOURCES().loadTexture("assets/sprites/player.png");
if (playerTexture == INVALID_HANDLE) {
    LOG_ERROR("Failed to load player texture");
    return;
}

// Obtenir les dimensions
Vec2u texSize = RESOURCES().getTextureSize(playerTexture);
LOG_INFO("Player texture: {}x{}", texSize.x, texSize.y);

// Utiliser la texture dans un sprite (voir GraphicsBackend)
SpriteData sprite;
sprite.textureHandle = playerTexture;
sprite.position = Vec2f(100, 100);
sprite.size = Vec2f(texSize.x, texSize.y);
GRAPHICS().drawSprite(sprite);

// Libérer quand plus nécessaire (ex: fin du niveau)
RESOURCES().unloadTexture(playerTexture);
```

**Chargement de polices :**

```cpp
FontHandle font = RESOURCES().loadFont("assets/fonts/arial.ttf");
if (font == INVALID_HANDLE) {
    LOG_ERROR("Failed to load font");
}

// Utilisation avec FONTS() pour mesurer du texte
TextMetrics metrics = FONTS().measureText("Hello World", font, 24);
LOG_INFO("Text width: {}, height: {}", metrics.width, metrics.height);
```

**Gestion d'erreur robuste :**

```cpp
TextureHandle loadPlayerTexture() {
    TextureHandle handle = RESOURCES().loadTexture("assets/player.png");
    if (handle == INVALID_HANDLE) {
        LOG_WARN("Player texture not found, using fallback");
        handle = RESOURCES().loadTexture("assets/fallback.png");
    }
    return handle;
}
```

**Nettoyage global :**

```cpp
// Libérer toutes les ressources en cache (fin du jeu, changement de niveau majeur)
RESOURCES().clearCache();
```

---

## 11. AudioBackend - Gestion Audio

Le `AudioBackend` contrôle la lecture de sons et musiques. Il gère deux systèmes séparés : les **effets sonores** (multiples instances simultanées) et la **musique de fond** (une seule piste à la fois). Chaque système a son propre contrôle de volume.

**Interface complète :**

```cpp
class IAudioBackend {
public:
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Sons (effets)
    virtual void playSound(SoundHandle handle, f32 volume = 1.0f,
                          f32 pitch = 1.0f, bool loop = false) = 0;
    virtual void stopSound(SoundHandle handle) = 0;
    virtual void stopAllSounds() = 0;
    virtual void setSoundVolume(f32 volume) = 0;  // Volume global sons (0.0 à 1.0)
    virtual f32 getSoundVolume() const = 0;
    virtual SoundStatus getSoundStatus(SoundHandle handle) const = 0;

    // Musique
    virtual void playMusic(MusicHandle handle, bool loop = true) = 0;
    virtual void stopMusic() = 0;
    virtual void pauseMusic() = 0;
    virtual void resumeMusic() = 0;
    virtual void setMusicVolume(f32 volume) = 0;  // Volume global musique (0.0 à 1.0)
    virtual f32 getMusicVolume() const = 0;
    virtual SoundStatus getMusicStatus() const = 0;
};
```

**États de lecture :**

```cpp
enum class SoundStatus { Stopped, Paused, Playing };
```

**Explications détaillées :**

- **Volume** : Valeur entre 0.0 (silence) et 1.0 (volume max). Le volume global multiplicateur s'applique à tous les sons/musiques. Chaque son peut aussi avoir son propre volume lors de `playSound()`.

- **Pitch** : Hauteur tonale. 1.0 = normal, 2.0 = deux fois plus aigu (et rapide), 0.5 = deux fois plus grave (et lent). Utile pour variations (bruits de pas différents, impacts, etc.).

- **Loop** : Si `true`, le son/musique boucle indéfiniment jusqu'à `stop()`. Pour musiques de fond, presque toujours `true`. Pour sons, généralement `false`.

**Jouer un son :**

```cpp
// Charger le son d'abord
SoundHandle jumpSound = RESOURCES().loadSound("assets/sounds/jump.ogg");

// Jouer avec paramètres par défaut
AUDIO().playSound(jumpSound);

// Jouer avec volume réduit et pitch varié (pour variation)
AUDIO().playSound(jumpSound, 0.7f, 1.1f, false);

// Jouer en boucle (rare pour effets, mais possible)
AUDIO().playSound(jumpSound, 1.0f, 1.0f, true);
```

**Musique de fond :**

```cpp
// Charger et jouer musique de niveau
MusicHandle levelMusic = RESOURCES().loadMusic("assets/music/level1.ogg");
AUDIO().playMusic(levelMusic, true);  // Loop activé

// Mettre en pause pendant le menu pause
if (gamePaused) {
    AUDIO().pauseMusic();
} else {
    AUDIO().resumeMusic();
}

// Changer de musique
AUDIO().stopMusic();
MusicHandle bossMusic = RESOURCES().loadMusic("assets/music/boss.ogg");
AUDIO().playMusic(bossMusic, true);
```

**Contrôle de volume :**

```cpp
// Volume sons effets
AUDIO().setSoundVolume(0.8f);  // 80% volume

// Volume musique
AUDIO().setMusicVolume(0.5f);  // 50% volume

// Récupérer volumes actuels
f32 soundVol = AUDIO().getSoundVolume();
f32 musicVol = AUDIO().getMusicVolume();
```

**Arrêt d'urgence :**

```cpp
// Arrêter tous les sons (utile pour transitions de scène)
AUDIO().stopAllSounds();

// Arrêter la musique
AUDIO().stopMusic();
```

**Vérification d'état :**

```cpp
SoundStatus musicState = AUDIO().getMusicStatus();

if (musicState == SoundStatus::Playing) {
    LOG_INFO("Music is playing");
} else if (musicState == SoundStatus::Paused) {
    LOG_INFO("Music is paused");
} else {
    LOG_INFO("Music stopped");
}
```

**Système audio avancé avec Component :**

Le moteur fournit un `AudioComponent` (ECS) qui automatise la lecture de sons attachés aux entités :

```cpp
class AudioComponent : public Component {
public:
    SoundHandle soundHandle = INVALID_HANDLE;
    f32 volume = 1.0f;
    f32 pitch = 1.0f;
    bool loop = false;
    bool playOnStart = false;
    bool isPlaying = false;

    COMPONENT_TYPE_ID(AudioComponent)
    // ... serialize/deserialize
};
```

L'`AudioSystem` parcourt toutes les entités avec `AudioComponent` et gère leur lecture automatiquement.

---

## 12. FontBackend - Gestion des Polices

Le `FontBackend` gère le chargement de polices TrueType et permet de mesurer les dimensions du texte avant de le dessiner. C'est essentiel pour créer des interfaces utilisateur qui s'adaptent au contenu textuel.

**Interface complète :**

```cpp
class IFontBackend {
public:
    // Chargement
    virtual FontHandle loadFont(const String& path) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual bool isFontLoaded(FontHandle handle) const = 0;

    // Mesure
    virtual TextMetrics measureText(const String& text, FontHandle font,
                                   u32 characterSize,
                                   TextStyle style = TextStyle::Regular) const = 0;
    virtual f32 getLineSpacing(FontHandle font, u32 characterSize) const = 0;
};
```

**Structure TextMetrics :**

```cpp
struct TextMetrics {
    f32 width;      // Largeur totale du texte en pixels
    f32 height;     // Hauteur totale du texte en pixels
    f32 baseline;   // Distance de la baseline depuis le haut
};
```

**Styles de texte :**

```cpp
enum class TextStyle {
    Regular = 0,
    Bold = 1 << 0,          // Gras
    Italic = 1 << 1,        // Italique
    Underlined = 1 << 2,    // Souligné
    StrikeThrough = 1 << 3  // Barré
};
```

Les styles peuvent être **combinés** avec l'opérateur `|` :

```cpp
TextStyle boldItalic = TextStyle::Bold | TextStyle::Italic;
```

**Mesurer du texte :**

La mesure de texte est cruciale pour créer des boutons, boîtes de dialogue, et UI adaptatives :

```cpp
FontHandle font = FONTS().loadFont("assets/fonts/roboto.ttf");

// Mesurer une chaîne
TextMetrics metrics = FONTS().measureText("Score: 1000", font, 24);
LOG_INFO("Text dimensions: {}x{}", metrics.width, metrics.height);

// Créer un bouton adaptatif
RectData buttonBg;
buttonBg.position = Vec2f(100, 100);
buttonBg.size = Vec2f(metrics.width + 20, metrics.height + 10);  // +padding
buttonBg.fillColor = Color(50, 50, 200);
GRAPHICS().drawRect(buttonBg);

// Centrer le texte dans le bouton
TextData buttonText;
buttonText.text = "Score: 1000";
buttonText.font = font;
buttonText.characterSize = 24;
buttonText.position = Vec2f(
    buttonBg.position.x + 10,  // padding gauche
    buttonBg.position.y + 5    // padding haut
);
GRAPHICS().drawText(buttonText);
```

**Espacement de lignes :**

Pour du texte multi-lignes, utilisez `getLineSpacing()` pour calculer l'espacement vertical :

```cpp
FontHandle font = FONTS().loadFont("assets/fonts/arial.ttf");
f32 lineSpacing = FONTS().getLineSpacing(font, 16);

std::vector<std::string> lines = {"Line 1", "Line 2", "Line 3"};
Vec2f position(10, 10);

for (const auto& line : lines) {
    TextData text;
    text.text = line;
    text.font = font;
    text.characterSize = 16;
    text.position = position;
    GRAPHICS().drawText(text);

    position.y += lineSpacing;  // Descendre à la ligne suivante
}
```

**Texte avec styles combinés :**

```cpp
TextData fancyText;
fancyText.text = "Important!";
fancyText.font = font;
fancyText.characterSize = 32;
fancyText.fillColor = Color::Red;
fancyText.outlineColor = Color::Black;
fancyText.outlineThickness = 2.0f;
fancyText.style = TextStyle::Bold | TextStyle::Underlined;  // Gras ET souligné
fancyText.position = Vec2f(100, 100);

GRAPHICS().drawText(fancyText);
```

**Vérification de chargement :**

```cpp
FontHandle font = FONTS().loadFont("assets/fonts/custom.ttf");
if (font == INVALID_HANDLE || !FONTS().isFontLoaded(font)) {
    LOG_ERROR("Font loading failed");
    font = FONTS().loadFont("assets/fonts/fallback.ttf");  // Police de secours
}
```

---

## 13. ViewportBackend - Caméra et Vues

Le `ViewportBackend` gère la **caméra virtuelle** qui détermine quelle partie du monde est visible à l'écran. Il permet le scrolling, le zoom, et la conversion entre coordonnées écran et monde. C'est essentiel pour les jeux 2D avec défilement (platformers, RPGs, etc.).

**Interface complète :**

```cpp
class IViewportBackend {
public:
    // Configuration viewport (portion de la fenêtre utilisée pour rendu)
    virtual void setViewport(const Rect& viewport) = 0;
    virtual Rect getViewport() const = 0;
    virtual void resetViewport() = 0;

    // Vue (caméra virtuelle)
    virtual void setView(const ViewportData& view) = 0;
    virtual ViewportData getView() const = 0;
    virtual void resetView() = 0;

    // Manipulation caméra
    virtual void setViewCenter(const Vec2f& center) = 0;
    virtual Vec2f getViewCenter() const = 0;
    virtual void setViewSize(const Vec2f& size) = 0;
    virtual Vec2f getViewSize() const = 0;
    virtual void moveView(const Vec2f& offset) = 0;
    virtual void zoomView(f32 factor) = 0;

    // Conversions coordonnées
    virtual Vec2f screenToWorld(const Vec2i& point) const = 0;
    virtual Vec2i worldToScreen(const Vec2f& point) const = 0;
};
```

**Concepts clés :**

- **Viewport** : Rectangle normalisé [0,1] définissant quelle portion de la fenêtre est utilisée pour le rendu. Par défaut (0, 0, 1, 1) = toute la fenêtre. Utilisé pour split-screen ou mini-maps.

- **View (Vue)** : Caméra virtuelle définissant quelle partie du monde est visible. Définie par un centre et une taille.

**Structure ViewportData :**

```cpp
struct ViewportData {
    Rect viewport;   // Zone de rendu normalisée (0-1)
    Vec2f center;    // Centre de la caméra dans le monde
    Vec2f size;      // Taille de la zone visible
    f32 rotation;    // Rotation de la vue (rarement utilisé)
};
```

**Suivre un joueur avec la caméra :**

```cpp
// Centrer la caméra sur le joueur
Vec2f playerPos = player.getPosition();
VIEWPORT().setViewCenter(playerPos);
```

**Caméra avec limites (ne pas sortir du niveau) :**

```cpp
void updateCamera(const Vec2f& playerPos, const Rect& levelBounds) {
    Vec2f viewSize = VIEWPORT().getViewSize();
    Vec2f cameraPos = playerPos;

    // Limiter la caméra aux bords du niveau
    f32 halfWidth = viewSize.x / 2.0f;
    f32 halfHeight = viewSize.y / 2.0f;

    if (cameraPos.x - halfWidth < levelBounds.left) {
        cameraPos.x = levelBounds.left + halfWidth;
    }
    if (cameraPos.x + halfWidth > levelBounds.left + levelBounds.width) {
        cameraPos.x = levelBounds.left + levelBounds.width - halfWidth;
    }
    if (cameraPos.y - halfHeight < levelBounds.top) {
        cameraPos.y = levelBounds.top + halfHeight;
    }
    if (cameraPos.y + halfHeight > levelBounds.top + levelBounds.height) {
        cameraPos.y = levelBounds.top + levelBounds.height - halfHeight;
    }

    VIEWPORT().setViewCenter(cameraPos);
}
```

**Zoom :**

```cpp
// Zoomer de 50% (zoom in)
VIEWPORT().zoomView(0.5f);

// Dézoomer de 200% (zoom out)
VIEWPORT().zoomView(2.0f);

// Réinitialiser le zoom
VIEWPORT().setViewSize(Vec2f(1920, 1080));  // Taille d'origine
```

**Déplacement caméra (scrolling manuel) :**

```cpp
// Déplacer la caméra de 10 pixels vers la droite
VIEWPORT().moveView(Vec2f(10, 0));

// Contrôles caméra avec touches fléchées
if (INPUT().isKeyPressed(KeyCode::Left))  VIEWPORT().moveView(Vec2f(-5, 0));
if (INPUT().isKeyPressed(KeyCode::Right)) VIEWPORT().moveView(Vec2f(5, 0));
if (INPUT().isKeyPressed(KeyCode::Up))    VIEWPORT().moveView(Vec2f(0, -5));
if (INPUT().isKeyPressed(KeyCode::Down))  VIEWPORT().moveView(Vec2f(0, 5));
```

**Conversion coordonnées écran ↔ monde :**

Essentiel pour détecter où l'utilisateur clique dans le monde du jeu :

```cpp
// Récupérer position de la souris en coordonnées écran
Vec2i mouseScreen = INPUT().getMousePosition();

// Convertir en coordonnées monde (tient compte caméra et zoom)
Vec2f mouseWorld = VIEWPORT().screenToWorld(mouseScreen);

LOG_INFO("Mouse screen: ({}, {}), world: ({}, {})",
         mouseScreen.x, mouseScreen.y, mouseWorld.x, mouseWorld.y);

// Vérifier si la souris est sur une entité
if (entityRect.contains(mouseWorld)) {
    LOG_INFO("Mouse over entity!");
}
```

**Inverse (monde → écran) :**

```cpp
Vec2f entityWorldPos(500, 300);
Vec2i entityScreenPos = VIEWPORT().worldToScreen(entityWorldPos);

// Dessiner un indicateur UI à l'écran au-dessus de l'entité
// (réinitialiser la vue pour dessiner en coordonnées écran)
VIEWPORT().resetView();
drawHealthBarAt(entityScreenPos);
// Puis remettre la vue caméra pour continuer le rendu monde
```

**Split-screen (2 joueurs) :**

```cpp
// Joueur 1 : moitié gauche de l'écran
VIEWPORT().setViewport(Rect(0.0f, 0.0f, 0.5f, 1.0f));  // x:[0-0.5], y:[0-1]
VIEWPORT().setViewCenter(player1Pos);
renderWorld();

// Joueur 2 : moitié droite de l'écran
VIEWPORT().setViewport(Rect(0.5f, 0.0f, 0.5f, 1.0f));  // x:[0.5-1], y:[0-1]
VIEWPORT().setViewCenter(player2Pos);
renderWorld();

// Réinitialiser pour UI
VIEWPORT().resetViewport();
```

---

## 14. Logger - Système de Journalisation

Le `Logger` est un singleton thread-safe qui enregistre des messages dans la console et optionnellement dans un fichier. Il supporte 6 niveaux de log avec formatage style `{}` et coloration ANSI optionnelle.

**Niveaux de log :**

```cpp
enum class LogLevel {
    Trace,    // Détails ultra-verbeux (debug profond)
    Debug,    // Informations de débogage
    Info,     // Informations générales
    Warning,  // Avertissements (non-bloquants)
    Error,    // Erreurs (mais le programme continue)
    Fatal     // Erreurs fatales (crash imminent)
};
```

**Macros de logging :**

Le moteur fournit 6 macros qui incluent automatiquement le nom du fichier source :

```cpp
LOG_TRACE("Player position: ({}, {})", x, y);
LOG_DEBUG("Loading scene: {}", sceneName);
LOG_INFO("Game started successfully");
LOG_WARN("Texture not found: {}, using fallback", path);
LOG_ERROR("Failed to parse JSON: {}", error);
LOG_FATAL("Out of memory!");
```

**Formatage style `{}` :**

Le système utilise un formatage simple où chaque `{}` est remplacé par l'argument correspondant :

```cpp
LOG_INFO("Player {} has {} health", playerName, health);
// Sortie: "Player John has 100 health"

LOG_DEBUG("Position: ({}, {}), Velocity: ({}, {})",
          pos.x, pos.y, vel.x, vel.y);
```

**Configuration du logger :**

```cpp
// Définir le niveau minimal (masquer Trace et Debug en production)
Logger::getInstance().setLogLevel(LogLevel::Info);

// Activer l'écriture dans un fichier
Logger::getInstance().setLogFile("logs/game.log");

// Activer/désactiver les couleurs ANSI (désactiver pour fichiers)
Logger::getInstance().enableAnsiColors(true);
```

**Couleurs ANSI :**

Quand activées, chaque niveau a sa couleur dans la console :
- **Trace** : Gris
- **Debug** : Cyan
- **Info** : Vert
- **Warning** : Jaune
- **Error** : Rouge
- **Fatal** : Rouge gras

**Format de sortie :**

Chaque message inclut automatiquement :

```
[timestamp] [niveau] [fichier] message
```

Exemple :

```
[2025-01-15 14:32:10] [INFO] [Main.cpp] Game started successfully
[2025-01-15 14:32:11] [WARN] [ResourceManager.cpp] Texture not found: player.png
[2025-01-15 14:32:11] [ERROR] [Scene.cpp] Failed to load scene: invalid JSON
```

**Thread-safety :**

Le logger utilise un `std::mutex` interne, donc vous pouvez logger depuis n'importe quel thread sans risque de corruption :

```cpp
// Thread 1
LOG_INFO("Loading resources...");

// Thread 2 (simultané)
LOG_DEBUG("Physics update tick");

// Pas de race condition, les messages sont sérialisés
```

**Désactiver le logging :**

Pour builds de release, vous pouvez désactiver complètement le logging en définissant `NOVA_LOGGING_ENABLED false` dans Logger.hpp (ligne 16). Toutes les macros deviennent des no-ops et le compilateur les élimine.

**Utilisation typique dans une application :**

```cpp
int main() {
    // Configuration initiale
    Logger::getInstance().setLogFile("logs/nova.log");
    Logger::getInstance().setLogLevel(LogLevel::Debug);
    Logger::getInstance().enableAnsiColors(true);

    LOG_INFO("=== Application Starting ===");

    if (!engine.initialize()) {
        LOG_FATAL("Engine initialization failed");
        return -1;
    }

    LOG_INFO("Engine initialized successfully");

    // ... code du jeu ...

    LOG_INFO("=== Application Exiting ===");
    return 0;
}
```

---

## 15. ConfigManager - Gestion de Configuration

Le `ConfigManager` est un singleton qui charge et sauvegarde la configuration du moteur depuis des fichiers `.ini`. Il organise la configuration en 5 catégories : Display, Audio, Input, Debug, et Game.

**Structures de configuration :**

```cpp
struct DisplayConfig {
    u32 width = 1920;
    u32 height = 1080;
    bool fullscreen = false;
    bool vsync = true;
    u32 frameRateLimit = 60;
    u32 antialiasingLevel = 0;
    u32 nativeWidth = 3840;   // Résolution native pour rescaling
    u32 nativeHeight = 2160;
};

struct AudioConfig {
    f32 masterVolume = 100.0f;
    f32 musicVolume = 80.0f;
    f32 soundVolume = 90.0f;
    bool muteAll = false;
    std::string audioDevice = "default";
};

struct InputConfig {
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
    u32 autoSaveInterval = 300;  // secondes
    std::string savePath = "saves/";
};
```

**Chargement de configuration :**

```cpp
// Charger depuis le fichier par défaut
ConfigManager::getInstance().loadFromFile("config/engine.ini");

// Ou créer un fichier par défaut s'il n'existe pas
if (!ConfigManager::getInstance().loadFromFile("config/engine.ini")) {
    LOG_WARN("Config file not found, creating default");
    ConfigManager::getInstance().createDefaultConfig("config/engine.ini");
    ConfigManager::getInstance().loadFromFile("config/engine.ini");
}
```

**Accès aux configurations :**

```cpp
// Lecture
const DisplayConfig& display = CONFIG().getDisplayConfig();
u32 width = display.width;
u32 height = display.height;

// Modification
DisplayConfig& display = CONFIG().getDisplayConfig();
display.width = 2560;
display.height = 1440;
display.fullscreen = true;

// Sauvegarder les changements
CONFIG().saveToFile("config/engine.ini");
```

**Macros d'accès rapide :**

```cpp
#define DISPLAY_CONFIG ConfigManager::getInstance().getDisplayConfig()
#define AUDIO_CONFIG ConfigManager::getInstance().getAudioConfig()
#define INPUT_CONFIG ConfigManager::getInstance().getInputConfig()
#define DEBUG_CONFIG ConfigManager::getInstance().getDebugConfig()
#define GAME_CONFIG ConfigManager::getInstance().getGameConfig()
```

Utilisation :

```cpp
u32 screenWidth = DISPLAY_CONFIG.width;
f32 masterVol = AUDIO_CONFIG.masterVolume;
std::string lang = GAME_CONFIG.language;
```

**Format de fichier INI :**

Le format est simple et lisible :

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
muteAll=false

[Debug]
enableLogging=true
logLevel=INFO
logFile=logs/nova.log
showFPS=true

[Game]
language=fr
playerName=Joueur1
autoSave=true
```

**Validation et clamping :**

Le ConfigManager valide automatiquement les valeurs lors du chargement :

```cpp
// Valider et corriger les valeurs invalides
CONFIG().clampValues();  // Clamp volumes entre 0-100, résolutions min/max, etc.

bool valid = CONFIG().validateDisplayConfig();
if (!valid) {
    LOG_WARN("Invalid display config, using defaults");
}
```

**Debug de la configuration :**

```cpp
// Afficher toute la configuration dans les logs
CONFIG().debugPrint();

// Obtenir la config sous forme de chaîne
std::string configStr = CONFIG().getConfigString();
LOG_INFO("Current config:\n{}", configStr);
```

**Utilisation typique au démarrage :**

```cpp
int main() {
    // Charger la configuration
    if (!ConfigManager::getInstance().loadFromFile("config/engine.ini")) {
        ConfigManager::getInstance().createDefaultConfig("config/engine.ini");
        ConfigManager::getInstance().loadFromFile("config/engine.ini");
    }

    // Initialiser le logger selon la config
    const DebugConfig& debug = DEBUG_CONFIG;
    if (debug.enableLogging) {
        Logger::getInstance().setLogFile(debug.logFile);
        // Convertir logLevel string en enum...
    }

    // Initialiser le backend avec config display
    const DisplayConfig& display = DISPLAY_CONFIG;
    BACKEND().initialize(BackendType::SFML,
                        display.width, display.height,
                        "My Game", display.fullscreen);

    WINDOW().setVSync(display.vsync);
    if (display.frameRateLimit > 0) {
        WINDOW().setFramerateLimit(display.frameRateLimit);
    }

    // Configurer audio
    const AudioConfig& audio = AUDIO_CONFIG;
    AUDIO().setMusicVolume(audio.musicVolume / 100.0f);
    AUDIO().setSoundVolume(audio.soundVolume / 100.0f);

    // ... lancer le jeu ...

    return 0;
}
```

---

## 16. NovaEngine - Classe Principale (Legacy)

**Note** : La classe `NovaEngine` (NovaEngine.hpp) est l'ancienne interface héritée de SFML. Pour les nouveaux projets, utilisez plutôt la classe `Application` (section suivante) qui utilise le système backend abstrait.

La classe `NovaEngine` fournit un singleton pour gérer la fenêtre SFML, les ressources, et les événements de manière directe. Elle existe pour compatibilité avec l'ancien code mais n'utilise pas le système Backend abstrait.

**Interface héritée :**

```cpp
class NovaEngine {
public:
    static NovaEngine& get();

    // Initialisation
    bool initialize(const std::string& title, u32 width, u32 height,
                   bool fullscreen = false);
    bool initializeWithConfig(const std::string& title,
                             const std::string& configPath = "config/engine.ini");

    // Boucle principale
    void run();
    void shutdown();

    // Accès fenêtre SFML
    sf::RenderWindow& getWindow();

    // Accès composants
    ResourceManager& getResourceManager();
    EventDispatcher& getEventDispatcher();

    // État
    bool isRunning() const;

    // Configuration affichage
    void applyDisplaySettings(const DisplayConfig& config);
    u32 getWidth() const;
    u32 getHeight() const;
    bool isFullscreen() const;
    void toggleFullscreen();
    void setTitle(const std::string& title);
};
```

**Recommandation** : Pour les nouveaux projets, utilisez la classe `Application` (section 17) qui est plus moderne et portable.

---

## 17. Application - Classe de Base Moderne

La classe `Application` est la fondation recommandée pour créer des applications avec NovaEngine. Elle utilise le système Backend abstrait, gère automatiquement la boucle principale, et fournit des hooks virtuels pour initialisation, mise à jour, et rendu.

**Philosophie :**

Au lieu d'écrire votre propre boucle `while(running)`, vous **héritez** de `Application` et implémentez les méthodes virtuelles `onInitialize()`, `onUpdate()`, `onRender()`. La classe de base gère tout le boilerplate : initialisation du backend, boucle de jeu, gestion d'événements, calcul de deltaTime, et shutdown propre.

**Interface (Application.hpp lignes 13-187) :**

```cpp
class Application {
public:
    struct Config {
        String windowTitle = "NovaEngine Application";
        u32 windowWidth = 1920;
        u32 windowHeight = 1080;
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

public:
    Application() = default;
    explicit Application(const Config& config);
    virtual ~Application() = default;

    // Méthode principale - lance l'application
    int run();

    // Quitter l'application
    void quit();

    // Getters
    float getDeltaTime() const;
    const Config& getConfig() const;
    bool isInitialized() const;

protected:
    // Hooks virtuels à implémenter
    virtual bool onInitialize() = 0;     // Initialisation de votre jeu
    virtual void onUpdate(float deltaTime) = 0;  // Mise à jour logique
    virtual void onRender() = 0;         // Rendu graphique

    // Hooks optionnels
    virtual void onEvent(const Event& event) {}  // Traitement d'événements
    virtual void onShutdown() {}         // Nettoyage avant fermeture
};
```

**Créer votre première application :**

```cpp
#include <NovaEngine/Core/Application.hpp>

class MyGame : public NovaEngine::Application {
private:
    NovaEngine::TextureHandle m_playerTexture;
    NovaEngine::Vec2f m_playerPos;

public:
    MyGame() {
        m_config.windowTitle = "Mon Jeu";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vSync = true;
    }

    bool onInitialize() override {
        LOG_INFO("Game initializing...");

        // Charger ressources
        m_playerTexture = RESOURCES().loadTexture("assets/player.png");
        if (m_playerTexture == INVALID_HANDLE) {
            LOG_ERROR("Failed to load player texture");
            return false;
        }

        m_playerPos = Vec2f(640, 360);
        return true;
    }

    void onUpdate(float deltaTime) override {
        // Déplacement du joueur
        Vec2f movement(0, 0);
        const float speed = 200.0f;

        if (INPUT().isKeyPressed(KeyCode::W)) movement.y -= 1;
        if (INPUT().isKeyPressed(KeyCode::S)) movement.y += 1;
        if (INPUT().isKeyPressed(KeyCode::A)) movement.x -= 1;
        if (INPUT().isKeyPressed(KeyCode::D)) movement.x += 1;

        if (movement.x != 0 || movement.y != 0) {
            float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            movement.x /= len;
            movement.y /= len;
            m_playerPos += movement * speed * deltaTime;
        }
    }

    void onRender() override {
        // Dessiner le joueur
        SpriteData sprite;
        sprite.textureHandle = m_playerTexture;
        sprite.position = m_playerPos;
        sprite.size = Vec2f(64, 64);
        GRAPHICS().drawSprite(sprite);
    }

    void onEvent(const Event& event) override {
        // Traiter événements personnalisés
    }

    void onShutdown() override {
        LOG_INFO("Game shutting down");
        RESOURCES().unloadTexture(m_playerTexture);
    }
};

int main() {
    MyGame game;
    return game.run();
}
```

**Flux d'exécution de `run()` (lignes 54-83) :**

1. **Initialisation du moteur** : Appelle `BACKEND().initialize()` avec les paramètres de `Config`
2. **Configuration fenêtre** : Active VSync et limite FPS selon `Config`
3. **Initialisation utilisateur** : Appelle votre `onInitialize()` - **retour false = échec fatal**
4. **Boucle principale** : Tant que la fenêtre est ouverte :
   - Calcule `deltaTime` (temps écoulé depuis dernier frame)
   - Traite tous les événements (`pollEvent` + gestion Escape et Closed)
   - Appelle votre `onUpdate(deltaTime)`
   - Clear fenêtre avec `clearColor`
   - Appelle votre `onRender()`
   - `display()` pour afficher le frame
5. **Shutdown** : Appelle votre `onShutdown()` puis `BACKEND().shutdown()`

**Gestion automatique Escape et fermeture fenêtre (lignes 161-180) :**

```cpp
void processEvents() {
    InputEvent inputEvent;

    while (INPUT().pollEvent(inputEvent)) {
        // Fermeture fenêtre
        if (inputEvent.type == InputEventType::Closed) {
            quit();
            continue;
        }

        // Touche Escape = quitter
        if (inputEvent.type == InputEventType::KeyPressed) {
            if (inputEvent.key.code == KeyCode::Escape) {
                LOG_INFO("Escape pressed - quitting application");
                quit();
                continue;
            }
        }

        // Propager l'événement à l'utilisateur
        Event novaEvent(inputEvent);
        onEvent(novaEvent);
    }
}
```

**DeltaTime :**

Le `deltaTime` représente le temps écoulé en secondes depuis le dernier frame. Utilisez-le pour **rendre les mouvements indépendants du framerate** :

```cpp
// MAUVAIS : dépend du FPS (60 FPS = 60px/sec, 120 FPS = 120px/sec)
player.x += 1;

// BON : indépendant du FPS (toujours 100px/sec quel que soit le FPS)
player.x += 100 * deltaTime;
```

**Configuration avancée :**

```cpp
MyGame::MyGame() {
    m_config.windowTitle = "Mon RPG";
    m_config.windowWidth = 1920;
    m_config.windowHeight = 1080;
    m_config.fullscreen = true;
    m_config.frameRateLimit = 144;  // 144 FPS max
    m_config.vSync = false;         // Désactiver VSync pour FPS illimité
    m_config.clearColor = Color(20, 20, 40);  // Fond bleu foncé
}
```

**Quitter proprement :**

```cpp
void onUpdate(float deltaTime) override {
    if (playerDead) {
        LOG_INFO("Player died, exiting in 3 seconds...");
        // quit() fermera proprement après le frame actuel
        quit();
    }
}
```

---

## PARTIE III : ENTITY COMPONENT SYSTEM (ECS)

L'ECS (Entity Component System) est le cœur architectural du moteur NovaEngine. C'est un paradigme de **composition** où les entités sont des conteneurs vides, les composants sont des données pures, et les systèmes sont la logique pure.

### Philosophie ECS

**Problème de l'héritage profond :**

```cpp
// Approche orientée objet classique (à éviter)
class GameObject {};
class MovableObject : public GameObject {};
class AnimatedObject : public MovableObject {};
class Enemy : public AnimatedObject {};
class FlyingEnemy : public Enemy {};  // Hiérarchie profonde, rigide
```

**Solution ECS :**

```cpp
// Entité = ID + Composants
Entity enemy(42);
enemy.addComponent<TransformComponent>();  // Position, rotation
enemy.addComponent<SpriteComponent>();     // Rendu visuel
enemy.addComponent<AnimationComponent>();  // Animations
enemy.addComponent<ColliderComponent>();   // Collisions
// Flexible! On peut ajouter/retirer des composants dynamiquement
```

**Avantages :**

- **Composition** : Combinez des comportements librement
- **Data-Oriented** : Les données (Components) sont séparées de la logique (Systems)
- **Performance** : Cache-friendly, facile à optimiser
- **Flexibilité** : Ajoutez/retirez fonctionnalités sans refactoring massif

## 18. Component - Conteneurs de Données

Un `Component` est une **structure de données pure** sans logique. Il représente un aspect d'une entité (position, apparence, son, etc.).

**Classe de base (Component.hpp lignes 10-44) :**

```cpp
using ComponentTypeID = std::string;

class Component {
public:
    virtual ~Component() = default;

    // Identification du type de composant
    virtual ComponentTypeID getTypeID() const = 0;

    // Sérialisation JSON
    virtual void serialize(nlohmann::json& json) const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
};

// Macro helper pour implémenter getTypeID()
#define COMPONENT_TYPE_ID(TypeName) \
    ComponentTypeID getTypeID() const override { return #TypeName; }
```

**Créer un composant personnalisé :**

```cpp
class HealthComponent : public Component {
public:
    f32 current = 100.0f;
    f32 maximum = 100.0f;

    COMPONENT_TYPE_ID(HealthComponent)

    void serialize(nlohmann::json& json) const override {
        json["current"] = current;
        json["maximum"] = maximum;
    }

    void deserialize(const nlohmann::json& json) override {
        current = json.value("current", 100.0f);
        maximum = json.value("maximum", 100.0f);
    }
};
```

**Composants built-in (Components.hpp) :**

NovaEngine fournit 10+ composants prêts à l'emploi :

1. **TransformComponent** : Position, rotation, échelle
2. **SpriteComponent** : Rendu de sprites
3. **AnimationComponent** : Animations sprite sheet
4. **LightComponent** : Lumières 2D
5. **ColliderComponent** : Boîtes de collision
6. **AudioComponent** : Effets sonores
7. **TagComponent** : Tags pour identification
8. **ActivatorComponent** : Zones déclencheurs
9. **JourneyComponent** : Déplacements multi-scènes NPCs
10. **SceneTransitionComponent** : Transitions entre scènes

**Exemple TransformComponent :**

```cpp
class TransformComponent : public Component {
public:
    Vec2f position = Vec2f(0, 0);
    f32 rotation = 0.0f;       // En degrés
    Vec2f scale = Vec2f(1, 1);

    COMPONENT_TYPE_ID(TransformComponent)

    void serialize(nlohmann::json& json) const override {
        json["position"] = {position.x, position.y};
        json["rotation"] = rotation;
        json["scale"] = {scale.x, scale.y};
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("position")) {
            auto& pos = json["position"];
            position = Vec2f(pos[0], pos[1]);
        }
        if (json.contains("rotation")) {
            rotation = json["rotation"];
        }
        if (json.contains("scale")) {
            auto& sc = json["scale"];
            scale = Vec2f(sc[0], sc[1]);
        }
    }
};
```

**Règles des composants :**

- ✅ **Données seulement** : Pas de méthodes de logique complexe
- ✅ **Petits et focalisés** : Un composant = un aspect (position, santé, sprite, etc.)
- ✅ **Sérialisables** : Doivent pouvoir être sauvegardés/chargés en JSON
- ❌ **Pas de références externes** : Pas de pointeurs vers autres entités (utilisez des IDs)
- ❌ **Pas de logique de jeu** : La logique va dans les Systems

## 19. Entity - Conteneur de Composants

Une `Entity` est simplement un **conteneur identifié** qui possède des composants. Elle n'a aucune logique propre, juste des méthodes pour gérer ses composants.

**Interface (Entity.hpp lignes 9-144) :**

```cpp
class Entity {
private:
    u64 m_id;
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
    explicit Entity(u64 id);

    u64 getID() const;

    // Ajouter un composant (transfert ownership)
    template<typename T>
    T* addComponent(std::unique_ptr<T> component);

    // Récupérer un composant
    template<typename T>
    T* getComponent();

    template<typename T>
    const T* getComponent() const;

    // Vérifier présence
    template<typename T>
    bool hasComponent() const;

    bool hasComponent(const ComponentTypeID& typeID) const;

    // Retirer un composant
    template<typename T>
    void removeComponent();

    // Lister tous les types de composants
    std::vector<ComponentTypeID> getComponentTypes() const;
};
```

**Utilisation typique :**

```cpp
// Créer une entité
Entity player(1);

// Ajouter des composants
auto* transform = player.addComponent(std::make_unique<TransformComponent>());
transform->position = Vec2f(100, 200);

auto* sprite = player.addComponent(std::make_unique<SpriteComponent>());
sprite->textureHandle = RESOURCES().loadTexture("player.png");
sprite->size = Vec2f(64, 64);

auto* health = player.addComponent(std::make_unique<HealthComponent>());
health->current = 100.0f;
health->maximum = 100.0f;

// Récupérer un composant
TransformComponent* t = player.getComponent<TransformComponent>();
if (t) {
    t->position.x += 10;  // Déplacer
}

// Vérifier présence
if (player.hasComponent<HealthComponent>()) {
    LOG_INFO("Player has health");
}

// Retirer un composant
player.removeComponent<HealthComponent>();
```

**Stockage interne :**

Les composants sont stockés dans une `std::unordered_map<string, unique_ptr<Component>>`. La clé est le nom de la classe (via `getTypeID()`). Cela permet :

- **Recherche O(1)** par type de composant
- **Ownership automatique** : `unique_ptr` gère la mémoire
- **Type-safety** : Le template garantit le bon type

**Pattern commun : composants requis :**

```cpp
void updatePlayer(Entity& player) {
    // Vérifier que l'entité a tous les composants requis
    auto* transform = player.getComponent<TransformComponent>();
    auto* sprite = player.getComponent<SpriteComponent>();

    if (!transform || !sprite) {
        LOG_WARN("Player missing required components");
        return;
    }

    // Logique garantie de fonctionner
    sprite->position = transform->position;
}
```

## 20. System - Logique de Jeu

Un `System` contient la **logique pure** qui opère sur des entités ayant certains composants. Les systèmes ne stockent pas de données, ils **transforment** les données des composants chaque frame.

**Classe de base (System.hpp lignes 9-45) :**

```cpp
class System {
public:
    virtual ~System() = default;

    // Mise à jour principale
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;

    // Composants requis
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;

    // Hooks optionnels
    virtual void onInit() {}
    virtual void onShutdown() {}
};
```

**Créer un système personnalisé :**

```cpp
class HealthRegenerationSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // Parcourir toutes les entités qui ont TransformComponent ET HealthComponent
        for (auto* entity : registry.getEntities()) {
            auto* health = entity->getComponent<HealthComponent>();
            auto* transform = entity->getComponent<TransformComponent>();

            if (!health || !transform) continue;  // Skip si composants manquants

            // Régénération de santé : +5 HP/seconde
            if (health->current < health->maximum) {
                health->current += 5.0f * deltaTime;
                if (health->current > health->maximum) {
                    health->current = health->maximum;
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"HealthComponent"};  // Minimum requis
    }
};
```

**Systèmes built-in :**

NovaEngine fournit plusieurs systèmes prêts à l'emploi (Systems.hpp) :

1. **RenderSystem** : Rendu de tous les sprites
2. **AnimationSystem** : Mise à jour des animations
3. **LightSystem** : Rendu des lumières 2D
4. **AudioSystem** : Gestion sons attachés aux entités
5. **PhysicsSystem** : Collisions et déplacements
6. **ActivatorSystem** : Détection zones trigger
7. **JourneySystem** : Navigation multi-scène des NPCs

**Exemple concret : RenderSystem (Systems.hpp) :**

```cpp
class RenderSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // Collecter toutes les entités avec Transform + Sprite
        std::vector<std::pair<f32, Entity*>> renderables;

        for (auto* entity : registry.getEntities()) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* sprite = entity->getComponent<SpriteComponent>();

            if (!transform || !sprite) continue;

            // Calculer Y pour tri (draw order)
            f32 sortKey = transform->position.y + sprite->zOffset;
            renderables.push_back({sortKey, entity});
        }

        // Trier par Y (entités plus hautes dessinées en premier)
        std::sort(renderables.begin(), renderables.end(),
                 [](const auto& a, const auto& b) { return a.first < b.first; });

        // Dessiner dans l'ordre
        for (const auto& [y, entity] : renderables) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* sprite = entity->getComponent<SpriteComponent>();

            SpriteData data;
            data.textureHandle = sprite->textureHandle;
            data.position = transform->position;
            data.size = sprite->size;
            data.rotation = transform->rotation;
            data.scale = transform->scale;
            data.color = sprite->color;
            data.textureRect = sprite->textureRect;

            GRAPHICS().drawSprite(data);
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SpriteComponent"};
    }
};
```

**Ordre des systèmes :**

L'ordre d'exécution des systèmes est **crucial**. Exemple dans une Scene (Scene.hpp lignes 39-46) :

```cpp
m_systems.push_back(std::make_unique<AnimationSystem>());  // 1. Mettre à jour animations
m_systems.push_back(std::make_unique<PhysicsSystem>());    // 2. Mettre à jour positions
m_systems.push_back(std::make_unique<ActivatorSystem>());  // 3. Détecter triggers
m_systems.push_back(std::make_unique<AudioSystem>());      // 4. Jouer sons
m_systems.push_back(std::make_unique<LightSystem>());      // 5. Dessiner lumières
m_systems.push_back(std::make_unique<RenderSystem>());     // 6. Dessiner sprites (dernier!)
```

**Pourquoi cet ordre ?**

- Animation met à jour le `textureRect` du sprite avant le rendu
- Physics déplace les entités avant de vérifier les collisions
- Activators doivent voir les positions finales après physics
- RenderSystem doit être **dernier** pour dessiner l'état final

**Pattern : Système avec état :**

```cpp
class ParticleSystem : public System {
private:
    struct Particle {
        Vec2f position, velocity;
        f32 lifetime;
    };
    std::vector<Particle> m_particles;

public:
    void emitParticle(Vec2f pos, Vec2f vel) {
        m_particles.push_back({pos, vel, 2.0f});  // 2 secondes de vie
    }

    void update(float deltaTime, EntityRegistry& registry) override {
        // Mettre à jour particules
        for (auto it = m_particles.begin(); it != m_particles.end();) {
            it->position += it->velocity * deltaTime;
            it->lifetime -= deltaTime;

            if (it->lifetime <= 0) {
                it = m_particles.erase(it);
            } else {
                ++it;
            }
        }

        // Dessiner particules
        for (const auto& p : m_particles) {
            RectData rect;
            rect.position = p.position;
            rect.size = Vec2f(2, 2);
            rect.fillColor = Color::White;
            GRAPHICS().drawRect(rect);
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {};  // Pas besoin de composants spécifiques
    }
};
```

---

## 21. EntityRegistry - Gestion des Entités

L'`EntityRegistry` est le conteneur central qui gère toutes les entités d'une scène. Il fournit des méthodes pour créer, détruire, et interroger les entités.

**Interface (EntityRegistry.hpp) :**

```cpp
class EntityRegistry {
private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    u64 m_nextEntityID = 1;

public:
    // Création
    Entity* createEntity();
    Entity* createEntityWithID(u64 id);

    // Destruction
    void destroyEntity(u64 id);
    void clear();

    // Recherche
    Entity* getEntity(u64 id);
    const Entity* getEntity(u64 id) const;

    // Interrogation
    std::vector<Entity*> getEntities();
    std::vector<const Entity*> getEntities() const;

    std::vector<Entity*> getEntitiesWithComponent(const ComponentTypeID& typeID);

    template<typename... ComponentTypes>
    std::vector<Entity*> getEntitiesWithComponents();

    // Taille
    size_t getEntityCount() const;
};
```

**Utilisation typique :**

```cpp
EntityRegistry registry;

// Créer des entités
Entity* player = registry.createEntity();
player->addComponent(std::make_unique<TransformComponent>());
player->addComponent(std::make_unique<SpriteComponent>());

Entity* enemy1 = registry.createEntity();
Entity* enemy2 = registry.createEntity();

// Trouver une entité par ID
u64 playerID = player->getID();
Entity* found = registry.getEntity(playerID);

// Obtenir toutes les entités
for (auto* entity : registry.getEntities()) {
    LOG_INFO("Entity ID: {}", entity->getID());
}

// Filtrer par composants
auto movables = registry.getEntitiesWithComponent("TransformComponent");
LOG_INFO("{} entities have TransformComponent", movables.size());

// Filtrer par plusieurs composants (variadique)
auto renderables = registry.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

// Détruire une entité
registry.destroyEntity(playerID);

// Vider complètement le registre
registry.clear();
```

**Requêtes multi-composants :**

```cpp
// Trouver toutes les entités avec Transform ET Sprite ET Animation
auto animated = registry.getEntitiesWithComponents<
    TransformComponent,
    SpriteComponent,
    AnimationComponent
>();

for (auto* entity : animated) {
    auto* anim = entity->getComponent<AnimationComponent>();
    LOG_INFO("Animated entity: {} (frame {})", entity->getID(), anim->currentFrame);
}
```

**Gestion automatique de la mémoire :**

Le registre utilise `std::unique_ptr<Entity>`, donc :

- **Création** : `createEntity()` alloue et retourne un pointeur brut (ownership reste au registre)
- **Destruction** : `destroyEntity()` libère automatiquement la mémoire
- **Clear** : `clear()` détruit toutes les entités proprement

**IDs uniques :**

Chaque entité créée reçoit un ID unique auto-incrémenté :

```cpp
Entity* e1 = registry.createEntity();  // ID = 1
Entity* e2 = registry.createEntity();  // ID = 2
Entity* e3 = registry.createEntity();  // ID = 3
```

Vous pouvez aussi spécifier l'ID manuellement (utile pour chargement depuis JSON) :

```cpp
Entity* loaded = registry.createEntityWithID(42);
```

---

## 22. DefinitionManager - Système à Deux Niveaux

Le `DefinitionManager` implémente l'architecture **deux niveaux** de NovaEngine :

- **Niveau 1 (Definitions)** : Fichiers JSON décrivant les types d'entités (chargés **une fois** au démarrage)
- **Niveau 2 (Instances)** : Fichiers de scène référençant les définitions par ID (chargés à la demande)

**Problème résolu :**

Sans ce système, chaque scène contiendrait toutes les données de texture, animation, sons, etc. pour chaque entité. Avec 100 ennemis identiques, vous dupliqueriez 100 fois les mêmes données. Le DefinitionManager permet de **référencer** une définition commune.

**Fichiers de définitions (assets/data/definitions/) :**

```
definitions/
  ├── Sprites.json      # Définitions de tous les sprites
  ├── Lights.json       # Définitions des lumières
  ├── Sounds.json       # Définitions des sons
  ├── Animations.json   # Définitions des animations
  └── NPCs.json         # Définitions des NPCs
```

**Exemple Sprites.json :**

```json
{
  "sprites": [
    {
      "id": "player_idle",
      "texture": "assets/sprites/player.png",
      "size": [64, 64],
      "textureRect": [0, 0, 64, 64],
      "origin": [32, 32]
    },
    {
      "id": "enemy_goblin",
      "texture": "assets/sprites/enemies.png",
      "size": [48, 48],
      "textureRect": [0, 48, 48, 48]
    }
  ]
}
```

**Fichier de scène utilisant les définitions :**

```json
{
  "name": "Level1",
  "entities": [
    {
      "id": 1,
      "components": {
        "Transform": {"position": [100, 200]},
        "Sprite": {
          "definitionID": "player_idle"  // Référence la définition!
        }
      }
    },
    {
      "id": 2,
      "components": {
        "Transform": {"position": [300, 200]},
        "Sprite": {
          "definitionID": "enemy_goblin"
        }
      }
    }
  ]
}
```

**Interface DefinitionManager :**

```cpp
class DefinitionManager {
public:
    // Charger toutes les définitions
    bool loadDefinitions(const std::string& definitionsPath);

    // Créer un composant depuis une définition
    std::unique_ptr<SpriteComponent> createSpriteFromDefinition(const std::string& defID);
    std::unique_ptr<LightComponent> createLightFromDefinition(const std::string& defID);
    std::unique_ptr<AnimationComponent> createAnimationFromDefinition(const std::string& defID);
    std::unique_ptr<AudioComponent> createAudioFromDefinition(const std::string& defID);

    // Vérifier existence
    bool hasSpriteDefinition(const std::string& id) const;
    bool hasLightDefinition(const std::string& id) const;
    // ...
};
```

**Utilisation dans SceneManager :**

```cpp
// Au démarrage (une seule fois)
DefinitionManager defMgr;
defMgr.loadDefinitions("assets/data/definitions/");

// Lors du chargement d'une scène
Entity* player = registry.createEntity();

// Le JSON de la scène contient: "Sprite": {"definitionID": "player_idle"}
auto* sprite = defMgr.createSpriteFromDefinition("player_idle");
player->addComponent(std::unique_ptr<SpriteComponent>(sprite));
```

**Avantages :**

- ✅ **Mémoire** : Les textures/sons ne sont chargés qu'une fois
- ✅ **Maintenance** : Modifier une définition met à jour toutes les instances
- ✅ **Fichiers scène légers** : Seulement des IDs et positions
- ✅ **Rechargement à chaud** : Modifier `Sprites.json` et recharger sans recompiler

---

## PARTIE IV : GESTION DE SCÈNES

Les scènes sont des **conteneurs logiques** pour grouper des entités et systèmes. Une scène peut représenter un niveau, une pièce, un menu, etc.

## 23. Scene - Conteneur d'Entités et Systèmes

Une `Scene` contient un `EntityRegistry` et une liste de `Systems`. Elle orchestre la mise à jour et le rendu de son contenu.

**Structure (Scene.hpp lignes 15-100) :**

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
    explicit Scene(const std::string& name);

    // Getters
    const std::string& getName() const;
    const std::string& getType() const;
    const Color& getBackgroundColor() const;

    // Chargement
    bool loadFromJSON(const nlohmann::json& sceneData,
                     const DefinitionManager& defManager);

    // Mise à jour
    void update(float deltaTime);
    void render();

    // Accès au registre
    EntityRegistry& getEntityRegistry();
    const EntityRegistry& getEntityRegistry() const;

    // Waypoints
    const WaypointGraph& getWaypointGraph() const;
};
```

**Initialisation par défaut (lignes 38-49) :**

```cpp
Scene::Scene(const std::string& name) : m_name(name) {
    // Créer les systèmes par défaut dans l'ordre optimal
    m_systems.push_back(std::make_unique<AnimationSystem>());
    m_systems.push_back(std::make_unique<PhysicsSystem>());
    m_systems.push_back(std::make_unique<ActivatorSystem>());
    m_systems.push_back(std::make_unique<AudioSystem>());
    m_systems.push_back(std::make_unique<LightSystem>());
    m_systems.push_back(std::make_unique<RenderSystem>());  // DERNIER

    LOG_DEBUG("Created scene: {}", m_name);
}
```

**Méthode update() :**

```cpp
void Scene::update(float deltaTime) {
    // Mettre à jour tous les systèmes dans l'ordre
    for (auto& system : m_systems) {
        system->update(deltaTime, m_entityRegistry);
    }
}
```

**Méthode render() :**

```cpp
void Scene::render() {
    // Clear avec la couleur de fond
    WINDOW().clear(m_backgroundColor);

    // Les systèmes qui dessinent (LightSystem, RenderSystem) ont déjà
    // fait leur rendu pendant update()
}
```

**Chargement depuis JSON :**

La scène charge sa configuration (nom, type, couleur fond, waypoints) puis instancie toutes les entités décrites :

```cpp
bool Scene::loadFromJSON(const nlohmann::json& sceneData,
                        const DefinitionManager& defManager) {
    // 1. Charger métadonnées
    m_name = sceneData.value("name", m_name);
    m_type = sceneData.value("type", "interior");

    if (sceneData.contains("backgroundColor")) {
        auto& bg = sceneData["backgroundColor"];
        m_backgroundColor = Color(bg[0], bg[1], bg[2], bg[3]);
    }

    // 2. Charger waypoint graph
    if (sceneData.contains("pathfinding")) {
        m_waypointGraph.loadFromJSON(sceneData["pathfinding"]);
    }

    // 3. Instancier les entités
    if (sceneData.contains("entities")) {
        for (const auto& entData : sceneData["entities"]) {
            u64 id = entData["id"];
            Entity* entity = m_entityRegistry.createEntityWithID(id);

            // Créer chaque composant à partir des définitions
            if (entData.contains("components")) {
                auto& comps = entData["components"];

                if (comps.contains("Transform")) {
                    auto* t = entity->addComponent(std::make_unique<TransformComponent>());
                    t->deserialize(comps["Transform"]);
                }

                if (comps.contains("Sprite")) {
                    std::string defID = comps["Sprite"]["definitionID"];
                    auto sprite = defManager.createSpriteFromDefinition(defID);
                    entity->addComponent(std::move(sprite));
                }

                // ... autres composants ...
            }
        }
    }

    return true;
}
```

**Utilisation typique :**

```cpp
// Créer une scène
Scene scene("Level1");

// Ajouter des entités manuellement
Entity* player = scene.getEntityRegistry().createEntity();
player->addComponent(std::make_unique<TransformComponent>());
player->addComponent(std::make_unique<SpriteComponent>());

// Ou charger depuis JSON
DefinitionManager defMgr;
defMgr.loadDefinitions("assets/data/definitions/");

std::ifstream file("assets/data/scenes/level1.json");
nlohmann::json sceneData;
file >> sceneData;

scene.loadFromJSON(sceneData, defMgr);

// Boucle de jeu
while (running) {
    float deltaTime = calculateDeltaTime();
    scene.update(deltaTime);
    scene.render();
}
```

---

## 24. SceneManager - Gestion Multi-Scènes

Le `SceneManager` gère le **chargement, déchargement, et commutation** entre plusieurs scènes. Il implémente aussi la mise à jour de scènes inactives pour les NPCs qui voyagent.

**Interface (SceneManager.hpp lignes 15-60) :**

```cpp
class SceneManager {
private:
    DefinitionManager m_definitionManager;
    SceneGraph m_sceneGraph;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::unordered_set<std::string> m_activeScenesForUpdate;  // Scènes actives en arrière-plan

public:
    // Initialisation (charge les définitions une fois)
    bool initialize(const std::string& definitionsPath,
                   const std::string& sceneGraphPath);

    // Chargement de scènes
    bool loadScene(const std::string& scenePath, const std::string& sceneName);
    bool unloadScene(const std::string& sceneName);

    // Gestion scène active (celle où est le joueur)
    bool setActiveScene(const std::string& sceneName);
    Scene* getActiveScene();
    const Scene* getActiveScene() const;

    // Accès par nom
    Scene* getScene(const std::string& sceneName);
    const Scene* getScene(const std::string& sceneName) const;

    // Mise à jour
    void update(float deltaTime);

    // Accès aux sous-systèmes
    DefinitionManager& getDefinitionManager();
    SceneGraph& getSceneGraph();
};
```

**Initialisation au démarrage :**

```cpp
SceneManager sceneMgr;

// Charger les définitions d'entités (une seule fois)
// + le graphe de scènes pour navigation multi-scène
if (!sceneMgr.initialize("assets/data/definitions/",
                         "assets/data/scenegraph.json")) {
    LOG_FATAL("Failed to initialize SceneManager");
    return -1;
}
```

**Charger et activer une scène :**

```cpp
// Charger la scène de la ville
sceneMgr.loadScene("assets/data/scenes/ville.json", "ville");

// Charger les intérieurs
sceneMgr.loadScene("assets/data/scenes/taverne.json", "taverne");
sceneMgr.loadScene("assets/data/scenes/magasin.json", "magasin");

// Définir la scène active (celle où est le joueur)
sceneMgr.setActiveScene("ville");
```

**Mise à jour :**

Le SceneManager met à jour :

1. **Scène active** (toujours)
2. **Scènes avec NPCs en transit** (pour qu'ils continuent de bouger même hors écran)

```cpp
void SceneManager::update(float deltaTime) {
    // Mettre à jour la scène active
    if (m_activeScene) {
        m_activeScene->update(deltaTime);
    }

    // Mettre à jour les scènes avec NPCs en transit
    for (const std::string& sceneName : m_activeScenesForUpdate) {
        if (sceneName == m_activeScene->getName()) continue;  // Déjà mise à jour

        Scene* scene = getScene(sceneName);
        if (scene) {
            scene->update(deltaTime);
        }
    }
}
```

**Changement de scène (transition joueur) :**

```cpp
void playerEntersDoor(const std::string& targetScene, const Vec2f& spawnPos) {
    // Charger la scène cible si pas déjà chargée
    if (!sceneMgr.getScene(targetScene)) {
        std::string path = "assets/data/scenes/" + targetScene + ".json";
        sceneMgr.loadScene(path, targetScene);
    }

    // Changer de scène active
    sceneMgr.setActiveScene(targetScene);

    // Téléporter le joueur
    Scene* scene = sceneMgr.getActiveScene();
    Entity* player = scene->getEntityRegistry().getEntity(PLAYER_ID);
    if (player) {
        auto* transform = player->getComponent<TransformComponent>();
        transform->position = spawnPos;
    }
}
```

**Déchargement (libérer mémoire) :**

```cpp
// Quitter un donjon, libérer ses ressources
sceneMgr.unloadScene("donjon_level5");
```

---

## 25. SceneGraph - Navigation Multi-Scène

Le `SceneGraph` stocke les **connexions entre scènes** et calcule les **routes** pour les NPCs qui voyagent entre scènes. C'est essentiel pour le système de journeys.

**Structure (SceneGraph.hpp) :**

```cpp
struct SceneConnection {
    std::string from;
    std::string to;
    Vec2f exitPortal;   // Position de sortie dans la scène 'from'
    Vec2f entryPortal;  // Position d'entrée dans la scène 'to'
};

class SceneGraph {
private:
    std::vector<SceneConnection> m_connections;

public:
    bool loadFromJSON(const std::string& filepath);
    bool loadFromJSON(const nlohmann::json& json);

    // Trouver le chemin entre deux scènes (BFS)
    std::vector<std::string> findPath(const std::string& start,
                                     const std::string& end) const;

    // Obtenir la connexion entre deux scènes adjacentes
    const SceneConnection* getConnection(const std::string& from,
                                        const std::string& to) const;
};
```

**Fichier scenegraph.json :**

```json
{
  "connections": [
    {
      "from": "ville",
      "to": "taverne",
      "exitPortal": [800, 300],
      "entryPortal": [400, 580]
    },
    {
      "from": "ville",
      "to": "magasin",
      "exitPortal": [1200, 500],
      "entryPortal": [200, 580]
    },
    {
      "from": "ville",
      "to": "maison_bob",
      "exitPortal": [250, 450],
      "entryPortal": [400, 580]
    }
  ]
}
```

**Algorithme de pathfinding (BFS) :**

```cpp
std::vector<std::string> SceneGraph::findPath(const std::string& start,
                                              const std::string& end) const {
    if (start == end) return {start};

    std::queue<std::string> queue;
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_set<std::string> visited;

    queue.push(start);
    visited.insert(start);

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();

        if (current == end) {
            // Reconstruire le chemin
            std::vector<std::string> path;
            std::string node = end;
            while (node != start) {
                path.push_back(node);
                node = cameFrom[node];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        // Explorer les voisins
        for (const auto& conn : m_connections) {
            std::string neighbor;
            if (conn.from == current) {
                neighbor = conn.to;
            } else if (conn.to == current) {
                neighbor = conn.from;
            } else {
                continue;
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

**Utilisation pour NPCs :**

```cpp
// Bob veut aller de sa maison au magasin
std::vector<std::string> route = sceneGraph.findPath("maison_bob", "magasin");
// Résultat: ["maison_bob", "ville", "magasin"]

LOG_INFO("Bob's route: {} -> {} -> {}", route[0], route[1], route[2]);

// Bob devra traverser la ville pour aller au magasin!
```

---

## 26. WaypointGraph - Pathfinding Intérieur de Scène

Le `WaypointGraph` permet de définir des **waypoints fixes** dans une scène et de calculer des chemins entre eux. Les NPCs utilisent ce système pour naviguer intelligemment avec des routes personnalisées.

**Pourquoi des waypoints ?**

Dans une ville complexe, un NPC doit éviter les bâtiments, suivre les routes, et parfois prendre des raccourcis. Au lieu d'implémenter un A* complet sur une grille, on place des waypoints manuellement et on les connecte.

**Structure (WaypointGraph.hpp lignes 17-47) :**

```cpp
struct Waypoint {
    std::string id;              // "fountain", "north_plaza", etc.
    Vec2f position;              // Position dans la scène
    std::vector<std::string> tags; // ["main_road"], ["shortcut"], ["scenic"]
};

struct WaypointConnection {
    std::string from;
    std::string to;
    f32 cost;                    // Distance ou temps
    std::vector<std::string> tags;
    bool bidirectional;
};

class WaypointGraph {
public:
    bool loadFromJSON(const nlohmann::json& json);

    const Waypoint* findNearestWaypoint(const Vec2f& position,
                                       f32 maxDistance = -1.0f) const;

    const Waypoint* findWaypointByID(const std::string& id) const;

    // Trouver le chemin entre deux positions
    std::vector<Vec2f> findPath(const Vec2f& startPos, const Vec2f& endPos,
                               const std::vector<std::string>& preferredTags = {}) const;

    // Trouver le chemin entre deux waypoints
    std::vector<std::string> findPathByID(const std::string& startID, const std::string& endID,
                                         const std::vector<std::string>& preferredTags = {}) const;
};
```

**Fichier ville.json avec waypoints :**

```json
{
  "name": "Ville",
  "type": "exterior",
  "backgroundColor": [50, 50, 70, 255],

  "pathfinding": {
    "waypoints": [
      {"id": "fountain", "position": [640, 360], "tags": ["landmark", "center"]},
      {"id": "north_plaza", "position": [640, 200], "tags": ["main_road"]},
      {"id": "tavern_door", "position": [800, 300], "tags": ["main_road"]},
      {"id": "shortcut_ne", "position": [850, 250], "tags": ["shortcut"]}
    ],
    "connections": [
      {
        "from": "fountain",
        "to": "north_plaza",
        "tags": ["main_road"],
        "bidirectional": true
      },
      {
        "from": "fountain",
        "to": "shortcut_ne",
        "tags": ["shortcut"],
        "bidirectional": true
      },
      {
        "from": "north_plaza",
        "to": "tavern_door",
        "tags": ["main_road"],
        "bidirectional": true
      }
    ]
  }
}
```

**Pathfinding avec tags de personnalité :**

```cpp
// Alice (garde) préfère les routes principales
std::vector<std::string> alicePrefs = {"main_road"};
auto alicePath = waypointGraph.findPath(aliceStart, aliceEnd, alicePrefs);
// Résultat: fountain -> north_plaza -> tavern_door (suit main_road)

// Bob (marchand) préfère les raccourcis
std::vector<std::string> bobPrefs = {"shortcut"};
auto bobPath = waypointGraph.findPath(bobStart, bobEnd, bobPrefs);
// Résultat: fountain -> shortcut_ne -> tavern_door (prend le raccourci!)
```

**Algorithme (BFS avec filtrage de tags) :**

```cpp
std::vector<std::string> WaypointGraph::findPathByID(
    const std::string& startID, const std::string& endID,
    const std::vector<std::string>& preferredTags) const {

    if (startID == endID) return {startID};

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

        for (const auto& conn : m_connections) {
            // Vérifier si cette connexion nous concerne
            std::string neighbor;
            if (conn.from == current) {
                neighbor = conn.to;
            } else if (conn.bidirectional && conn.to == current) {
                neighbor = conn.from;
            } else {
                continue;
            }

            // Filtrer par tags préférés
            if (!preferredTags.empty() && !hasPreferredTag(conn, preferredTags)) {
                continue;  // Cette connexion n'a pas les bons tags
            }

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    LOG_WARN("No waypoint path found from '{}' to '{}'", startID, endID);
    return {};
}
```

---

## 27. JourneySystem - Voyages Multi-Scènes des NPCs

Le `JourneySystem` permet aux NPCs de voyager entre scènes de manière **physique** (pas de téléportation). Quand un NPC doit aller de la scène A à la scène C en passant par B, il traverse réellement B et est visible par le joueur.

**Composants requis :**

1. **JourneyComponent** : Stocke l'itinéraire du NPC
2. **TransformComponent** : Position actuelle
3. **SceneTransitionComponent** : Gestion des transitions entre scènes

**JourneyComponent (Components.hpp) :**

```cpp
class JourneyComponent : public Component {
public:
    std::vector<std::string> scenePath;   // ["maison_bob", "ville", "magasin"]
    int currentSceneIndex = 0;            // Position dans scenePath

    std::string targetScene;              // Scène de destination dans scenePath
    Vec2f targetPosition;                 // Position finale dans targetScene

    std::vector<Vec2f> localWaypointPath; // Waypoints dans la scène actuelle
    int currentLocalWaypointIndex = 0;

    std::vector<std::string> preferredPathTags;  // ["shortcut"], ["main_road"], etc.

    bool isActive = false;
    bool hasReachedDestination = false;

    COMPONENT_TYPE_ID(JourneyComponent)
    // ... serialize/deserialize
};
```

**Logique du système (Systems.hpp - JourneySystem::update) :**

```cpp
void JourneySystem::update(float deltaTime, EntityRegistry& registry) {
    for (auto* entity : registry.getEntities()) {
        auto* journey = entity->getComponent<JourneyComponent>();
        auto* transform = entity->getComponent<TransformComponent>();

        if (!journey || !transform || !journey->isActive) continue;

        // 1. Si pas de waypoints locaux, en calculer
        if (journey->localWaypointPath.empty()) {
            calculateLocalWaypointPath(entity, scene);
        }

        // 2. Se déplacer vers le prochain waypoint local
        if (journey->currentLocalWaypointIndex < journey->localWaypointPath.size()) {
            Vec2f target = journey->localWaypointPath[journey->currentLocalWaypointIndex];
            Vec2f direction = target - transform->position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance < 5.0f) {
                // Waypoint atteint, passer au suivant
                journey->currentLocalWaypointIndex++;
            } else {
                // Se déplacer vers le waypoint
                direction.x /= distance;
                direction.y /= distance;
                transform->position += direction * walkSpeed * deltaTime;
            }
        } else {
            // Tous les waypoints locaux atteints
            // -> Passer à la scène suivante du journey
            journey->currentSceneIndex++;

            if (journey->currentSceneIndex >= journey->scenePath.size()) {
                // Arrivé à destination finale!
                journey->hasReachedDestination = true;
                journey->isActive = false;
                LOG_INFO("NPC {} reached final destination", entity->getID());
            } else {
                // Transition vers la prochaine scène
                auto* transition = entity->getComponent<SceneTransitionComponent>();
                if (transition) {
                    transition->targetScene = journey->scenePath[journey->currentSceneIndex];
                    transition->isTransitioning = true;
                    // SceneManager déplacera le NPC vers la nouvelle scène
                }
            }
        }
    }
}
```

**Calcul de waypoints locaux :**

```cpp
void calculateLocalWaypointPath(Entity* entity, Scene* scene) {
    auto* journey = entity->getComponent<JourneyComponent>();
    auto* transform = entity->getComponent<TransformComponent>();

    const WaypointGraph& graph = scene->getWaypointGraph();

    // Trouver le chemin de la position actuelle à la cible dans cette scène
    std::vector<Vec2f> path = graph.findPath(
        transform->position,
        journey->targetPosition,
        journey->preferredPathTags  // Personnalité!
    );

    journey->localWaypointPath = path;
    journey->currentLocalWaypointIndex = 0;
}
```

**Exemple concret : Bob va au magasin :**

```json
{
  "id": "merchant_bob",
  "name": "Bob le Marchand",
  "preferredPathTags": ["shortcut"],
  "dailySchedule": [
    {
      "startTime": 7.0,
      "activity": "travel",
      "targetScene": "magasin",
      "targetPosition": [300, 250]
    }
  ]
}
```

Au démarrage du voyage :

1. SceneGraph calcule scenePath: `["maison_bob", "ville", "magasin"]`
2. Bob commence dans `maison_bob`, se dirige vers la sortie
3. Il entre dans `ville`, WaypointGraph calcule le chemin avec tag `"shortcut"`
4. Bob traverse la ville en empruntant les raccourcis
5. Il arrive à la porte du magasin dans `ville`, transition vers `magasin`
6. Il arrive à sa position finale `[300, 250]` dans `magasin`

**Si le joueur attend dans ville**, il verra Bob traverser la scène!

---

## PARTIE V : GUIDES PRATIQUES

## 28. Créer Votre Premier Jeu Complet

Suivez ce guide pas à pas pour créer un petit RPG avec NovaEngine.

**Étape 1 : Structure des dossiers**

```
MonJeu/
├── CMakeLists.txt
├── src/
│   └── Main.cpp
└── assets/
    ├── data/
    │   ├── definitions/
    │   │   ├── Sprites.json
    │   │   └── NPCs.json
    │   └── scenes/
    │       ├── village.json
    │       └── taverne.json
    ├── sprites/
    │   ├── player.png
    │   └── npc.png
    └── fonts/
        └── arial.ttf
```

**Étape 2 : Main.cpp**

```cpp
#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>

using namespace NovaEngine;

class MyRPG : public Application {
private:
    SceneManager m_sceneMgr;
    u64 m_playerID = 0;

public:
    MyRPG() {
        m_config.windowTitle = "Mon RPG";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vSync = true;
        m_config.clearColor = Color(30, 30, 40);
    }

    bool onInitialize() override {
        LOG_INFO("Initializing RPG...");

        // Charger les définitions d'entités
        if (!m_sceneMgr.initialize("assets/data/definitions/",
                                   "assets/data/scenegraph.json")) {
            LOG_ERROR("Failed to initialize SceneManager");
            return false;
        }

        // Charger la scène de départ
        if (!m_sceneMgr.loadScene("assets/data/scenes/village.json", "village")) {
            LOG_ERROR("Failed to load village scene");
            return false;
        }

        m_sceneMgr.setActiveScene("village");

        // Créer le joueur
        Scene* village = m_sceneMgr.getActiveScene();
        Entity* player = village->getEntityRegistry().createEntity();
        m_playerID = player->getID();

        auto* transform = player->addComponent(std::make_unique<TransformComponent>());
        transform->position = Vec2f(640, 360);

        auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
        sprite->textureHandle = RESOURCES().loadTexture("assets/sprites/player.png");
        sprite->size = Vec2f(64, 64);

        LOG_INFO("RPG initialized successfully");
        return true;
    }

    void onUpdate(float deltaTime) override {
        // Mise à jour des scènes (NPCs, animations, etc.)
        m_sceneMgr.update(deltaTime);

        // Contrôles du joueur
        Scene* scene = m_sceneMgr.getActiveScene();
        Entity* player = scene->getEntityRegistry().getEntity(m_playerID);
        if (!player) return;

        auto* transform = player->getComponent<TransformComponent>();
        if (!transform) return;

        Vec2f movement(0, 0);
        const float speed = 200.0f;

        if (INPUT().isKeyPressed(KeyCode::W)) movement.y -= 1;
        if (INPUT().isKeyPressed(KeyCode::S)) movement.y += 1;
        if (INPUT().isKeyPressed(KeyCode::A)) movement.x -= 1;
        if (INPUT().isKeyPressed(KeyCode::D)) movement.x += 1;

        if (movement.x != 0 || movement.y != 0) {
            float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            movement /= len;
            transform->position += movement * speed * deltaTime;
        }

        // Caméra suit le joueur
        VIEWPORT().setViewCenter(transform->position);
    }

    void onRender() override {
        // Les systems de la scène dessinent automatiquement
    }

    void onShutdown() override {
        LOG_INFO("RPG shutting down");
    }
};

int main() {
    MyRPG game;
    return game.run();
}
```

**Étape 3 : Sprites.json**

```json
{
  "sprites": [
    {
      "id": "player_idle",
      "texture": "assets/sprites/player.png",
      "size": [64, 64]
    },
    {
      "id": "npc_villager",
      "texture": "assets/sprites/npc.png",
      "size": [64, 64]
    }
  ]
}
```

**Étape 4 : village.json**

```json
{
  "name": "Village",
  "type": "exterior",
  "backgroundColor": [50, 100, 50, 255],

  "entities": [
    {
      "id": 100,
      "components": {
        "Transform": {"position": [400, 300]},
        "Sprite": {"definitionID": "npc_villager"}
      }
    }
  ]
}
```

**Compiler et lancer :**

```bash
mkdir build && cd build
cmake ..
make
./MonJeu
```

Vous devriez voir votre joueur contrôlable et un NPC dans le village!

---

## 29. Formats JSON du Moteur

### Format Sprite Definition

```json
{
  "id": "unique_id",
  "texture": "path/to/texture.png",
  "size": [width, height],
  "textureRect": [left, top, width, height],  // Optionnel, pour sprite sheets
  "origin": [x, y],  // Optionnel, point de rotation/échelle
  "zOffset": 10.0    // Optionnel, pour draw order
}
```

### Format Animation Definition

```json
{
  "id": "walk_cycle",
  "texture": "spritesheet.png",
  "frameSize": [64, 64],
  "frames": [
    {"rect": [0, 0, 64, 64], "duration": 0.1},
    {"rect": [64, 0, 64, 64], "duration": 0.1},
    {"rect": [128, 0, 64, 64], "duration": 0.1}
  ],
  "loop": true
}
```

### Format Scene

```json
{
  "name": "SceneName",
  "type": "interior",  // ou "exterior"
  "backgroundColor": [R, G, B, A],

  "pathfinding": {
    "waypoints": [
      {"id": "wp1", "position": [x, y], "tags": ["tag1", "tag2"]}
    ],
    "connections": [
      {"from": "wp1", "to": "wp2", "tags": ["road"], "bidirectional": true}
    ]
  },

  "entities": [
    {
      "id": 1,
      "components": {
        "Transform": {
          "position": [x, y],
          "rotation": 0.0,
          "scale": [1.0, 1.0]
        },
        "Sprite": {
          "definitionID": "sprite_def_id"
        }
      }
    }
  ]
}
```

### Format SceneGraph

```json
{
  "connections": [
    {
      "from": "scene1",
      "to": "scene2",
      "exitPortal": [x, y],
      "entryPortal": [x, y]
    }
  ]
}
```

---

## 30. Workflow de Développement

**1. Initialisation projet**

```bash
git clone https://github.com/votre-repo/NovaEngine.git
cd MonProjet
mkdir build && cd build
cmake ..
make
```

**2. Créer assets**

- Dessinez sprites (64x64 recommandé pour personnages)
- Organisez en sprite sheets si possible
- Exportez en PNG avec transparence

**3. Créer définitions**

```bash
cd assets/data/definitions
nano Sprites.json  # Ajouter vos sprites
```

**4. Créer scènes**

```bash
cd ../scenes
nano level1.json  # Placer vos entités
```

**5. Tester**

```bash
./MonJeu
# Observer logs pour erreurs de chargement
```

**6. Déboguer**

- Activer `showDebugInfo` dans config
- Utiliser `LOG_DEBUG` abondamment
- Vérifier les warnings du logger

**7. Optimiser**

- Grouper sprites en texture atlases
- Limiter entités actives par scène (< 1000)
- Utiliser `unloadScene` pour libérer mémoire

---

## 31. Debugging et Résolution de Problèmes

### Problème : Texture ne s'affiche pas

**Symptômes** : Sprite invisible ou carré blanc

**Causes possibles** :

1. Chemin de fichier incorrect
2. Texture non chargée
3. TextureRect invalide

**Solutions** :

```cpp
TextureHandle tex = RESOURCES().loadTexture("assets/player.png");
if (tex == INVALID_HANDLE) {
    LOG_ERROR("Failed to load texture");  // Vérifier logs!
}

// Vérifier dimensions
Vec2u size = RESOURCES().getTextureSize(tex);
LOG_INFO("Texture size: {}x{}", size.x, size.y);

// S'assurer que textureRect est dans les limites
sprite->textureRect = IntRect(0, 0, size.x, size.y);
```

### Problème : Entité ne se met pas à jour

**Symptômes** : Entité ne bouge pas, animations figées

**Causes** :

1. Composants manquants
2. Système non ajouté à la scène
3. Scene::update() pas appelé

**Solutions** :

```cpp
// Vérifier composants
if (!entity->hasComponent<TransformComponent>()) {
    LOG_WARN("Entity {} missing TransformComponent", entity->getID());
}

// Vérifier systems
for (const auto& system : scene->getSystems()) {
    LOG_DEBUG("System: {}", typeid(*system).name());
}

// S'assurer que update est appelé
void onUpdate(float deltaTime) override {
    m_sceneMgr.update(deltaTime);  // CRUCIAL!
}
```

### Problème : Crash au chargement JSON

**Symptômes** : Exception ou crash à `loadScene()`

**Solutions** :

```cpp
try {
    if (!sceneMgr.loadScene("scene.json", "scene")) {
        LOG_ERROR("Scene load failed");
    }
} catch (const nlohmann::json::exception& e) {
    LOG_FATAL("JSON error: {}", e.what());
}

// Valider JSON en ligne : https://jsonlint.com
```

### Problème : FPS bas

**Causes** :

1. Trop d'entités
2. Tri inefficace dans RenderSystem
3. VSync désactivé avec limite FPS absente

**Solutions** :

```cpp
// Limiter FPS
m_config.frameRateLimit = 60;
m_config.vSync = true;

// Compter entités
LOG_INFO("Entity count: {}", registry.getEntityCount());

// Optimiser RenderSystem : ne pas trier chaque frame si statique
```

---

## 32. Extensions et Personnalisation

### Ajouter un nouveau composant

1. **Déclarer dans Components.hpp**

```cpp
class VelocityComponent : public Component {
public:
    Vec2f velocity = Vec2f(0, 0);
    f32 maxSpeed = 100.0f;

    COMPONENT_TYPE_ID(VelocityComponent)

    void serialize(nlohmann::json& json) const override {
        json["velocity"] = {velocity.x, velocity.y};
        json["maxSpeed"] = maxSpeed;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("velocity")) {
            auto& v = json["velocity"];
            velocity = Vec2f(v[0], v[1]);
        }
        maxSpeed = json.value("maxSpeed", 100.0f);
    }
};
```

2. **Créer un système pour l'utiliser**

```cpp
class MovementSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        for (auto* entity : registry.getEntities()) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* velocity = entity->getComponent<VelocityComponent>();

            if (!transform || !velocity) continue;

            transform->position += velocity->velocity * deltaTime;
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "VelocityComponent"};
    }
};
```

3. **Ajouter le système à la scène**

```cpp
scene->addSystem(std::make_unique<MovementSystem>());
```

### Créer un nouveau Backend

Si vous voulez utiliser SDL au lieu de SFML :

1. Créer `SDLGraphicsBackend.hpp` implémentant `IGraphicsBackend`
2. Implémenter toutes les méthodes virtuelles
3. Modifier `BackendManager::createBackends()` pour instancier SDL

```cpp
bool BackendManager::createBackends(BackendType type) {
    switch (type) {
    case BackendType::SDL:
        m_graphics = std::make_unique<SDLGraphicsBackend>();
        m_window = std::make_unique<SDLWindowBackend>();
        // ...
        break;
    // ...
    }
}
```

---

## 33. Performance et Optimisation

### Cache-Friendly ECS

Les composants sont stockés éparpillés (map dans Entity). Pour haute performance :

```cpp
// Grouper les composants de même type
std::vector<TransformComponent*> transforms;
for (auto* entity : registry.getEntities()) {
    if (auto* t = entity->getComponent<TransformComponent>()) {
        transforms.push_back(t);
    }
}

// Itérer sur le vecteur (meilleur cache locality)
for (auto* t : transforms) {
    t->position.x += 10;
}
```

### Limiter allocations

```cpp
// MAUVAIS : Alloc chaque frame
void update(float dt, EntityRegistry& reg) {
    std::vector<Entity*> entities = reg.getEntities();  // Nouvelle alloc!
    for (auto* e : entities) { /*...*/ }
}

// BON : Réutiliser buffer
class MySystem {
    std::vector<Entity*> m_cache;
public:
    void update(float dt, EntityRegistry& reg) {
        m_cache = reg.getEntities();  // Réutilise capacité
        for (auto* e : m_cache) { /*...*/ }
    }
};
```

### Culling (ne dessiner que le visible)

```cpp
Rect viewBounds = getViewBounds();

for (auto* entity : renderables) {
    auto* transform = entity->getComponent<TransformComponent>();
    auto* sprite = entity->getComponent<SpriteComponent>();

    Rect entityBounds(transform->position.x, transform->position.y,
                     sprite->size.x, sprite->size.y);

    if (!viewBounds.intersects(entityBounds)) {
        continue;  // Skip dessin si hors écran
    }

    GRAPHICS().drawSprite(...);
}
```

---

## 34. Ressources et Communauté

### Documentation Externe

- **SFML** : https://www.sfml-dev.org/documentation/
- **nlohmann/json** : https://json.nlohmann.me/
- **C++ Reference** : https://en.cppreference.com/

### Tutoriels Recommandés

- **ECS Architecture** : "Overwatch Gameplay Architecture" (GDC Talk)
- **2D Game Math** : "Math for Game Programmers" série
- **JSON in C++** : Documentation nlohmann/json

### Exemples de Jeux

Voir le dossier `examples/` du repo NovaEngine :

- `examples/platformer/` - Platformer simple
- `examples/rpg_village/` - RPG avec NPCs
- `examples/particle_demo/` - Système de particules

### Contribuer

1. Fork le repo
2. Créer une branche (`git checkout -b feature/ma-feature`)
3. Commit (`git commit -am 'Add amazing feature'`)
4. Push (`git push origin feature/ma-feature`)
5. Ouvrir une Pull Request

---

# CONCLUSION

Vous avez maintenant une compréhension complète de **NovaEngine** :

- **Architecture Backend** : Abstraction multi-plateforme (SFML, SDL, etc.)
- **Système ECS** : Composition flexible, data-oriented design
- **Gestion Scènes** : Multi-scènes avec NPCs vivants en temps réel
- **Pathfinding** : Waypoints avec personnalité, navigation multi-scène
- **Outils** : Logger, ConfigManager, ResourceManager

**Prochaines étapes suggérées** :

1. Suivre le tutorial "Premier Jeu" (section 28)
2. Expérimenter avec les composants built-in
3. Créer vos propres composants et systèmes
4. Construire un petit prototype de jeu

**Bonne création avec NovaEngine!** 🚀