# Système Core - NovaEngine

Le système Core fournit les fondations du moteur : types de base, logging, classe Application, et gestion de configuration.

## Fichiers

```
sdk/include/NovaEngine/Core/
├── Types.hpp          # Types de base (u8-u64, smart pointers)
├── Logger.hpp         # Système de logging
├── Application.hpp    # Classe de base pour applications
├── ConfigManager.hpp  # Chargement configuration INI
└── NovaEngine.hpp     # Header principal (si utilisé standalone)
```

---

## Types.hpp

### Types entiers

```cpp
namespace NovaEngine {
    // Signed integers
    using i8  = int8_t;    // -128 to 127
    using i16 = int16_t;   // -32,768 to 32,767
    using i32 = int32_t;   // -2.1B to 2.1B
    using i64 = int64_t;   // Très grand

    // Unsigned integers
    using u8  = uint8_t;   // 0 to 255
    using u16 = uint16_t;  // 0 to 65,535
    using u32 = uint32_t;  // 0 to 4.2B
    using u64 = uint64_t;  // Très grand

    // Floating point
    using f32 = float;     // 32-bit float
    using f64 = double;    // 64-bit double
}
```

**Pourquoi ces types ?**
- **Taille garantie** : `int` peut être 16 ou 32 bits selon plateforme
- **Clarté du code** : `u32` indique explicitement unsigned 32-bit
- **Interopérabilité** : Compatibilité avec formats binaires, shaders
- **Convention commune** : Utilisé dans beaucoup de moteurs (Unreal, etc.)

### Types de chaînes

```cpp
using String = std::string;
using ID = std::string;  // Pour identifiants (spriteID, sceneID, etc.)
```

### Smart Pointers

```cpp
// Unique ownership (99% des cas)
template <typename T>
using Unique = std::unique_ptr<T>;

// Shared ownership (ressources partagées)
template <typename T>
using Ref = std::shared_ptr<T>;

// Weak reference (observer sans ownership)
template <typename T>
using Weak = std::weak_ptr<T>;
```

**Exemple d'utilisation** :

```cpp
// Création avec ownership unique
auto entity = std::make_unique<Entity>(123);
Unique<Component> component = std::make_unique<TransformComponent>();

// Transfert d'ownership
entity->addComponent(std::move(component));
// component est maintenant nullptr

// Shared ownership (rare)
Ref<Texture> texture = std::make_shared<Texture>();
```

---

## Logger.hpp

### Système de logging multi-niveaux

```cpp
enum class LogLevel {
    Trace,    // Très verbose, debug détaillé
    Debug,    // Information de debug
    Info,     // Information normale
    Warning,  // Avertissements
    Error,    // Erreurs récupérables
    Fatal     // Erreurs fatales (crash)
};
```

### API

```cpp
class Logger {
public:
    static Logger& getInstance();  // Singleton

    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args);
};
```

### Macros pratiques

```cpp
#define LOG_TRACE(...)   Logger::getInstance().log(LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...)   Logger::getInstance().log(LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...)    Logger::getInstance().log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...)    Logger::getInstance().log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   Logger::getInstance().log(LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...)   Logger::getInstance().log(LogLevel::Fatal, __VA_ARGS__)
```

### Utilisation

```cpp
// main.cpp
int main() {
    Logger::getInstance().setLogLevel(LogLevel::Trace);

    LOG_INFO("Application starting...");
    LOG_DEBUG("Loading config from: {}", configPath);

    if (!success) {
        LOG_ERROR("Failed to load texture: {}", path);
    }

    LOG_TRACE("Entity {} moved to ({}, {})", id, x, y);
}
```

### Format de log

```
[TIMESTAMP] [LEVEL] Message
```

Exemple :
```
[2025-11-26 10:30:45] [INFO] Application starting...
[2025-11-26 10:30:45] [DEBUG] Loading config from: config/engine.ini
[2025-11-26 10:30:45] [ERROR] Failed to load texture: assets/missing.png
```

### Paramétrage de verbosité

```cpp
// Production : seulement warnings et erreurs
logger.setLogLevel(LogLevel::Warning);

// Développement : tout sauf trace
logger.setLogLevel(LogLevel::Debug);

// Debug intensif : absolument tout
logger.setLogLevel(LogLevel::Trace);
```

---

## Application.hpp

### Classe de base pour toutes les applications

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

    Application();
    explicit Application(const Config& config);
    virtual ~Application();

    // Point d'entrée principal
    int run();

    // Quitter proprement
    void quit();

    // Accesseurs
    float getDeltaTime() const;
    const Config& getConfig() const;
    bool isInitialized() const;

protected:
    // Hooks à implémenter par sous-classes
    virtual bool onInitialize() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onEvent(const Event& event);
    virtual void onShutdown();

