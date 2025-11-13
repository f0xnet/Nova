#pragma once
#include "Core/BackendTypes.hpp"
#include "Interfaces/IWindowBackend.hpp"
#include "Interfaces/IInputBackend.hpp"
#include "Interfaces/IGraphicsBackend.hpp"
#include "Interfaces/IResourceBackend.hpp"
#include "Interfaces/IAudioBackend.hpp"
#include "Interfaces/IFontBackend.hpp"
#include "Interfaces/IViewportBackend.hpp"
#include <memory>

namespace NovaEngine {
class BackendManager {
public:
    static BackendManager& get();
    BackendManager(const BackendManager&) = delete;
    BackendManager& operator=(const BackendManager&) = delete;
    
    bool initialize(BackendType type = BackendType::SFML, u32 windowWidth = 800, u32 windowHeight = 600,
                   const String& windowTitle = "NovaEngine", bool fullscreen = false);
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    BackendType getCurrentBackendType() const { return m_currentBackend; }
    void reconnectWindow();
    
    IWindowBackend& window();
    IInputBackend& input();
    IGraphicsBackend& graphics();
    IResourceBackend& resources();
    IAudioBackend& audio();
    IFontBackend& fonts();
    IViewportBackend& viewport();
    
    const IWindowBackend& window() const;
    const IInputBackend& input() const;
    const IGraphicsBackend& graphics() const;
    const IResourceBackend& resources() const;
    const IAudioBackend& audio() const;
    const IFontBackend& fonts() const;
    const IViewportBackend& viewport() const;
    
private:
    BackendManager();
    ~BackendManager();
    bool createBackends(BackendType type);
    void destroyBackends();
    
    bool m_initialized;
    BackendType m_currentBackend;
    std::unique_ptr<IWindowBackend> m_window;
    std::unique_ptr<IInputBackend> m_input;
    std::unique_ptr<IGraphicsBackend> m_graphics;
    std::unique_ptr<IResourceBackend> m_resources;
    std::unique_ptr<IAudioBackend> m_audio;
    std::unique_ptr<IFontBackend> m_fonts;
    std::unique_ptr<IViewportBackend> m_viewport;
};

#define BACKEND()   NovaEngine::BackendManager::get()
#define WINDOW()    NovaEngine::BackendManager::get().window()
#define INPUT()     NovaEngine::BackendManager::get().input()
#define GRAPHICS()  NovaEngine::BackendManager::get().graphics()
#define RESOURCES() NovaEngine::BackendManager::get().resources()
#define AUDIO()     NovaEngine::BackendManager::get().audio()
#define FONTS()     NovaEngine::BackendManager::get().fonts()
#define VIEWPORT()  NovaEngine::BackendManager::get().viewport()
}
