# Backend Abstraction - NovaEngine

Le système de backend abstrait toutes les dépendances aux bibliothèques externes (SFML, SDL, etc.), permettant de changer de bibliothèque graphique sans modifier le code du jeu.

## Vue d'ensemble

```
┌────────────────────────────────────────┐
│         GAME CODE                      │
│  Utilise uniquement les interfaces     │
│  GRAPHICS(), WINDOW(), INPUT()         │
└────────────────────────────────────────┘
                 ↓
┌────────────────────────────────────────┐
│      BACKEND ABSTRACTION LAYER         │
│  ┌──────────────────────────────────┐ │
│  │    BackendManager (singleton)    │ │
│  │  - Crée et gère tous backends    │ │
│  │  - Macros d'accès global         │ │
│  └──────────────────────────────────┘ │
│  ┌────────┬────────┬─────────┬──────┐│
│  │Graphics│ Window │  Input  │Audio ││
│  │Backend │Backend │ Backend │Backend││
│  └────────┴────────┴─────────┴──────┘│
│  ┌────────┬────────┬─────────┐       │
│  │  Font  │Resource│Viewport │       │
│  │Backend │Backend │Backend  │       │
│  └────────┴────────┴─────────┘       │
└────────────────────────────────────────┘
                 ↓
┌────────────────────────────────────────┐
│    IMPLEMENTATION (SFML, SDL, etc.)    │
│  - SFMLGraphicsBackend                 │
│  - SFMLWindowBackend                   │
│  - SFMLInputBackend                    │
│  - etc.                                │
└────────────────────────────────────────┘
```

---

## Structure des fichiers

```
sdk/include/NovaEngine/Backend/
├── BackendManager.hpp              # Gestionnaire principal
├── Core/
│   └── BackendTypes.hpp            # Types communs (Vec2f, Color, etc.)
├── Interfaces/                     # Interfaces abstraites
│   ├── IGraphicsBackend.hpp
│   ├── IWindowBackend.hpp
│   ├── IInputBackend.hpp
│   ├── IAudioBackend.hpp
│   ├── IFontBackend.hpp
│   ├── IResourceBackend.hpp
│   └── IViewportBackend.hpp
└── SFML/                           # Implémentation SFML
    ├── SFMLGraphicsBackend.hpp
    ├── SFMLWindowBackend.hpp
    ├── SFMLInputBackend.hpp
    ├── SFMLAudioBackend.hpp
    ├── SFMLFontBackend.hpp
    ├── SFMLResourceBackend.hpp
    ├── SFMLViewportBackend.hpp
    ├── SFMLConversions.hpp         # Utilitaires de conversion
    ├── SFMLDrawable.hpp
    └── SFMLHelpers.hpp
```

---

## BackendManager

### Singleton gérant tous les backends

```cpp
class BackendManager {
public:
    // Accès singleton
    static BackendManager& get();

    // Initialisation/shutdown
    bool initialize(BackendType type = BackendType::SFML,
                   u32 windowWidth = 800,
                   u32 windowHeight = 600,
                   const String& windowTitle = "NovaEngine",
                   bool fullscreen = false);
    void shutdown();

    // Status
    bool isInitialized() const;
    BackendType getCurrentBackendType() const;

    // Accès aux backends
    IWindowBackend& window();
    IInputBackend& input();
    IGraphicsBackend& graphics();
    IResourceBackend& resources();
    IAudioBackend& audio();
    IFontBackend& fonts();
    IViewportBackend& viewport();

private:
    BackendManager();  // Privé (singleton)
    ~BackendManager();

    bool createBackends(BackendType type);
    void destroyBackends();

    bool m_initialized;
    BackendType m_currentBackend;

    // Tous les backends
    std::unique_ptr<IWindowBackend> m_window;
    std::unique_ptr<IInputBackend> m_input;
    std::unique_ptr<IGraphicsBackend> m_graphics;
    std::unique_ptr<IResourceBackend> m_resources;
    std::unique_ptr<IAudioBackend> m_audio;
    std::unique_ptr<IFontBackend> m_fonts;
    std::unique_ptr<IViewportBackend> m_viewport;
};
```

### Macros d'accès global

