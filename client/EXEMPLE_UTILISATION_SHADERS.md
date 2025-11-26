# Système Modulaire de Post-Processing

## Architecture

Le nouveau système est basé sur 3 composants :

1. **PostProcessPipeline** : Gère la chaîne de rendu
2. **PostProcessEffect** : Interface abstraite pour les effets
3. **Effets concrets** : CRTEffect, BloomEffect, etc.

## Utilisation Simple

### Dans Game.hpp
```cpp
#include "NovaEngine/Rendering/PostProcessPipeline.hpp"

class Game : public NovaEngine::Application {
private:
    std::unique_ptr<NovaEngine::PostProcessPipeline> m_postProcessing;
};
```

### Dans Game.cpp - Initialisation

```cpp
#include "NovaEngine/Rendering/Effects/CRTEffect.hpp"
#include "NovaEngine/Rendering/Effects/PassthroughEffect.hpp"

bool Game::onInitialize() {
    // ... initialisation normale ...

    // Créer le pipeline de post-processing
    m_postProcessing = std::make_unique<NovaEngine::PostProcessPipeline>(&GRAPHICS());

    if(!m_postProcessing->initialize(logicalWidth, logicalHeight)) {
        LOG_WARN("Failed to initialize post-processing");
        m_postProcessing.reset();
    } else {
        // Ajouter l'effet CRT
        auto* crt = m_postProcessing->addEffect<NovaEngine::CRTEffect>();

        // Optionnel : ajuster les paramètres
        crt->setScanlineIntensity(0.2f);
        crt->setCurvature(0.1f);

        LOG_INFO("Post-processing initialized with CRT effect");
    }

    return true;
}
```

### Dans Game.cpp - Rendu

```cpp
void Game::onRender() {
    // Commencer le rendu de la scène avec post-processing
    if(m_postProcessing) {
        m_postProcessing->beginSceneRender();
    }

    // Rendu de la scène (sera capturé dans la texture)
    m_sceneManager.render();

    // Terminer et appliquer les effets
    if(m_postProcessing) {
        m_postProcessing->endSceneRender(0.016f); // ~60 FPS
    }

    // UI rendu directement (sans shader)
    m_uiManager.render();
}
```

## Ajouter Plusieurs Effets

```cpp
// Dans onInitialize()
m_postProcessing->addEffect<BloomEffect>();
m_postProcessing->addEffect<CRTEffect>();
m_postProcessing->addEffect<VignetteEffect>();

// Les effets sont appliqués dans l'ordre :
// Scène -> Bloom -> CRT -> Vignette -> Écran
```

## Contrôler les Effets en Temps Réel

```cpp
// Récupérer un effet
auto* crt = m_postProcessing->getEffect<CRTEffect>();
if(crt) {
    // Activer/désactiver
    crt->setEnabled(false);

    // Modifier les paramètres
    crt->setScanlineIntensity(0.5f);
}

// Désactiver tout le post-processing
m_postProcessing->setEnabled(false);
```

## Créer un Nouvel Effet

### 1. Créer le header (MyEffect.hpp)

```cpp
#pragma once
#include "NovaEngine/Rendering/PostProcessEffect.hpp"

namespace NovaEngine {

class MyEffect : public PostProcessEffect {
public:
    bool initialize(IGraphicsBackend* backend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle renderTexture, f32 deltaTime) override;
    const char* getName() const override { return "MyEffect"; }

private:
    ShaderHandle m_shader;
};

}
```

### 2. Implémenter (.cpp)

```cpp
bool MyEffect::initialize(IGraphicsBackend* backend, u32 width, u32 height) {
    m_graphicsBackend = backend;

    m_shader = backend->loadShader(
        "data/shaders/myeffect.vert",
        "data/shaders/myeffect.frag"
    );

    return m_shader != INVALID_HANDLE;
}

void MyEffect::apply(RenderTextureHandle renderTexture, f32 deltaTime) {
    // Envoyer les paramètres au shader
    m_graphicsBackend->setShaderParameter(m_shader, "time", deltaTime);

    // Dessiner avec le shader
    m_graphicsBackend->drawRenderTextureToScreen(renderTexture, m_shader);
}
```

### 3. Utiliser

```cpp
m_postProcessing->addEffect<MyEffect>();
```

## Avantages du Système

✅ **Modulaire** : Ajoute/retire des effets facilement
✅ **Extensible** : Crée de nouveaux effets sans toucher au moteur
✅ **Flexible** : Contrôle chaque effet individuellement
✅ **Performant** : Pas de copie de texture inutile
✅ **Propre** : Séparation scène / UI
✅ **Abstrait** : Aucune dépendance SFML hors du backend

## Exemples d'Effets Futurs

- **BloomEffect** : Effet de lumière/glow
- **MotionBlurEffect** : Flou de mouvement
- **ColorGradingEffect** : Correction des couleurs
- **VignetteEffect** : Assombrissement des bords
- **FilmGrainEffect** : Grain de film
- **DepthOfFieldEffect** : Flou de profondeur
- **LightingEffect** : Éclairage dynamique 2D
