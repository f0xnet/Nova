#pragma once
#include "../Backend/Core/BackendTypes.hpp"
#include "../Backend/Interfaces/IGraphicsBackend.hpp"
#include <vector>

namespace NovaEngine {

/**
 * @brief Pool de RenderTextures partagées entre les effets de post-processing.
 *
 * Les effets s'exécutent séquentiellement : ils peuvent emprunter une texture,
 * l'utiliser pendant apply(), puis la rendre. Cela évite que chaque effet alloue
 * ses propres textures scratch permanentes et fragmente la mémoire GPU.
 *
 * Usage (dans un PostProcessEffect) :
 *   auto tex = m_pool->acquire(width, height);
 *   // ... utiliser tex ...
 *   m_pool->release(tex);
 */
class RenderTexturePool {
public:
    void initialize(IGraphicsBackend* backend) {
        m_backend = backend;
    }

    void shutdown() {
        if (!m_backend) return;
        for (auto& entry : m_entries) {
            if (entry.handle != INVALID_HANDLE)
                m_backend->unloadRenderTexture(entry.handle);
        }
        m_entries.clear();
    }

    // Emprunte une texture de la taille exacte demandée.
    // Crée une nouvelle entrée si aucune texture libre correspondante n'existe.
    RenderTextureHandle acquire(u32 width, u32 height) {
        if (!m_backend) return INVALID_HANDLE;
        for (auto& entry : m_entries) {
            if (!entry.inUse && entry.width == width && entry.height == height) {
                entry.inUse = true;
                return entry.handle;
            }
        }
        RenderTextureHandle h = m_backend->createRenderTexture(width, height);
        if (h != INVALID_HANDLE)
            m_entries.push_back({h, width, height, true});
        return h;
    }

    // Rend une texture empruntée au pool.
    void release(RenderTextureHandle handle) {
        for (auto& entry : m_entries) {
            if (entry.handle == handle) {
                entry.inUse = false;
                return;
            }
        }
    }

    size_t size() const { return m_entries.size(); }

private:
    struct Entry {
        RenderTextureHandle handle;
        u32 width, height;
        bool inUse = false;
    };
    IGraphicsBackend* m_backend = nullptr;
    std::vector<Entry> m_entries;
};

} // namespace NovaEngine