```cpp
// Définies dans BackendManager.hpp
#define BACKEND()   NovaEngine::BackendManager::get()
#define WINDOW()    NovaEngine::BackendManager::get().window()
#define INPUT()     NovaEngine::BackendManager::get().input()
#define GRAPHICS()  NovaEngine::BackendManager::get().graphics()
#define RESOURCES() NovaEngine::BackendManager::get().resources()
#define AUDIO()     NovaEngine::BackendManager::get().audio()
#define FONTS()     NovaEngine::BackendManager::get().fonts()
#define VIEWPORT()  NovaEngine::BackendManager::get().viewport()
```

### Utilisation

```cpp
// Initialisation (faite par Application)
BACKEND().initialize(BackendType::SFML, 1920, 1080, "Mon Jeu", false);

// Utilisation partout dans le code
WINDOW().setTitle("Nouveau titre");
GRAPHICS().drawSprite(spriteData);
TextureHandle tex = RESOURCES().loadTexture("path/to/texture.png");
```

---

## BackendTypes.hpp

### Types communs à tous les backends

#### Vecteurs

```cpp
struct Vec2f {
    f32 x, y;

    Vec2f() : x(0), y(0) {}
    Vec2f(f32 x, f32 y) : x(x), y(y) {}

    // Opérateurs
    Vec2f operator+(const Vec2f& o) const { return Vec2f(x + o.x, y + o.y); }
    Vec2f operator-(const Vec2f& o) const { return Vec2f(x - o.x, y - o.y); }
    Vec2f operator*(f32 s) const { return Vec2f(x * s, y * s); }
    Vec2f operator/(f32 s) const { return Vec2f(x / s, y / s); }
    Vec2f& operator+=(const Vec2f& o) { x += o.x; y += o.y; return *this; }
    // ... etc
};

struct Vec2i { i32 x, y; /* ... */ };
struct Vec2u { u32 x, y; /* ... */ };
struct Vec3f { f32 x, y, z; /* ... */ };
```

#### Rectangles

```cpp
struct Rect {
    f32 left, top, width, height;

    Rect() : left(0), top(0), width(0), height(0) {}
    Rect(f32 l, f32 t, f32 w, f32 h) : left(l), top(t), width(w), height(h) {}

    bool contains(f32 x, f32 y) const {
        return x >= left && x <= left + width &&
               y >= top && y <= top + height;
    }

    bool contains(const Vec2f& p) const { return contains(p.x, p.y); }
};

struct IntRect {
    i32 left, top, width, height;
    // ...
};
```

#### Couleurs

```cpp
struct Color {
    u8 r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}

    // Couleurs prédéfinies
    static const Color Black, White, Red, Green, Blue, Yellow, Transparent;

    bool operator==(const Color& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

// Implémentation des constantes (dans .cpp)
const Color Color::Black{0, 0, 0, 255};
const Color Color::White{255, 255, 255, 255};
const Color Color::Red{255, 0, 0, 255};
const Color Color::Green{0, 255, 0, 255};
const Color Color::Blue{0, 0, 255, 255};
const Color Color::Yellow{255, 255, 0, 255};
const Color Color::Transparent{0, 0, 0, 0};
```

#### Handles de ressources

```cpp
using TextureHandle = u64;
using FontHandle = u64;
using SoundHandle = u64;
using MusicHandle = u64;
using ShaderHandle = u64;
using RenderTextureHandle = u64;

constexpr u64 INVALID_HANDLE = 0;
```

**Pourquoi des handles ?**
- Évite les pointeurs invalides (dangling pointers)
- Permet le resource pooling
- Type-safe (TextureHandle ≠ SoundHandle)
- Facilite le hot-reloading

#### Enums

```cpp
enum class BackendType { SFML, SDL, Custom };

enum class BlendMode { Alpha, Add, Multiply, None };

enum class KeyCode {
    Unknown = -1,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, LControl, LShift, LAlt, LSystem,
    RControl, RShift, RAlt, RSystem,
    Space, Enter, Backspace, Tab,
    Left, Right, Up, Down
};

enum class MouseButton { Left, Right, Middle };

enum class InputEventType {
    Closed, Resized,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved,
    TextEntered
};

enum class TextStyle {
    Regular = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    Underlined = 1 << 2,
    StrikeThrough = 1 << 3
};

enum class SoundStatus { Stopped, Paused, Playing };
```

#### Structures de données

