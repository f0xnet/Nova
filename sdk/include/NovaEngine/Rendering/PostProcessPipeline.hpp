#pragma once
#include "PostProcessEffect.hpp"
#include "RenderTexturePool.hpp"
#include <vector>
#include <memory>

namespace NovaEngine {

/**
 * @brief Pipeline de post-processing modulaire
 *
 * Gère une chaîne d'effets appliqués séquentiellement.
 * Permet de séparer le rendu de la scène et de l'UI.
 *
 * Utilisation:
 *   pipeline.addEffect<CRTEffect>();
 *   pipeline.addEffect<BloomEffect>();
 *
 *   pipeline.beginSceneRender();
 *   scene.render();
 *   pipeline.endSceneRender(deltaTime);
 *
 *   ui.render(); // Direct, sans shader
 */
class PostProcessPipeline {
public:
    PostProcessPipeline(IGraphicsBackend* graphicsBackend);
    ~PostProcessPipeline();

    /**
     * @brief Initialise le pipeline avec la résolution donnée
     */
    bool initialize(u32 width, u32 height);

    /**
     * @brief Nettoie le pipeline
     */
    void shutdown();

    /**
     * @brief Ajoute un effet au pipeline
     * @tparam T Type de l'effet (doit hériter de PostProcessEffect)
     * @return Pointeur vers l'effet créé (pour configuration)
     */
    template<typename T>
    T* addEffect() {
        static_assert(std::is_base_of<PostProcessEffect, T>::value,
                      "T must inherit from PostProcessEffect");

        auto effect = std::make_unique<T>();
        T* ptr = effect.get();
        ptr->setPool(&m_pool);
        m_effects.push_back(std::move(effect));

        // Initialiser l'effet immédiatement si le pipeline est déjà initialisé
        if(m_initialized) {
            ptr->initialize(m_graphicsBackend, m_width, m_height);
        }

        return ptr;
    }

    /**
     * @brief Récupère un effet par son type
     * @tparam T Type de l'effet
     * @return Pointeur vers l'effet ou nullptr
     */
    template<typename T>
    T* getEffect() {
        for(auto& effect : m_effects) {
            T* ptr = dynamic_cast<T*>(effect.get());
            if(ptr) return ptr;
        }
        return nullptr;
    }

    /**
     * @brief Retire tous les effets
     */
    void clearEffects();

    /**
     * @brief Commence le rendu de la scène (bind render texture)
     */
    void beginSceneRender();

    /**
     * @brief Termine le rendu de la scène et applique les effets
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    void endSceneRender(f32 deltaTime);

    /**
     * @brief Active/désactive le pipeline complet
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    bool hasAnyEffectEnabled() const;

    IGraphicsBackend* m_graphicsBackend;
    RenderTextureHandle m_renderTexture;      // Texture principale (scène)
    RenderTextureHandle m_tempTexture;        // Texture temporaire pour ping-pong
    RenderTexturePool m_pool;                 // Pool partagé entre les effets
    std::vector<std::unique_ptr<PostProcessEffect>> m_effects;

    u32 m_width;
    u32 m_height;
    bool m_initialized;
    bool m_enabled;
};

}