private:
    bool initializeEngine();
    void runMainLoop();
    void processEvents();
    void shutdownEngine();

    Config m_config;
    float m_deltaTime;
    bool m_initialized;
    f32 m_lastTime;
};
```

### Pattern Template Method

`Application::run()` définit le squelette du programme :

```cpp
int Application::run() {
    try {
        LOG_INFO("=== Application Starting ===");

        // 1. Initialiser le moteur (backends)
        if (!initializeEngine()) {
            LOG_FATAL("Failed to initialize NovaEngine");
            return -1;
        }

        // 2. Initialiser l'application spécifique
        if (!onInitialize()) {
            LOG_FATAL("Failed to initialize application");
            return -1;
        }

        m_initialized = true;
        LOG_INFO("=== Application Initialized Successfully ===");

        // 3. Boucle principale
        runMainLoop();

        // 4. Nettoyage
        onShutdown();
        shutdownEngine();

        LOG_INFO("=== Application Exited Successfully ===");
        return 0;

    } catch (const std::exception& e) {
        LOG_FATAL("Exception in Application::run(): {}", e.what());
        return -1;
    }
}
```

### Game Loop implémentée

```cpp
void Application::runMainLoop() {
    LOG_DEBUG("Entering main loop");

    m_lastTime = 0.0f;

    while (WINDOW().isOpen()) {
        // Calculate delta time
        f32 currentTime = m_lastTime + 0.016f;  // ~60fps
        m_deltaTime = currentTime - m_lastTime;
        m_lastTime = currentTime;

        // Process input
        processEvents();

        // Update logic
        onUpdate(m_deltaTime);

        // Render
        WINDOW().clear(m_config.clearColor);
        onRender();
        WINDOW().display();
    }

    LOG_DEBUG("Exited main loop");
}
```

### Traitement des événements

```cpp
void Application::processEvents() {
    InputEvent inputEvent;

    while (INPUT().pollEvent(inputEvent)) {
        // Fermeture fenêtre
        if (inputEvent.type == InputEventType::Closed) {
            quit();
            continue;
        }

        // ESC pour quitter
        if (inputEvent.type == InputEventType::KeyPressed) {
            if (inputEvent.key.code == KeyCode::Escape) {
                LOG_INFO("Escape pressed - quitting application");
                quit();
                continue;
            }
        }

        // Dispatcher aux sous-classes
        Event novaEvent(inputEvent);
        onEvent(novaEvent);
    }
}
```

### Utilisation dans Game

```cpp
class Game : public Application {
public:
    Game() : Application(createConfig()) {
        // Constructeur
    }

protected:
    bool onInitialize() override {
        LOG_INFO("Initializing Game");

        // Charger scènes
        m_sceneManager.initialize("data/definitions/", "data/scenegraph.json");
        m_sceneManager.loadScene("data/scenes/test.json", "test");

        // Setup UI, post-processing, etc.
        // ...

        return true;
    }

    void onUpdate(float deltaTime) override {
        // Update player, NPCs, physics, etc.
        m_playerController->update(deltaTime);
        m_sceneManager.update(deltaTime);
    }

    void onRender() override {
        // Render scene + UI
        m_sceneManager.render();
        m_uiManager.render();
    }

    void onEvent(const Event& event) override {
        // Handle input
        m_uiManager.dispatchEvent(event);

        if (event.inputEvent.type == InputEventType::KeyPressed) {
            // Custom key handling
        }
    }

    void onShutdown() override {
        LOG_INFO("Game shutting down");
        m_sceneManager.shutdown();
    }

private:
    static Config createConfig();

    SceneManager m_sceneManager;
    PlayerController m_playerController;
    UIManager m_uiManager;
};
```

---

## ConfigManager.hpp

### Chargement de configuration INI

```cpp
class ConfigManager {
public:
    struct DisplayConfig {
        u32 width = 1920;
        u32 height = 1080;
        u32 nativeWidth = 1920;   // Résolution logique
        u32 nativeHeight = 1080;
        bool fullscreen = false;
        bool vsync = true;
        u32 frameRateLimit = 60;
    };

    static bool initializeGlobalConfig(const String& configPath);
    static ConfigManager& getInstance();

    const DisplayConfig& getDisplayConfig() const;

private:
    ConfigManager() = default;

    DisplayConfig m_displayConfig;
    bool m_initialized = false;
};
```

### Format de fichier INI

```ini
# config/engine.ini
[Display]
width=1920
height=1080
nativeWidth=1920
nativeHeight=1080
fullscreen=false
vsync=true
frameRateLimit=60
```

### Utilisation

```cpp
// Au démarrage
if (!ConfigManager::initializeGlobalConfig("config/engine.ini")) {
    LOG_WARN("Failed to load config, using defaults");
}

// Accès à la config
const auto& displayConfig = ConfigManager::getInstance().getDisplayConfig();

config.windowWidth = displayConfig.width;
config.windowHeight = displayConfig.height;
config.fullscreen = displayConfig.fullscreen;
config.vSync = displayConfig.vsync;
config.frameRateLimit = displayConfig.frameRateLimit;
```

### Avantages

- **Configuration sans recompilation** : Modifier INI et relancer
- **Valeurs par défaut** : Si fichier manquant, utilise defaults
- **Séparation préoccupations** : Config séparée du code
- **Support multi-environnement** : config_dev.ini, config_prod.ini

---

## NovaEngine.hpp (optionnel)

Header principal si le moteur est utilisé comme bibliothèque standalone.

```cpp
#pragma once