```cpp
// Input event
struct InputEvent {
    InputEventType type;
    union {
        struct { u32 width, height; } size;
        struct { u32 unicode; } text;
        struct { KeyCode code; bool alt, control, shift, system; } key;
        struct { MouseButton button; i32 x, y; } mouseButton;
        struct { i32 x, y; } mouseMove;
    };
};

// Transform
struct Transform {
    Vec2f position;
    f32 rotation;
    Vec2f scale;
    Vec2f origin;
};

// Sprite data
struct SpriteData {
    TextureHandle texture;
    Vec2f position;
    Vec2f size;
    f32 rotation;
    Vec2f scale;
    Vec2f origin;
    IntRect textureRect;
    Color color;
    BlendMode blendMode;
    ShaderHandle shader;
};

// Rectangle data
struct RectData {
    Vec2f position;
    Vec2f size;
    Color fillColor;
    Color outlineColor;
    f32 outlineThickness;
    f32 rotation;
    Vec2f origin;
};

// Text data
struct TextData {
    String text;
    FontHandle font;
    u32 characterSize;
    Color fillColor;
    Color outlineColor;
    f32 outlineThickness;
    TextStyle style;
    Vec2f position;
    f32 rotation;
    Vec2f scale;
    Vec2f origin;
    BlendMode blendMode;
};

// Text metrics
struct TextMetrics {
    f32 width, height, baseline;
};

// Viewport data
struct ViewportData {
    Rect viewport;   // (0-1, 0-1) normalized
    Vec2f center;    // World center
    Vec2f size;      // World size
    f32 rotation;
};
```

---

## Interfaces Backend

### IGraphicsBackend

```cpp
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;

    // Primitives de rendu
    virtual void drawSprite(const SpriteData& data) = 0;
    virtual void drawRect(const RectData& data) = 0;
    virtual void drawText(const TextData& data) = 0;

    // Gestion des textures
    virtual TextureHandle createTexture(u32 width, u32 height) = 0;
    virtual void updateTexture(TextureHandle handle, const u8* pixels,
                              u32 width, u32 height, u32 x, u32 y) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;

    // Gestion des shaders
    virtual ShaderHandle loadShader(const String& vertexPath,
                                    const String& fragmentPath) = 0;
    virtual void destroyShader(ShaderHandle handle) = 0;
    virtual void setShaderUniform(ShaderHandle handle,
                                 const String& name, f32 value) = 0;
    virtual void setShaderUniform(ShaderHandle handle,
                                 const String& name, const Vec2f& value) = 0;
    // ... (autres types: Vec3f, Color, int, etc.)

    // Render textures
    virtual RenderTextureHandle createRenderTexture(u32 width, u32 height) = 0;
    virtual void destroyRenderTexture(RenderTextureHandle handle) = 0;
    virtual void setRenderTarget(RenderTextureHandle handle) = 0;
    virtual void resetRenderTarget() = 0;
    virtual TextureHandle getRenderTextureTexture(RenderTextureHandle handle) = 0;
};
```

### IWindowBackend

```cpp
class IWindowBackend {
public:
    virtual ~IWindowBackend() = default;

    // Gestion fenêtre
    virtual bool create(u32 width, u32 height,
                       const String& title, bool fullscreen) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // Propriétés
    virtual void setTitle(const String& title) = 0;
    virtual void setFramerateLimit(u32 limit) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;

    // Rendu
    virtual void clear(const Color& color) = 0;
    virtual void display() = 0;
};
```

### IInputBackend

```cpp
class IInputBackend {
public:
    virtual ~IInputBackend() = default;

    // Poll events
    virtual bool pollEvent(InputEvent& event) = 0;

    // État clavier
    virtual bool isKeyPressed(KeyCode key) const = 0;

    // État souris
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual Vec2i getMousePosition() const = 0;
    virtual void setMousePosition(const Vec2i& position) = 0;
};
```

### IAudioBackend

```cpp
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    // Sons (effets courts)
    virtual void playSound(SoundHandle handle, f32 volume = 100.0f,
                          f32 pitch = 1.0f, bool loop = false) = 0;
    virtual void stopSound(SoundHandle handle) = 0;
    virtual void pauseSound(SoundHandle handle) = 0;
    virtual SoundStatus getSoundStatus(SoundHandle handle) const = 0;

    // Musique (streaming long)
    virtual void playMusic(MusicHandle handle, f32 volume = 100.0f,
                          bool loop = true) = 0;
    virtual void stopMusic(MusicHandle handle) = 0;
    virtual void pauseMusic(MusicHandle handle) = 0;
    virtual void setMusicVolume(MusicHandle handle, f32 volume) = 0;
};
```

### IFontBackend

