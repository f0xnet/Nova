# Système de Rendu - NovaEngine

Le système de rendu applique des effets de post-processing via un pipeline modulaire.

## PostProcessPipeline

**Rôle** : Chaîne d'effets appliqués séquentiellement

```cpp
class PostProcessPipeline {
public:
    PostProcessPipeline(IGraphicsBackend* backend);

    bool initialize(u32 width, u32 height);
    void shutdown();

    // Ajouter effet
    template<typename T>
    T* addEffect();

    // Récupérer effet
    template<typename T>
    T* getEffect();

    void clearEffects();

    // Render workflow
    void beginSceneRender();
    void endSceneRender(f32 deltaTime);

    void setEnabled(bool enabled);
    bool isEnabled() const;
};
```

**Utilisation** :
```cpp
// Initialisation
PostProcessPipeline pipeline(&GRAPHICS());
pipeline.initialize(1920, 1080);

// Ajouter effets
auto* ssao = pipeline.addEffect<SSAOEffect>();
ssao->setStrength(0.4f);

auto* bloom = pipeline.addEffect<BloomEffect>();
bloom->setIntensity(0.5f);

auto* lighting = pipeline.addEffect<DynamicLightingEffect>();
lighting->setEnabled(true);

// Game loop
pipeline.beginSceneRender();
  scene.render();  // Rendu vers texture
pipeline.endSceneRender(deltaTime);
  // Effets appliqués, résultat à l'écran

ui.render();  // UI rendue directement (pas d'effets)
```

## PostProcessEffect (base)

```cpp
class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;

    virtual bool initialize(IGraphicsBackend* backend, u32 width, u32 height) = 0;
    virtual void apply(RenderTextureHandle input, RenderTextureHandle output, f32 deltaTime) = 0;
    virtual void shutdown() = 0;

    void setEnabled(bool enabled);
    bool isEnabled() const;

protected:
    IGraphicsBackend* m_graphicsBackend;
    u32 m_width, m_height;
    bool m_enabled;
};
```

## Effets built-in

### 1. SSAOEffect

**Screen Space Ambient Occlusion** - Ajoute ombres ambiantes

```cpp
class SSAOEffect : public PostProcessEffect {
public:
    void setStrength(f32 strength);   // 0.0-1.0
    void setRadius(f32 radius);       // Pixels
    void setSamples(u32 samples);     // Qualité

    f32 getStrength() const;
    f32 getRadius() const;
};
```

**Shader** : `ssao.frag`
- Échantillonne autour de chaque pixel
- Détecte bords proches
- Assombrit proportionnellement à proximité

### 2. BloomEffect

**Glow sur zones lumineuses**

```cpp
class BloomEffect : public PostProcessEffect {
public:
    void setIntensity(f32 intensity);      // Force du glow
    void setThreshold(f32 threshold);      // Seuil luminosité (0.6)
    void setBlurPasses(u32 passes);        // Qualité blur

    f32 getIntensity() const;
};
```

**Algorithme** :
1. Extraire pixels lumineux (brightness > threshold)
2. Appliquer blur gaussien
3. Additionner avec image originale

**Shader** : `bloom.frag` + `blur.frag`

### 3. ColorGradingEffect

**Correction colorimétrique**

```cpp
class ColorGradingEffect : public PostProcessEffect {
public:
    void setSaturation(f32 saturation);     // 0.0-2.0 (1.0=normal)
    void setContrast(f32 contrast);         // 0.0-2.0 (1.0=normal)
    void setBrightness(f32 brightness);     // -1.0-1.0 (0=normal)

    f32 getSaturation() const;
    f32 getContrast() const;
    f32 getBrightness() const;
};
```

**Shader** : `color_grading.frag`

### 4. DynamicLightingEffect

**Éclairage dynamique multi-lumières avec cycle jour/nuit**

```cpp
#define MAX_LIGHTS 8

class DynamicLightingEffect : public PostProcessEffect {
public:
    void setTimeOfDay(f32 time);          // 0.0-1.0 (0=minuit, 0.5=midi)
    void setAmbientDarkness(f32 darkness); // 0.0-1.0
    void setCamera(const Vec2f& center, const Vec2f& size);

    // Ajouter lumières
    void clearLights();
    void addLight(const Vec2f& position, const Color& color,
                 f32 radius, f32 intensity,
                 LightComponent::LightType type,
                 const Vec2f& direction = Vec2f{0, 0},
                 f32 angle = 45.0f);

    f32 getTimeOfDay() const;
};
```

**Shader** : `dynamic_lighting.frag`
- **8 lumières max** simultanées
- **Types** : Point, Directional, Spot
- **Cycle jour/nuit** : Teinte + luminosité ambiante
- **Conversion world → screen** pour positions lumières

**Périodes** :
- Nuit : 0.0-0.2, 0.8-1.0 (teinte bleue, sombre)
- Aube : 0.2-0.3 (teinte orange)
- Jour : 0.3-0.7 (teinte neutre, lumineux)
- Crépuscule : 0.7-0.8 (teinte rouge-orange)

### 5. CRTEffect

**Simulation écran cathodique** (actuellement non utilisé, mais disponible)

```cpp
class CRTEffect : public PostProcessEffect {
public:
    void setScanlineIntensity(f32 intensity);
    void setCurvature(f32 curvature);
    void setChromaticAberration(f32 aberration);
    void setVignetteStrength(f32 strength);
    // ... etc
};
```

**Effets** : Scanlines, courbure écran, aberration chromatique, vignette, bruit

## Pipeline workflow

```
┌─────────────────────────────────────┐
│  1. beginSceneRender()              │
│     → Bind RenderTexture (main)     │
│     → Clear                          │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  2. scene.render()                  │
│     → RenderSystem dessine sprites  │
│     → Output vers RenderTexture     │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  3. endSceneRender(deltaTime)       │
│                                     │
│  Pour chaque effet enabled:         │
│    - effect.apply(input, output)    │
│    - Swap input/output (ping-pong)  │
│                                     │
│  Ordre:                             │
│    1. SSAOEffect                    │
│    2. BloomEffect                   │
│    3. ColorGradingEffect            │
│    4. DynamicLightingEffect         │
│                                     │
│  → Résultat final à l'écran         │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  4. ui.render()                     │
│     → Rendu direct (pas d'effets)   │
└─────────────────────────────────────┘
```

## Ping-Pong Rendering

```cpp
RenderTextureHandle input = m_renderTexture;
RenderTextureHandle output = m_tempTexture;

for (auto& effect : m_effects) {
    if (effect->isEnabled()) {
        effect->apply(input, output, deltaTime);

        // Swap pour prochain effet
        std::swap(input, output);
    }
}

// Dernier output contient résultat final
drawFinalToScreen(input);
```

**Pourquoi ?** Évite de copier textures inutilement. Chaque effet lit l'output du précédent.

---

**Prochaine section** : [Shaders](08-SHADERS.md)