// Core
#include "Core/Types.hpp"
#include "Core/Logger.hpp"
#include "Core/Application.hpp"

// Backend
#include "Backend/BackendManager.hpp"
#include "Backend/Core/BackendTypes.hpp"

// ECS
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "ECS/System.hpp"
#include "ECS/Components.hpp"
#include "ECS/Systems.hpp"
#include "ECS/Scene.hpp"
#include "ECS/SceneManager.hpp"

// Rendering
#include "Rendering/PostProcessPipeline.hpp"
#include "Rendering/Effects/CRTEffect.hpp"
#include "Rendering/Effects/BloomEffect.hpp"
// ... etc

// UI
#include "UI/UIManager.hpp"
#include "UI/Components/Button.hpp"
// ... etc
```

**Utilisation** :

```cpp
// Dans votre application
#include <NovaEngine/NovaEngine.hpp>

// Tout le moteur est accessible
using namespace NovaEngine;
```

---

## Exemples complets

### Application minimale

```cpp
#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/Core/Logger.hpp>

class MinimalApp : public NovaEngine::Application {
protected:
    bool onInitialize() override {
        LOG_INFO("Minimal app initialized");
        return true;
    }

    void onUpdate(float dt) override {
        // Logique
    }

    void onRender() override {
        // Rendu
    }
};

int main() {
    NovaEngine::Logger::getInstance().setLogLevel(NovaEngine::LogLevel::Info);

    MinimalApp app;
    return app.run();
}
```

### Application avec configuration personnalisée

```cpp
class CustomApp : public Application {
public:
    CustomApp() : Application(createCustomConfig()) {}

private:
    static Config createCustomConfig() {
        Config config;
        config.windowTitle = "Mon jeu";
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.fullscreen = false;
        config.frameRateLimit = 144;  // High refresh rate
        config.vSync = false;
        config.clearColor = Color{30, 30, 40};  // Gris foncé
        return config;
    }

protected:
    bool onInitialize() override { /* ... */ }
    void onUpdate(float dt) override { /* ... */ }
    void onRender() override { /* ... */ }
};
```

---

## Bonnes pratiques

### Logging

```cpp
// ✅ BON : Messages informatifs
LOG_INFO("Scene '{}' loaded with {} entities", sceneName, count);

// ❌ MAUVAIS : Trop verbose
LOG_INFO("x");
LOG_INFO("y");
LOG_INFO("z");

// ✅ BON : Utiliser le bon niveau
LOG_ERROR("Critical failure: {}", reason);  // Pour erreurs
LOG_DEBUG("Cache hit for texture '{}'", id);  // Pour debug
LOG_TRACE("Entity {} position: ({}, {})", id, x, y);  // Très verbose
```

### Configuration

```cpp
// ✅ BON : Charger tôt dans main()
int main() {
    ConfigManager::initializeGlobalConfig("config/engine.ini");
    Game game;
    return game.run();
}

// ❌ MAUVAIS : Charger tard
int main() {
    Game game;  // Game construit avant config chargée!
    ConfigManager::initializeGlobalConfig(...);
}
```

### Application lifecycle

```cpp
// ✅ BON : Utiliser les hooks appropriés
class Game : public Application {
    bool onInitialize() override {
        // Initialisation une fois au démarrage
        m_sceneManager.initialize();
        return true;
    }

    void onUpdate(float dt) override {
        // Logique chaque frame
        m_player.update(dt);
    }

    void onShutdown() override {
        // Nettoyage à la fermeture
        m_sceneManager.shutdown();
    }
};

// ❌ MAUVAIS : Initialiser dans update
void onUpdate(float dt) override {
    if (!initialized) {
        m_sceneManager.initialize();  // NON! Fait dans onInitialize()
        initialized = true;
    }
}
```

---

## Diagramme de classe

```
┌────────────────────────────┐
│      Application           │
│  (abstract base class)     │
├────────────────────────────┤
│ + run(): int               │
│ + quit(): void             │
│ # onInitialize(): bool     │◄──── Abstract (=0)
│ # onUpdate(dt): void       │◄──── Abstract (=0)
│ # onRender(): void         │◄──── Abstract (=0)
│ # onEvent(e): void         │
│ # onShutdown(): void       │
│ - runMainLoop(): void      │
│ - processEvents(): void    │
└────────────────────────────┘
              △
              │ inherits
              │
┌────────────────────────────┐
│           Game             │
│  (concrete implementation) │
├────────────────────────────┤
│ # onInitialize(): bool     │◄──── Implémenté
│ # onUpdate(dt): void       │◄──── Implémenté
│ # onRender(): void         │◄──── Implémenté
│ # onEvent(e): void         │◄──── Implémenté
│ # onShutdown(): void       │◄──── Implémenté
└────────────────────────────┘
```

---

**Prochaine section** : [Backend Abstraction](03-BACKEND.md)