```cpp
class IFontBackend {
public:
    virtual ~IFontBackend() = default;

    // Calcul de dimensions de texte
    virtual TextMetrics measureText(const String& text, FontHandle font,
                                   u32 characterSize) const = 0;

    // Récupérer line height
    virtual f32 getLineHeight(FontHandle font, u32 characterSize) const = 0;
};
```

### IResourceBackend

```cpp
class IResourceBackend {
public:
    virtual ~IResourceBackend() = default;

    // Chargement ressources
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual FontHandle loadFont(const String& path) = 0;
    virtual SoundHandle loadSound(const String& path) = 0;
    virtual MusicHandle loadMusic(const String& path) = 0;

    // Libération ressources
    virtual void unloadTexture(TextureHandle handle) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual void unloadSound(SoundHandle handle) = 0;
    virtual void unloadMusic(MusicHandle handle) = 0;
};
```

### IViewportBackend

```cpp
class IViewportBackend {
public:
    virtual ~IViewportBackend() = default;

    // Gestion de la vue/caméra
    virtual void setView(const ViewportData& data) = 0;
    virtual ViewportData getView() const = 0;
    virtual void setViewCenter(const Vec2f& center) = 0;
    virtual void setViewSize(const Vec2f& size) = 0;
    virtual void resetView() = 0;

    // Conversions coordonnées
    virtual Vec2f screenToWorld(const Vec2i& screenPos) const = 0;
    virtual Vec2i worldToScreen(const Vec2f& worldPos) const = 0;
};
```

---

## Implémentation SFML

### SFMLGraphicsBackend

```cpp
class SFMLGraphicsBackend : public IGraphicsBackend {
public:
    SFMLGraphicsBackend(sf::RenderWindow* window);
    ~SFMLGraphicsBackend() override;

    void drawSprite(const SpriteData& data) override;
    void drawRect(const RectData& data) override;
    void drawText(const TextData& data) override;

    TextureHandle createTexture(u32 width, u32 height) override;
    // ... (reste des méthodes)

private:
    sf::RenderWindow* m_window;

    // Cache des ressources
    std::unordered_map<TextureHandle, sf::Texture> m_textures;
    std::unordered_map<ShaderHandle, sf::Shader> m_shaders;
    std::unordered_map<RenderTextureHandle, sf::RenderTexture> m_renderTextures;

    u64 m_nextTextureID = 1;
    u64 m_nextShaderID = 1;
    u64 m_nextRenderTextureID = 1;

    // Utilitaires de conversion
    sf::Vector2f toSFML(const Vec2f& vec) const;
    sf::Color toSFML(const Color& color) const;
    // ...
};
```

### SFMLConversions

```cpp
// Conversions de types
namespace SFMLConversions {
    // Vec2f ⇄ sf::Vector2f
    inline sf::Vector2f toSFML(const Vec2f& v) {
        return sf::Vector2f(v.x, v.y);
    }

    inline Vec2f fromSFML(const sf::Vector2f& v) {
        return Vec2f{v.x, v.y};
    }

    // Color ⇄ sf::Color
    inline sf::Color toSFML(const Color& c) {
        return sf::Color(c.r, c.g, c.b, c.a);
    }

    inline Color fromSFML(const sf::Color& c) {
        return Color{c.r, c.g, c.b, c.a};
    }

    // IntRect ⇄ sf::IntRect
    inline sf::IntRect toSFML(const IntRect& r) {
        return sf::IntRect(r.left, r.top, r.width, r.height);
    }

    inline IntRect fromSFML(const sf::IntRect& r) {
        return IntRect{r.left, r.top, r.width, r.height};
    }

    // KeyCode ⇄ sf::Keyboard::Key
    sf::Keyboard::Key toSFML(KeyCode key);
    KeyCode fromSFML(sf::Keyboard::Key key);

    // BlendMode ⇄ sf::BlendMode
    sf::BlendMode toSFML(BlendMode mode);

    // InputEvent ⇄ sf::Event
    bool fromSFML(const sf::Event& sfEvent, InputEvent& novaEvent);
}
```

### Exemple d'implémentation : drawSprite

