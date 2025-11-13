#include "NovaEngine/Backend/BackendManager.hpp"
#include "NovaEngine/Backend/SFML/SFMLWindowBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLInputBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLResourceBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLAudioBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLFontBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLViewportBackend.hpp"
#include <stdexcept>

namespace NovaEngine {
BackendManager::BackendManager() : m_initialized(false), m_currentBackend(BackendType::SFML) {}
BackendManager::~BackendManager() { if(m_initialized) shutdown(); }

BackendManager& BackendManager::get() {
    static BackendManager instance;
    return instance;
}

bool BackendManager::initialize(BackendType type, u32 windowWidth, u32 windowHeight,
                                const String& windowTitle, bool fullscreen) {
    if(m_initialized) return true;
    m_currentBackend = type;
    
    if(!createBackends(type)) return false;
    
    if(!m_window->create(windowWidth, windowHeight, windowTitle, fullscreen)) {
        destroyBackends();
        return false;
    }
    
    if(type == BackendType::SFML) {
        auto* sfmlWindow = dynamic_cast<SFMLWindowBackend*>(m_window.get());
        if(sfmlWindow) {
            auto* sfmlInput = dynamic_cast<SFMLInputBackend*>(m_input.get());
            auto* sfmlViewport = dynamic_cast<SFMLViewportBackend*>(m_viewport.get());
            if(sfmlInput) sfmlInput->setWindow(sfmlWindow->getSFMLWindow());
            if(sfmlViewport) sfmlViewport->setWindow(sfmlWindow->getSFMLWindow());
        }
    }
    
    void* windowHandle = m_window.get();  // ✅ Passe SFMLWindowBackend* directement
   if(!m_graphics->initialize(windowHandle)) {

        destroyBackends();
        return false;
    }
    
    if(!m_audio->initialize()) {
        destroyBackends();
        return false;
    }
    
    m_initialized = true;
    return true;
}

void BackendManager::shutdown() {
    if(!m_initialized) return;
    if(m_resources) m_resources->clearCache();
    if(m_audio) { m_audio->stopAllSounds(); m_audio->stopMusic(); m_audio->shutdown(); }
    if(m_graphics) m_graphics->shutdown();
    if(m_window) m_window->close();
    destroyBackends();
    m_initialized = false;
}

bool BackendManager::createBackends(BackendType type) {
    try {
        if(type == BackendType::SFML) {
            auto window = std::make_unique<SFMLWindowBackend>();
            auto input = std::make_unique<SFMLInputBackend>();
            auto graphics = std::make_unique<SFMLGraphicsBackend>();
            auto resources = std::make_unique<SFMLResourceBackend>();
            auto audio = std::make_unique<SFMLAudioBackend>();
            auto fonts = std::make_unique<SFMLFontBackend>();
            auto viewport = std::make_unique<SFMLViewportBackend>();
            
            resources->setGraphicsBackend(graphics.get());
            audio->setResourceBackend(resources.get());
            fonts->setResourceBackend(resources.get());
            fonts->setGraphicsBackend(graphics.get());
            
            m_window = std::move(window);
            m_input = std::move(input);
            m_graphics = std::move(graphics);
            m_resources = std::move(resources);
            m_audio = std::move(audio);
            m_fonts = std::move(fonts);
            m_viewport = std::move(viewport);
            return true;
        }
        return false;
    } catch(...) {
        destroyBackends();
        return false;
    }
}

void BackendManager::destroyBackends() {
    m_viewport.reset(); 
    m_fonts.reset(); 
    m_audio.reset(); 
    m_resources.reset();
    m_graphics.reset(); 
    m_input.reset(); 
    m_window.reset();
}

IWindowBackend& BackendManager::window() {
    if(!m_window) throw std::runtime_error("Window backend not initialized");
    return *m_window;
}
IInputBackend& BackendManager::input() {
    if(!m_input) throw std::runtime_error("Input backend not initialized");
    return *m_input;
}
IGraphicsBackend& BackendManager::graphics() {
    if(!m_graphics) throw std::runtime_error("Graphics backend not initialized");
    return *m_graphics;
}
IResourceBackend& BackendManager::resources() {
    if(!m_resources) throw std::runtime_error("Resource backend not initialized");
    return *m_resources;
}
IAudioBackend& BackendManager::audio() {
    if(!m_audio) throw std::runtime_error("Audio backend not initialized");
    return *m_audio;
}
IFontBackend& BackendManager::fonts() {
    if(!m_fonts) throw std::runtime_error("Font backend not initialized");
    return *m_fonts;
}
IViewportBackend& BackendManager::viewport() {
    if(!m_viewport) throw std::runtime_error("Viewport backend not initialized");
    return *m_viewport;
}

const IWindowBackend& BackendManager::window() const {
    if(!m_window) throw std::runtime_error("Window backend not initialized");
    return *m_window;
}
const IInputBackend& BackendManager::input() const {
    if(!m_input) throw std::runtime_error("Input backend not initialized");
    return *m_input;
}
const IGraphicsBackend& BackendManager::graphics() const {
    if(!m_graphics) throw std::runtime_error("Graphics backend not initialized");
    return *m_graphics;
}
const IResourceBackend& BackendManager::resources() const {
    if(!m_resources) throw std::runtime_error("Resource backend not initialized");
    return *m_resources;
}
const IAudioBackend& BackendManager::audio() const {
    if(!m_audio) throw std::runtime_error("Audio backend not initialized");
    return *m_audio;
}
const IFontBackend& BackendManager::fonts() const {
    if(!m_fonts) throw std::runtime_error("Font backend not initialized");
    return *m_fonts;
}
const IViewportBackend& BackendManager::viewport() const {
    if(!m_viewport) throw std::runtime_error("Viewport backend not initialized");
    return *m_viewport;
}

void BackendManager::reconnectWindow() {
    if(!m_initialized || !m_window) return;
    if(m_currentBackend == BackendType::SFML) {
        auto* sfmlWindow = dynamic_cast<SFMLWindowBackend*>(m_window.get());
        if(sfmlWindow) {
            auto* sfmlInput = dynamic_cast<SFMLInputBackend*>(m_input.get());
            auto* sfmlViewport = dynamic_cast<SFMLViewportBackend*>(m_viewport.get());
            if(sfmlInput) sfmlInput->setWindow(sfmlWindow->getSFMLWindow());
            if(sfmlViewport) sfmlViewport->setWindow(sfmlWindow->getSFMLWindow());
        }
    }
}
}
