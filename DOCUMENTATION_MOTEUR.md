# Documentation Complète - Moteur NovaEngine

## Table des Matières

1. [Vue d'ensemble du Moteur](#vue-densemble-du-moteur)
2. [Architecture Backend](#architecture-backend)
3. [Core Systems](#core-systems)
4. [Resource Management](#resource-management)
5. [UI System](#ui-system)
6. [Event System](#event-system)
7. [Application & Game](#application--game)
8. [Types de Base](#types-de-base)
9. [Configuration](#configuration)
10. [Guide d'Utilisation Complet](#guide-dutilisation-complet)

---

## Vue d'ensemble du Moteur

### Architecture Globale

NovaEngine est structuré en **couches d'abstraction** :

```
┌─────────────────────────────────────────────────┐
│           GAME (Votre Code)                     │
├─────────────────────────────────────────────────┤
│  ECS (Entities, Components, Systems, Scenes)    │
├─────────────────────────────────────────────────┤
│  UI (UIManager, Components, Loader)             │
├─────────────────────────────────────────────────┤
│  Resources (ResourceManager, Types)             │
├─────────────────────────────────────────────────┤
│  Events (EventDispatcher, EventHandler)         │
├─────────────────────────────────────────────────┤
│  Core (Logger, ConfigManager, NovaEngine)       │
├─────────────────────────────────────────────────┤
│  Backend (Abstraction SFML)                     │
│  ├─ Graphics  ├─ Audio    ├─ Input              │
│  ├─ Window    ├─ Resource ├─ Font               │
└─────────────────────────────────────────────────┘
│  SFML (Bibliothèque externe)                    │
└─────────────────────────────────────────────────┘
```

### Principes de Design

1. **Abstraction** : Le backend SFML est abstrait par des interfaces
2. **Singletons** : Accès global via macros (GRAPHICS(), AUDIO(), etc.)
3. **Type Safety** : Utilisation de types forts (u8, u16, u32, u64, f32, f64)
4. **Logging Complet** : Tous les événements importants sont loggés
5. **Data-Driven** : Configuration via JSON

---

## Architecture Backend

Le backend fournit une **abstraction complète** de SFML, permettant de changer de bibliothèque graphique sans toucher au reste du code.

### BackendManager

**Singleton central** qui gère tous les backends.

```cpp
class BackendManager {
public:
    static BackendManager& getInstance();

    // Initialisation
    bool initialize();
    void shutdown();

    // Accès aux backends
    IGraphicsBackend& getGraphicsBackend();
    IAudioBackend& getAudioBackend();
    IInputBackend& getInputBackend();
    IWindowBackend& getWindowBackend();
    IResourceBackend& getResourceBackend();
    IFontBackend& getFontBackend();
    IViewportBackend& getViewportBackend();

private:
    std::unique_ptr<IGraphicsBackend> m_graphicsBackend;
    std::unique_ptr<IAudioBackend> m_audioBackend;
    // ... autres backends
};
```

**Macros d'accès global** :
```cpp
#define GRAPHICS() BackendManager::getInstance().getGraphicsBackend()
#define AUDIO() BackendManager::getInstance().getAudioBackend()
#define INPUT() BackendManager::getInstance().getInputBackend()
#define WINDOW() BackendManager::getInstance().getWindowBackend()
#define RESOURCES() BackendManager::getInstance().getResourceBackend()
#define FONTS() BackendManager::getInstance().getFontBackend()
#define VIEWPORT() BackendManager::getInstance().getViewportBackend()
```

### 1. IGraphicsBackend

Interface pour le **rendu graphique**.

```cpp
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;

    // Dessin de formes
    virtual void drawRectangle(const Vec2f& position, const Vec2f& size,
                              const Color& color, bool filled = true) = 0;
    virtual void drawCircle(const Vec2f& position, f32 radius,
                           const Color& color, bool filled = true) = 0;
    virtual void drawLine(const Vec2f& start, const Vec2f& end,
                         f32 thickness, const Color& color) = 0;
    virtual void drawTriangle(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3,
                             const Color& color, bool filled = true) = 0;

    // Dessin de textures
    virtual void drawTexture(TextureHandle handle, const Vec2f& position,
                            const IntRect* sourceRect = nullptr,
                            const Vec2f* scale = nullptr,
                            f32 rotation = 0.0f,
                            const Color* tint = nullptr,
                            const Vec2f* origin = nullptr) = 0;

    // Dessin de texte
    virtual void drawText(const std::string& text, const Vec2f& position,
                         u32 characterSize, const Color& color,
                         FontHandle fontHandle) = 0;

    // État de rendu
    virtual void setBlendMode(BlendMode mode) = 0;
    virtual void setTransform(const Transform& transform) = 0;
    virtual void resetTransform() = 0;
};
```

**Implémentation SFML** : `SFMLGraphicsBackend`

**Exemple d'utilisation** :
```cpp
// Dessiner un rectangle rouge
GRAPHICS().drawRectangle(Vec2f{100, 100}, Vec2f{200, 150}, Color::Red, true);

// Dessiner un cercle bleu
GRAPHICS().drawCircle(Vec2f{400, 300}, 50.0f, Color::Blue, false);

// Dessiner une texture avec rotation
GRAPHICS().drawTexture(
    textureHandle,
    Vec2f{640, 360},  // position
    nullptr,          // sourceRect (toute la texture)
    nullptr,          // scale (1:1)
    45.0f,            // rotation
    &Color::White,    // tint
    &Vec2f{16, 16}    // origin (centre de rotation)
);
```

### 2. IWindowBackend

Gère la **fenêtre principale**.

```cpp
class IWindowBackend {
public:
    virtual ~IWindowBackend() = default;

    // Gestion fenêtre
    virtual bool create(u32 width, u32 height, const std::string& title) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // Boucle de rendu
    virtual void clear(const Color& color) = 0;
    virtual void display() = 0;
    virtual bool pollEvent(Event& event) = 0;

    // Propriétés
    virtual Vec2u getSize() const = 0;
    virtual void setSize(const Vec2u& size) = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setFramerateLimit(u32 limit) = 0;
    virtual void setVerticalSyncEnabled(bool enabled) = 0;

    // Souris
    virtual Vec2i getMousePosition() const = 0;
    virtual void setMouseCursorVisible(bool visible) = 0;
};
```

**Exemple d'utilisation** :
```cpp
// Créer une fenêtre 1280x720
WINDOW().create(1280, 720, "Mon Jeu NovaEngine");

// Définir VSync
WINDOW().setVerticalSyncEnabled(true);

// Boucle principale
while (WINDOW().isOpen()) {
    Event event;
    while (WINDOW().pollEvent(event)) {
        // Gérer les événements
    }

    WINDOW().clear(Color::Black);
    // ... rendu ...
    WINDOW().display();
}
```

### 3. IInputBackend

Gère les **entrées clavier/souris**.

```cpp
class IInputBackend {
public:
    virtual ~IInputBackend() = default;

    // Clavier
    virtual bool isKeyPressed(Key key) const = 0;
    virtual bool isKeyJustPressed(Key key) const = 0;
    virtual bool isKeyJustReleased(Key key) const = 0;

    // Souris
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual bool isMouseButtonJustPressed(MouseButton button) const = 0;
    virtual bool isMouseButtonJustReleased(MouseButton button) const = 0;
    virtual Vec2i getMousePosition() const = 0;

    // Update (appelé chaque frame)
    virtual void update() = 0;
};
```

**Énumérations** :
```cpp
enum class Key {
    Unknown = -1,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, LControl, LShift, LAlt, Space, Enter,
    Left, Right, Up, Down,
    // ... etc
};

enum class MouseButton {
    Left, Right, Middle
};
```

**Exemple d'utilisation** :
```cpp
// Vérifier si une touche est maintenue
if (INPUT().isKeyPressed(Key::W)) {
    player.moveUp();
}

// Vérifier si une touche vient d'être pressée (pour les actions uniques)
if (INPUT().isKeyJustPressed(Key::Space)) {
    player.jump();
}

// Vérifier les boutons de souris
if (INPUT().isMouseButtonJustPressed(MouseButton::Left)) {
    Vec2i mousePos = INPUT().getMousePosition();
    handleClick(mousePos);
}
```

### 4. IAudioBackend

Gère le **son et la musique**.

```cpp
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    // Sons (courts, simultanés)
    virtual void playSound(SoundHandle handle, f32 volume = 100.0f, f32 pitch = 1.0f) = 0;
    virtual void stopSound(SoundHandle handle) = 0;
    virtual void pauseSound(SoundHandle handle) = 0;
    virtual void setSoundVolume(SoundHandle handle, f32 volume) = 0;
    virtual void setSoundPitch(SoundHandle handle, f32 pitch) = 0;
    virtual void setSoundLoop(SoundHandle handle, bool loop) = 0;

    // Musique (longue, streaming)
    virtual void playMusic(MusicHandle handle, f32 volume = 100.0f, bool loop = true) = 0;
    virtual void stopMusic(MusicHandle handle) = 0;
    virtual void pauseMusic(MusicHandle handle) = 0;
    virtual void setMusicVolume(MusicHandle handle, f32 volume) = 0;

    // Volume global
    virtual void setMasterVolume(f32 volume) = 0;
    virtual f32 getMasterVolume() const = 0;
};
```

**Exemple d'utilisation** :
```cpp
// Jouer un son court
SoundHandle footstep = RESOURCES().getSoundHandle("footstep.wav");
AUDIO().playSound(footstep, 80.0f);  // Volume 80%

// Jouer une musique en boucle
MusicHandle bgMusic = RESOURCES().getMusicHandle("theme.ogg");
AUDIO().playMusic(bgMusic, 50.0f, true);  // Volume 50%, loop

// Volume global
AUDIO().setMasterVolume(75.0f);
```

### 5. IResourceBackend

Gère le **chargement des ressources**.

```cpp
class IResourceBackend {
public:
    virtual ~IResourceBackend() = default;

    // Textures
    virtual TextureHandle loadTexture(const std::string& filepath) = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;

    // Sons
    virtual SoundHandle loadSound(const std::string& filepath) = 0;
    virtual void unloadSound(SoundHandle handle) = 0;

    // Musique
    virtual MusicHandle loadMusic(const std::string& filepath) = 0;
    virtual void unloadMusic(MusicHandle handle) = 0;

    // Validation
    virtual bool isValidTexture(TextureHandle handle) const = 0;
    virtual bool isValidSound(SoundHandle handle) const = 0;
    virtual bool isValidMusic(MusicHandle handle) const = 0;
};
```

**Types de handles** :
```cpp
using TextureHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
using FontHandle = u64;
```

### 6. IFontBackend

Gère les **polices de caractères**.

```cpp
class IFontBackend {
public:
    virtual ~IFontBackend() = default;

    virtual FontHandle loadFont(const std::string& filepath) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual bool isValidFont(FontHandle handle) const = 0;

    // Mesure de texte
    virtual Vec2f measureText(const std::string& text, FontHandle font,
                             u32 characterSize) const = 0;
};
```

### 7. IViewportBackend

Gère la **caméra/viewport**.

```cpp
class IViewportBackend {
public:
    virtual ~IViewportBackend() = default;

    virtual void setView(const View& view) = 0;
    virtual View getView() const = 0;
    virtual void resetView() = 0;

    // Conversions coordonnées
    virtual Vec2f mapPixelToCoords(const Vec2i& pixel) const = 0;
    virtual Vec2i mapCoordsToPixel(const Vec2f& coords) const = 0;
};
```

**Structure View** :
```cpp
struct View {
    Vec2f center = {0, 0};
    Vec2f size = {1280, 720};
    f32 rotation = 0.0f;
    FloatRect viewport = {0, 0, 1, 1};  // Proportion de la fenêtre
};
```

**Exemple d'utilisation** :
```cpp
// Créer une vue centrée sur le joueur
View view;
view.center = playerPosition;
view.size = Vec2f{1280, 720};
VIEWPORT().setView(view);

// Convertir coordonnées écran → monde
Vec2i mousePixel = INPUT().getMousePosition();
Vec2f worldPos = VIEWPORT().mapPixelToCoords(mousePixel);
```

---

## Core Systems

### Logger

Système de **logging multi-niveaux** avec sortie console et fichier.

```cpp
enum class LogLevel {
    Trace,    // Détails extrêmes
    Debug,    // Informations de debug
    Info,     // Informations générales
    Warn,     // Avertissements
    Error,    // Erreurs
    Critical  // Erreurs critiques
};

class Logger {
public:
    static Logger& getInstance();

    void setLogLevel(LogLevel level);
    void enableFileLogging(const std::string& filepath);
    void disableFileLogging();

    // Logging avec format style printf
    template<typename... Args>
    void trace(const std::string& format, Args&&... args);

    template<typename... Args>
    void debug(const std::string& format, Args&&... args);

    template<typename... Args>
    void info(const std::string& format, Args&&... args);

    template<typename... Args>
    void warn(const std::string& format, Args&&... args);

    template<typename... Args>
    void error(const std::string& format, Args&&... args);

    template<typename... Args>
    void critical(const std::string& format, Args&&... args);
};
```

**Macros** :
```cpp
#define LOG_TRACE(...) Logger::getInstance().trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...) Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().critical(__VA_ARGS__)
```

**Exemple d'utilisation** :
```cpp
LOG_INFO("Game initialized successfully");
LOG_DEBUG("Loading texture: {}", filepath);
LOG_WARN("Resource '{}' not found, using default", resourceID);
LOG_ERROR("Failed to load scene: {}", sceneName);
LOG_CRITICAL("Out of memory! Allocating {} bytes failed", size);
```

**Format des logs** :
```
[2025-01-15 14:32:45.123] [INFO] Game initialized successfully
[2025-01-15 14:32:45.456] [DEBUG] Loading texture: assets/player.png
[2025-01-15 14:32:46.789] [WARN] Resource 'missing_sound' not found, using default
[2025-01-15 14:32:47.012] [ERROR] Failed to load scene: corrupted_scene.json
```

### ConfigManager

Gestion de la **configuration** via JSON.

```cpp
class ConfigManager {
public:
    static ConfigManager& getInstance();

    // Chargement/Sauvegarde
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath);

    // Accès aux configurations
    const WindowConfig& getWindowConfig() const;
    const GraphicsConfig& getGraphicsConfig() const;
    const AudioConfig& getAudioConfig() const;
    const DebugConfig& getDebugConfig() const;

    // Modification
    void setWindowConfig(const WindowConfig& config);
    void setGraphicsConfig(const GraphicsConfig& config);
    void setAudioConfig(const AudioConfig& config);
    void setDebugConfig(const DebugConfig& config);
};
```

**Structures de configuration** :
```cpp
struct WindowConfig {
    u32 width = 1280;
    u32 height = 720;
    std::string title = "NovaEngine Game";
    bool fullscreen = false;
    bool vsync = true;
    u32 framerateLimit = 60;
};

struct GraphicsConfig {
    bool antialiasing = true;
    u32 antialiasingLevel = 8;
    bool showFPS = true;
};

struct AudioConfig {
    f32 masterVolume = 100.0f;
    f32 musicVolume = 80.0f;
    f32 sfxVolume = 100.0f;
};

struct DebugConfig {
    bool showColliders = false;
    bool showWaypoints = false;
    bool logToFile = true;
    std::string logFilepath = "game.log";
    LogLevel logLevel = LogLevel::Info;
};
```

**Fichier config.json** :
```json
{
  "window": {
    "width": 1920,
    "height": 1080,
    "title": "Mon Super Jeu",
    "fullscreen": false,
    "vsync": true,
    "framerateLimit": 60
  },
  "graphics": {
    "antialiasing": true,
    "antialiasingLevel": 8,
    "showFPS": true
  },
  "audio": {
    "masterVolume": 100.0,
    "musicVolume": 80.0,
    "sfxVolume": 100.0
  },
  "debug": {
    "showColliders": false,
    "showWaypoints": false,
    "logToFile": true,
    "logFilepath": "game.log",
    "logLevel": "Info"
  }
}
```

**Utilisation** :
```cpp
// Charger la config au démarrage
ConfigManager::getInstance().loadFromFile("config.json");

// Lire la config
const auto& windowConfig = ConfigManager::getInstance().getWindowConfig();
WINDOW().create(windowConfig.width, windowConfig.height, windowConfig.title);

// Modifier et sauvegarder
WindowConfig newConfig = windowConfig;
newConfig.fullscreen = true;
ConfigManager::getInstance().setWindowConfig(newConfig);
ConfigManager::getInstance().saveToFile("config.json");
```

### NovaEngine (Core Engine)

Classe principale qui **initialise et gère** tous les systèmes.

```cpp
class NovaEngine {
public:
    static NovaEngine& getInstance();

    // Lifecycle
    bool initialize(const std::string& configPath = "config.json");
    void shutdown();
    bool isInitialized() const;

    // Boucle principale (optionnelle, peut être gérée manuellement)
    void run(Application* app);

    // Accès
    f32 getDeltaTime() const;
    f32 getFPS() const;
    u64 getFrameCount() const;

private:
    bool m_initialized = false;
    f32 m_deltaTime = 0.0f;
    f32 m_fps = 0.0f;
    u64 m_frameCount = 0;
};
```

**Initialisation automatique** :
1. Charge `config.json`
2. Initialise Logger
3. Initialise BackendManager
4. Crée la fenêtre
5. Configure les paramètres graphiques/audio

**Exemple d'utilisation** :
```cpp
int main() {
    // Initialiser le moteur
    if (!NovaEngine::getInstance().initialize("config.json")) {
        return -1;
    }

    // Créer et lancer le jeu
    Game game;
    NovaEngine::getInstance().run(&game);

    // Cleanup
    NovaEngine::getInstance().shutdown();
    return 0;
}
```

---

## Resource Management

### ResourceManager

Gère le **cache et le lifecycle** de toutes les ressources.

```cpp
class ResourceManager {
public:
    static ResourceManager& getInstance();

    // Textures
    TextureHandle getTextureHandle(const ID& id);
    bool loadTexture(const ID& id, const std::string& filepath);
    void unloadTexture(const ID& id);
    Vec2u getTextureSize(const ID& id);

    // Sons
    SoundHandle getSoundHandle(const ID& id);
    bool loadSound(const ID& id, const std::string& filepath);
    void unloadSound(const ID& id);

    // Musique
    MusicHandle getMusicHandle(const ID& id);
    bool loadMusic(const ID& id, const std::string& filepath);
    void unloadMusic(const ID& id);

    // Fonts
    FontHandle getFontHandle(const ID& id);
    bool loadFont(const ID& id, const std::string& filepath);
    void unloadFont(const ID& id);

    // Gestion globale
    void unloadAll();
    void unloadAllTextures();
    void unloadAllSounds();
    void unloadAllMusic();
    void unloadAllFonts();

    // Statistiques
    size_t getTextureCount() const;
    size_t getSoundCount() const;
    size_t getMusicCount() const;
    size_t getFontCount() const;
};
```

**Principe** :
- Les ressources sont chargées **à la demande** ou **pré-chargées**
- Un **cache interne** évite les chargements multiples
- Les handles sont **réutilisables**

**Exemple d'utilisation** :
```cpp
// Pré-charger les ressources au démarrage
ResourceManager& rm = ResourceManager::getInstance();

rm.loadTexture("player", "assets/textures/player.png");
rm.loadTexture("enemy", "assets/textures/enemy.png");
rm.loadSound("jump", "assets/sounds/jump.wav");
rm.loadMusic("theme", "assets/music/theme.ogg");
rm.loadFont("main", "assets/fonts/arial.ttf");

// Utiliser les ressources
TextureHandle playerTex = rm.getTextureHandle("player");
GRAPHICS().drawTexture(playerTex, Vec2f{100, 100});

SoundHandle jumpSound = rm.getSoundHandle("jump");
AUDIO().playSound(jumpSound);

// Cleanup sélectif
rm.unloadTexture("enemy");

// Cleanup total
rm.unloadAll();
```

### ResourceTypes

Types de ressources supportés :

```cpp
// ID de ressource (string ou int)
using ID = std::string;

// Handles (entiers 64 bits)
using TextureHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
using FontHandle = u64;

// Invalid handles
constexpr TextureHandle INVALID_TEXTURE = 0;
constexpr SoundHandle INVALID_SOUND = 0;
constexpr MusicHandle INVALID_MUSIC = 0;
constexpr FontHandle INVALID_FONT = 0;
```

---

## UI System

### UIManager

Gère tous les **composants UI** et leur rendu.

```cpp
class UIManager {
public:
    static UIManager& getInstance();

    // Gestion des composants
    void addComponent(std::unique_ptr<UIComponent> component);
    void removeComponent(const std::string& id);
    UIComponent* getComponent(const std::string& id);

    // Update et Render
    void update(float deltaTime);
    void render();

    // Events
    void handleEvent(const Event& event);

    // Chargement depuis JSON
    bool loadFromFile(const std::string& filepath);

    // Cleanup
    void clear();
};
```

### UIComponent (Base)

Classe de base pour tous les composants UI.

```cpp
class UIComponent {
protected:
    std::string m_id;
    Vec2f m_position;
    Vec2f m_size;
    bool m_visible = true;
    bool m_enabled = true;

public:
    virtual ~UIComponent() = default;

    // Lifecycle
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleEvent(const Event& event) = 0;

    // Propriétés
    const std::string& getID() const { return m_id; }
    void setPosition(const Vec2f& pos) { m_position = pos; }
    void setSize(const Vec2f& size) { m_size = size; }
    void setVisible(bool visible) { m_visible = visible; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    // Collision
    virtual bool contains(const Vec2f& point) const;
};
```

### Composants UI Disponibles

#### 1. **Button**

```cpp
class Button : public UIComponent {
public:
    // États
    enum class State { Normal, Hovered, Pressed, Disabled };

    // Callbacks
    using ClickCallback = std::function<void()>;

    void setText(const std::string& text);
    void setFont(FontHandle font);
    void setTextSize(u32 size);
    void setBackgroundColor(const Color& color);
    void setTextColor(const Color& color);
    void setHoverColor(const Color& color);
    void setPressedColor(const Color& color);

    void setOnClick(ClickCallback callback);

    State getState() const;
};
```

**Exemple** :
```cpp
auto button = std::make_unique<Button>("btn_start");
button->setPosition(Vec2f{640, 360});
button->setSize(Vec2f{200, 50});
button->setText("Start Game");
button->setBackgroundColor(Color::Blue);
button->setOnClick([]() {
    LOG_INFO("Start button clicked!");
    // Démarrer le jeu
});
UIManager::getInstance().addComponent(std::move(button));
```

#### 2. **Panel**

```cpp
class Panel : public UIComponent {
public:
    void setBackgroundColor(const Color& color);
    void setBorderColor(const Color& color);
    void setBorderThickness(f32 thickness);
    void setOpacity(u8 alpha);

    // Children (pour créer des layouts)
    void addChild(UIComponent* child);
    void removeChild(const std::string& id);
};
```

#### 3. **Text**

```cpp
class Text : public UIComponent {
public:
    void setText(const std::string& text);
    void setFont(FontHandle font);
    void setCharacterSize(u32 size);
    void setColor(const Color& color);
    void setStyle(u32 style);  // Normal, Bold, Italic, Underlined

    std::string getText() const;
};
```

#### 4. **Image**

```cpp
class Image : public UIComponent {
public:
    void setTexture(TextureHandle texture);
    void setSourceRect(const IntRect& rect);
    void setTint(const Color& color);
    void setScale(const Vec2f& scale);
};
```

#### 5. **TextInput**

```cpp
class TextInput : public UIComponent {
public:
    void setPlaceholder(const std::string& text);
    void setMaxLength(u32 length);
    void setFont(FontHandle font);
    void setTextSize(u32 size);
    void setBackgroundColor(const Color& color);
    void setTextColor(const Color& color);
    void setFocusColor(const Color& color);

    void setOnSubmit(std::function<void(const std::string&)> callback);
    void setOnTextChanged(std::function<void(const std::string&)> callback);

    std::string getText() const;
    void setText(const std::string& text);
    void clear();
    bool isFocused() const;
};
```

#### 6. **Slider**

```cpp
class Slider : public UIComponent {
public:
    void setMinValue(f32 min);
    void setMaxValue(f32 max);
    void setValue(f32 value);
    void setStepSize(f32 step);

    void setBarColor(const Color& color);
    void setHandleColor(const Color& color);
    void setHandleSize(const Vec2f& size);

    void setOnValueChanged(std::function<void(f32)> callback);

    f32 getValue() const;
    f32 getPercentage() const;
};
```

### UILoader

Charge des **interfaces depuis JSON**.

```cpp
class UILoader {
public:
    static bool loadUI(const std::string& filepath, UIManager& uiManager);
    static std::unique_ptr<UIComponent> createComponent(const nlohmann::json& data);
};
```

**Exemple UI JSON** :
```json
{
  "components": [
    {
      "type": "Panel",
      "id": "main_menu",
      "position": [0, 0],
      "size": [1280, 720],
      "backgroundColor": [30, 30, 40, 255],
      "children": [
        {
          "type": "Text",
          "id": "title",
          "position": [640, 100],
          "text": "Mon Jeu NovaEngine",
          "characterSize": 48,
          "color": [255, 255, 255, 255]
        },
        {
          "type": "Button",
          "id": "btn_start",
          "position": [640, 300],
          "size": [200, 50],
          "text": "Nouvelle Partie",
          "backgroundColor": [50, 100, 200, 255],
          "textColor": [255, 255, 255, 255]
        },
        {
          "type": "Button",
          "id": "btn_options",
          "position": [640, 370],
          "size": [200, 50],
          "text": "Options",
          "backgroundColor": [50, 100, 200, 255]
        },
        {
          "type": "Button",
          "id": "btn_quit",
          "position": [640, 440],
          "size": [200, 50],
          "text": "Quitter",
          "backgroundColor": [200, 50, 50, 255]
        }
      ]
    }
  ]
}
```

**Chargement** :
```cpp
UILoader::loadUI("assets/ui/main_menu.json", UIManager::getInstance());

// Attacher les callbacks
Button* btnStart = dynamic_cast<Button*>(
    UIManager::getInstance().getComponent("btn_start")
);
btnStart->setOnClick([]() {
    // Démarrer le jeu
});
```

---

## Event System

### Event

Structure représentant un **événement**.

```cpp
enum class EventType {
    None,
    WindowClosed,
    WindowResized,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseWheelScrolled,
    TextEntered,
    // ... custom events
};

struct Event {
    EventType type = EventType::None;

    // Données spécifiques
    union {
        struct { u32 width, height; } size;           // WindowResized
        struct { Key code; } key;                     // KeyPressed/Released
        struct { MouseButton button; i32 x, y; } mouseButton;
        struct { i32 x, i32 y; } mouseMove;
        struct { f32 delta; i32 x, y; } mouseWheel;
        struct { u32 unicode; } text;
    };
};
```

### EventDispatcher

Gère la **distribution d'événements** avec un système de listeners.

```cpp
class EventDispatcher {
public:
    static EventDispatcher& getInstance();

    // Enregistrer des listeners
    using EventCallback = std::function<void(const Event&)>;
    u64 addEventListener(EventType type, EventCallback callback);
    void removeEventListener(u64 id);

    // Dispatcher un événement
    void dispatch(const Event& event);

    // Dispatcher à tous
    void broadcastEvent(const Event& event);
};
```

**Exemple d'utilisation** :
```cpp
// Écouter les touches
u64 keyListenerID = EventDispatcher::getInstance().addEventListener(
    EventType::KeyPressed,
    [](const Event& e) {
        if (e.key.code == Key::Escape) {
            LOG_INFO("Escape pressed, opening menu");
            // Ouvrir le menu
        }
    }
);

// Plus tard, retirer le listener
EventDispatcher::getInstance().removeEventListener(keyListenerID);

// Dispatcher un événement custom
Event customEvent;
customEvent.type = EventType::PlayerDied;
EventDispatcher::getInstance().dispatch(customEvent);
```

### EventHandler

Helper pour gérer les événements dans vos classes.

```cpp
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void onEvent(const Event& event) = 0;
};
```

---

## Application & Game

### Application (Base)

Classe de base pour créer votre jeu.

```cpp
class Application {
public:
    struct Config {
        std::string title = "NovaEngine Application";
        u32 width = 1280;
        u32 height = 720;
        bool fullscreen = false;
        bool vsync = true;
        u32 framerateLimit = 60;
    };

    virtual ~Application() = default;

    // Lifecycle (à implémenter)
    virtual void onInitialize() = 0;
    virtual void onShutdown() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onEvent(const Event& event) {}

    // Configuration
    virtual Config createConfig() const;

    // Contrôle
    void quit();
    bool isRunning() const;
};
```

### Game (Exemple)

Votre classe de jeu hérite d'Application.

```cpp
class Game : public NovaEngine::Application {
private:
    NovaEngine::SceneManager m_sceneManager;
    // Vos autres systèmes...

public:
    void onInitialize() override {
        LOG_INFO("Game initializing...");

        // Charger ressources
        RESOURCES().loadTexture("player", "assets/player.png");
        RESOURCES().loadSound("jump", "assets/jump.wav");

        // Initialiser ECS
        m_sceneManager.initialize(
            "assets/data/definitions",
            "assets/data/scenegraph.json"
        );
        m_sceneManager.loadScene("main", "assets/data/scenes/main.json");
        m_sceneManager.setActiveScene("main");

        // Charger UI
        UILoader::loadUI("assets/ui/hud.json", UIManager::getInstance());

        LOG_INFO("Game initialized successfully");
    }

    void onShutdown() override {
        LOG_INFO("Game shutting down...");
        RESOURCES().unloadAll();
    }

    void onUpdate(float deltaTime) override {
        // Update ECS
        m_sceneManager.update(deltaTime);

        // Update UI
        UIManager::getInstance().update(deltaTime);

        // Votre logique de jeu
        // ...
    }

    void onRender() override {
        // Render ECS
        m_sceneManager.render();

        // Render UI
        UIManager::getInstance().render();
    }

    void onEvent(const Event& event) override {
        if (event.type == EventType::KeyPressed) {
            if (event.key.code == Key::Escape) {
                quit();
            }
        }

        // Passer aux systèmes UI
        UIManager::getInstance().handleEvent(event);
    }

    Config createConfig() const override {
        Config config;
        config.title = "Mon Jeu NovaEngine";
        config.width = 1920;
        config.height = 1080;
        config.vsync = true;
        return config;
    }
};
```

---

## Types de Base

### Types Numériques

```cpp
// Entiers non signés
using u8 = uint8_t;      // 0 à 255
using u16 = uint16_t;    // 0 à 65,535
using u32 = uint32_t;    // 0 à 4,294,967,295
using u64 = uint64_t;    // 0 à 18,446,744,073,709,551,615

// Entiers signés
using i8 = int8_t;       // -128 à 127
using i16 = int16_t;     // -32,768 à 32,767
using i32 = int32_t;     // -2,147,483,648 à 2,147,483,647
using i64 = int64_t;     // Très grand

// Flottants
using f32 = float;       // Précision simple
using f64 = double;      // Précision double
```

### Vecteurs

```cpp
template<typename T>
struct Vec2 {
    T x, y;

    Vec2() : x(0), y(0) {}
    Vec2(T x_, T y_) : x(x_), y(y_) {}

    // Opérateurs
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(T scalar) const;
    Vec2 operator/(T scalar) const;

    // Fonctions utilitaires
    T length() const;
    T lengthSquared() const;
    Vec2 normalized() const;
    T dot(const Vec2& other) const;
};

using Vec2f = Vec2<f32>;
using Vec2i = Vec2<i32>;
using Vec2u = Vec2<u32>;
```

### Rectangles

```cpp
template<typename T>
struct Rect {
    T left, top, width, height;

    Rect() : left(0), top(0), width(0), height(0) {}
    Rect(T l, T t, T w, T h) : left(l), top(t), width(w), height(h) {}

    bool contains(T x, T y) const;
    bool contains(const Vec2<T>& point) const;
    bool intersects(const Rect& other) const;
};

using IntRect = Rect<i32>;
using FloatRect = Rect<f32>;
```

### Couleurs

```cpp
struct Color {
    u8 r, g, b, a;

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
constexpr Color Color::Black{0, 0, 0, 255};
constexpr Color Color::White{255, 255, 255, 255};
constexpr Color Color::Red{255, 0, 0, 255};
// etc...
```

### Transform

```cpp
struct Transform {
    Vec2f translation = {0, 0};
    f32 rotation = 0.0f;     // En degrés
    Vec2f scale = {1, 1};
    Vec2f origin = {0, 0};

    // Combine des transformations
    Transform combine(const Transform& other) const;

    // Transforme un point
    Vec2f transformPoint(const Vec2f& point) const;
};
```

---

## Configuration

### Structure des Fichiers

```
project/
├── assets/
│   ├── textures/
│   ├── sounds/
│   ├── music/
│   ├── fonts/
│   ├── data/
│   │   ├── definitions/
│   │   │   ├── Sprites.json
│   │   │   ├── Lights.json
│   │   │   ├── Animations.json
│   │   │   ├── Audio.json
│   │   │   ├── Activators.json
│   │   │   └── NPCs.json
│   │   ├── scenes/
│   │   │   ├── ville.json
│   │   │   ├── maison.json
│   │   │   └── ...
│   │   └── scenegraph.json
│   └── ui/
│       ├── main_menu.json
│       ├── hud.json
│       └── ...
├── config.json
├── game.log
└── executable
```

---

## Guide d'Utilisation Complet

### 1. Initialisation Minimale

```cpp
#include <NovaEngine/NovaEngine.hpp>
#include <NovaEngine/Game.hpp>

class MyGame : public NovaEngine::Application {
public:
    void onInitialize() override {
        LOG_INFO("Game starting");
    }

    void onShutdown() override {
        LOG_INFO("Game ending");
    }

    void onUpdate(float deltaTime) override {
        // Logique de jeu
    }

    void onRender() override {
        // Rendu simple
        GRAPHICS().drawRectangle(
            Vec2f{100, 100},
            Vec2f{200, 150},
            Color::Red
        );
    }
};

int main() {
    NovaEngine::NovaEngine::getInstance().initialize("config.json");

    MyGame game;
    NovaEngine::NovaEngine::getInstance().run(&game);

    NovaEngine::NovaEngine::getInstance().shutdown();
    return 0;
}
```

### 2. Jeu Complet avec ECS

```cpp
class FullGame : public NovaEngine::Application {
private:
    NovaEngine::SceneManager m_sceneManager;
    NovaEngine::ResourceManager& m_resources;

public:
    FullGame() : m_resources(NovaEngine::ResourceManager::getInstance()) {}

    void onInitialize() override {
        // 1. Charger les ressources
        m_resources.loadTexture("player", "assets/player.png");
        m_resources.loadTexture("enemy", "assets/enemy.png");
        m_resources.loadSound("shoot", "assets/shoot.wav");
        m_resources.loadMusic("bgm", "assets/music.ogg");

        // 2. Initialiser ECS
        m_sceneManager.initialize(
            "assets/data/definitions",
            "assets/data/scenegraph.json"
        );

        // 3. Charger les scènes
        m_sceneManager.loadScene("game", "assets/data/scenes/game.json");
        m_sceneManager.setActiveScene("game");

        // 4. Charger l'UI
        UILoader::loadUI("assets/ui/hud.json", UIManager::getInstance());

        // 5. Jouer la musique
        MusicHandle bgm = m_resources.getMusicHandle("bgm");
        AUDIO().playMusic(bgm, 50.0f, true);

        LOG_INFO("Game initialized");
    }

    void onUpdate(float deltaTime) override {
        // Update ECS
        m_sceneManager.update(deltaTime);

        // Update UI
        UIManager::getInstance().update(deltaTime);

        // Contrôles joueur
        handleInput();
    }

    void onRender() override {
        // Render ECS
        m_sceneManager.render();

        // Render UI
        UIManager::getInstance().render();

        // FPS counter
        if (ConfigManager::getInstance().getGraphicsConfig().showFPS) {
            drawFPS();
        }
    }

    void onEvent(const Event& event) override {
        // Events UI
        UIManager::getInstance().handleEvent(event);

        // Events custom
        if (event.type == EventType::KeyPressed) {
            if (event.key.code == Key::Escape) {
                // Ouvrir le menu pause
            }
        }
    }

private:
    void handleInput() {
        auto* scene = m_sceneManager.getActiveScene();
        if (!scene) return;

        // Trouver le joueur
        auto& registry = scene->getEntityRegistry();
        auto players = registry.getEntitiesWith({"TagComponent", "TransformComponent"});

        for (auto* player : players) {
            auto* tag = player->getComponent<TagComponent>();
            if (tag->tag != "player") continue;

            auto* transform = player->getComponent<TransformComponent>();

            // Mouvements
            f32 speed = 200.0f;
            if (INPUT().isKeyPressed(Key::W)) {
                transform->position.y -= speed * NovaEngine::getInstance().getDeltaTime();
            }
            if (INPUT().isKeyPressed(Key::S)) {
                transform->position.y += speed * NovaEngine::getInstance().getDeltaTime();
            }
            if (INPUT().isKeyPressed(Key::A)) {
                transform->position.x -= speed * NovaEngine::getInstance().getDeltaTime();
            }
            if (INPUT().isKeyPressed(Key::D)) {
                transform->position.x += speed * NovaEngine::getInstance().getDeltaTime();
            }

            // Tirer
            if (INPUT().isKeyJustPressed(Key::Space)) {
                SoundHandle shoot = m_resources.getSoundHandle("shoot");
                AUDIO().playSound(shoot);
                // Créer un projectile...
            }
        }
    }

    void drawFPS() {
        std::string fpsText = "FPS: " + std::to_string((int)NovaEngine::getInstance().getFPS());
        FontHandle font = m_resources.getFontHandle("main");
        GRAPHICS().drawText(fpsText, Vec2f{10, 10}, 14, Color::Yellow, font);
    }
};
```

### 3. Menu System

```cpp
class MenuSystem {
private:
    enum class MenuState { MainMenu, Options, InGame, Paused };
    MenuState m_state = MenuState::MainMenu;

public:
    void initialize() {
        showMainMenu();
    }

    void showMainMenu() {
        m_state = MenuState::MainMenu;

        UIManager::getInstance().clear();
        UILoader::loadUI("assets/ui/main_menu.json", UIManager::getInstance());

        // Attacher les callbacks
        auto* btnStart = dynamic_cast<Button*>(
            UIManager::getInstance().getComponent("btn_start")
        );
        btnStart->setOnClick([this]() {
            startGame();
        });

        auto* btnOptions = dynamic_cast<Button*>(
            UIManager::getInstance().getComponent("btn_options")
        );
        btnOptions->setOnClick([this]() {
            showOptions();
        });

        auto* btnQuit = dynamic_cast<Button*>(
            UIManager::getInstance().getComponent("btn_quit")
        );
        btnQuit->setOnClick([]() {
            WINDOW().close();
        });
    }

    void showOptions() {
        m_state = MenuState::Options;

        UIManager::getInstance().clear();
        UILoader::loadUI("assets/ui/options.json", UIManager::getInstance());

        // Slider de volume
        auto* volumeSlider = dynamic_cast<Slider*>(
            UIManager::getInstance().getComponent("volume_slider")
        );
        volumeSlider->setValue(AUDIO().getMasterVolume());
        volumeSlider->setOnValueChanged([](f32 value) {
            AUDIO().setMasterVolume(value);
        });

        // Bouton retour
        auto* btnBack = dynamic_cast<Button*>(
            UIManager::getInstance().getComponent("btn_back")
        );
        btnBack->setOnClick([this]() {
            showMainMenu();
        });
    }

    void startGame() {
        m_state = MenuState::InGame;
        UIManager::getInstance().clear();
        UILoader::loadUI("assets/ui/hud.json", UIManager::getInstance());
        // Démarrer le jeu...
    }

    void showPauseMenu() {
        m_state = MenuState::Paused;
        // ...
    }

    MenuState getState() const { return m_state; }
};
```

---

## Conclusion

### Points Forts de NovaEngine

✅ **Abstraction Backend Complète** : Changez de bibliothèque graphique facilement
✅ **Type Safety** : Types forts pour éviter les erreurs
✅ **Logging Robuste** : Débogage facilité
✅ **Configuration Flexible** : JSON pour tout paramétrer
✅ **Resource Management** : Cache automatique, handles sécurisés
✅ **UI System Complet** : Composants prêts à l'emploi
✅ **Event System** : Architecture découplée
✅ **ECS Intégré** : Voir DOCUMENTATION_ECS.md

### Performance

- **Abstraction Zero-Cost** : Les interfaces virtuelles ont un coût minimal
- **Cache de Ressources** : Évite les chargements redondants
- **Update Sélectif** : Seules les scènes actives sont mises à jour
- **Batch Rendering** : Les sprites sont triés par z-order pour optimiser

### Limitations Actuelles

- Backend SFML uniquement (mais architecture prête pour d'autres)
- Pas de système de particules built-in
- Pas de système de networking
- Pas de système de sauvegarde built-in

### Roadmap

- [ ] Backend Vulkan/OpenGL
- [ ] Particle System
- [ ] Physics Engine (Box2D)
- [ ] Networking (ENet)
- [ ] Save/Load System
- [ ] Scripting (Lua)
- [ ] Level Editor

---

**Félicitations !** Vous maîtrisez maintenant l'architecture complète de NovaEngine ! 🎮✨

Pour l'ECS, consultez **DOCUMENTATION_ECS.md**.