```cpp
void SFMLGraphicsBackend::drawSprite(const SpriteData& data) {
    // Récupérer la texture
    auto it = m_textures.find(data.texture);
    if (it == m_textures.end()) {
        LOG_ERROR("Invalid texture handle: {}", data.texture);
        return;
    }

    // Créer sprite SFML
    sf::Sprite sprite;
    sprite.setTexture(it->second);

    // Appliquer transformations
    sprite.setPosition(SFMLConversions::toSFML(data.position));
    sprite.setRotation(data.rotation);
    sprite.setScale(SFMLConversions::toSFML(data.scale));
    sprite.setOrigin(SFMLConversions::toSFML(data.origin));

    // Texture rect
    if (data.textureRect.width > 0 && data.textureRect.height > 0) {
        sprite.setTextureRect(SFMLConversions::toSFML(data.textureRect));
    }

    // Couleur/tint
    sprite.setColor(SFMLConversions::toSFML(data.color));

    // Blend mode
    sf::RenderStates states;
    states.blendMode = SFMLConversions::toSFML(data.blendMode);

    // Shader (optionnel)
    if (data.shader != INVALID_HANDLE) {
        auto shaderIt = m_shaders.find(data.shader);
        if (shaderIt != m_shaders.end()) {
            states.shader = &shaderIt->second;
        }
    }

    // Dessiner
    m_window->draw(sprite, states);
}
```

---

## Ajout d'un nouveau backend (SDL, Vulkan, etc.)

### Étapes

1. **Créer dossier** : `sdk/include/NovaEngine/Backend/SDL/`

2. **Implémenter interfaces** :
   ```cpp
   class SDLGraphicsBackend : public IGraphicsBackend { /* ... */ };
   class SDLWindowBackend : public IWindowBackend { /* ... */ };
   class SDLInputBackend : public IInputBackend { /* ... */ };
   // ... etc
   ```

3. **Ajouter à BackendManager** :
   ```cpp
   bool BackendManager::createBackends(BackendType type) {
       switch (type) {
           case BackendType::SFML:
               // ... code SFML existant
               break;

           case BackendType::SDL:
               m_window = std::make_unique<SDLWindowBackend>();
               m_input = std::make_unique<SDLInputBackend>();
               m_graphics = std::make_unique<SDLGraphicsBackend>(/* ... */);
               // ... autres backends
               break;

           // ...
       }
   }
   ```

4. **Tester** :
   ```cpp
   BACKEND().initialize(BackendType::SDL, 1920, 1080, "Test SDL");
   ```

---

## Avantages de cette architecture

### 1. Portabilité

```cpp
// Changer de backend = une seule ligne
#ifdef USE_SDL
    BACKEND().initialize(BackendType::SDL, ...);
#else
    BACKEND().initialize(BackendType::SFML, ...);
#endif
```

### 2. Testabilité

```cpp
// Mock backend pour tests unitaires
class MockGraphicsBackend : public IGraphicsBackend {
    void drawSprite(const SpriteData& data) override {
        m_spritesDrawn.push_back(data);  // Enregistre au lieu de dessiner
    }

    std::vector<SpriteData> m_spritesDrawn;
};

// Test
MockGraphicsBackend mockBackend;
RenderSystem system;
system.render(entities, &mockBackend);
ASSERT_EQ(mockBackend.m_spritesDrawn.size(), 10);
```

### 3. Séparation des préoccupations

```cpp
// Le jeu ne connaît que les types NovaEngine
Vec2f position{100, 200};
Color color{255, 0, 0};

// Pas de dépendance à SFML dans le code du jeu
// Pas d'include <SFML/...> dans les fichiers de jeu
```

### 4. Hot-swapping (avancé)

```cpp
// Changer de backend au runtime (possible mais complexe)
BACKEND().shutdown();
BACKEND().initialize(BackendType::SDL, ...);
// Recharger ressources...
```

---

## Bonnes pratiques

### ✅ Utiliser les macros

```cpp
// BON
GRAPHICS().drawSprite(spriteData);

// ÉVITER (trop verbeux)
BackendManager::get().graphics().drawSprite(spriteData);
```

### ✅ Toujours vérifier INVALID_HANDLE

```cpp
TextureHandle tex = RESOURCES().loadTexture("path.png");
if (tex == INVALID_HANDLE) {
    LOG_ERROR("Failed to load texture");
    return;
}

// Utiliser texture...
```

### ✅ Utiliser les types NovaEngine partout

```cpp
// BON
Vec2f position{100, 200};
sprite->position = position;

// ÉVITER (dépendance SFML dans game code)
sf::Vector2f position(100, 200);  // NON!
```

### ❌ Ne jamais inclure SFML dans game code

```cpp
// MAUVAIS
#include <SFML/Graphics.hpp>  // NON! Uniquement dans implémentation backend

// BON
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
```

---

**Prochaine section** : [Entity Component System](04-ECS.md)
