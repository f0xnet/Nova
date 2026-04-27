#pragma once
#include "../Backend/Core/BackendTypes.hpp"
#include "../Backend/Interfaces/IGraphicsBackend.hpp"
#include "RenderTexturePool.hpp"

namespace NovaEngine {

/**
 * @brief Interface abstraite pour un effet de post-processing
 *
 * Chaque effet est un shader appliqué à la texture de rendu.
 * Permet de créer des effets modulaires : CRT, Bloom, Lighting, etc.
 */
class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;

    /**
     * @brief Initialise l'effet (charge le shader)
     * @param graphicsBackend Le backend graphique
     * @param width Largeur de la texture de rendu
     * @param height Hauteur de la texture de rendu
     * @return true si l'initialisation a réussi
     */
    virtual bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) = 0;

    /**
     * @brief Nettoie les ressources de l'effet
     */
    virtual void shutdown() = 0;

    /**
     * @brief Applique l'effet sur la render texture
     * @param inputTexture Handle de la texture à traiter
     * @param outputTexture Handle de la texture de sortie (INVALID_HANDLE = écran)
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    virtual void apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) = 0;

    /**
     * @brief Active ou désactive l'effet
     */
    virtual void setEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Vérifie si l'effet est activé
     */
    virtual bool isEnabled() const { return m_enabled; }

    /**
     * @brief Retourne le nom de l'effet
     */
    virtual const char* getName() const = 0;

    void setPool(RenderTexturePool* pool) { m_pool = pool; }

protected:
    bool m_enabled = true;
    IGraphicsBackend* m_graphicsBackend = nullptr;
    RenderTexturePool* m_pool = nullptr;
};

}
