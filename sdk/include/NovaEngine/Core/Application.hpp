#pragma once

#include "../Backend/BackendManager.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Events/Event.hpp"
#include "Logger.hpp"

#include <string>
#include <functional>

namespace NovaEngine {

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

protected:
    Config m_config;
    float m_deltaTime;
    bool m_initialized;
    f32 m_lastTime;

public:
    Application()
        : m_deltaTime(0.0f)
        , m_initialized(false)
        , m_lastTime(0.0f)
    {
        LOG_TRACE("Application base class constructed");
    }

    explicit Application(const Config& config)
        : m_config(config)
        , m_deltaTime(0.0f)
        , m_initialized(false)
        , m_lastTime(0.0f)
    {
        LOG_TRACE("Application constructed with custom config");
    }

    virtual ~Application() {
        LOG_TRACE("Application destroyed");
    }

    int run() {
        try {
            LOG_INFO("=== Application Starting ===");
            
            if (!initializeEngine()) {
                LOG_FATAL("Failed to initialize NovaEngine");
                return -1;
            }
            
            if (!onInitialize()) {
                LOG_FATAL("Failed to initialize application");
                return -1;
            }
            
            m_initialized = true;
            LOG_INFO("=== Application Initialized Successfully ===");
            
            runMainLoop();
            
            onShutdown();
            shutdownEngine();
            
            LOG_INFO("=== Application Exited Successfully ===");
            return 0;
            
        } catch (const std::exception& e) {
            LOG_FATAL("Exception in Application::run(): {}", e.what());
            return -1;
        }
    }

    void quit() {
        BACKEND().shutdown();
        LOG_INFO("Application quit requested");
    }

    float getDeltaTime() const {
        return m_deltaTime;
    }

    const Config& getConfig() const {
        return m_config;
    }

    bool isInitialized() const {
        return m_initialized;
    }

protected:
    virtual bool onInitialize() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;

    virtual void onEvent(const Event& event) {
        (void)event;
    }

    virtual void onShutdown() {
        LOG_DEBUG("Application shutdown (default implementation)");
    }

private:
    bool initializeEngine() {
        bool success = false;
        if (!m_config.configPath.empty()) {
            success = BACKEND().initialize(BackendType::SFML, m_config.windowWidth, m_config.windowHeight, m_config.windowTitle, m_config.fullscreen);
        } else {
            success = BACKEND().initialize(BackendType::SFML, m_config.windowWidth, m_config.windowHeight, m_config.windowTitle, m_config.fullscreen);
        }
        
        if (!success) {
            return false;
        }
        
        WINDOW().setVSync(m_config.vSync);
        if (m_config.frameRateLimit > 0) {
            WINDOW().setFramerateLimit(m_config.frameRateLimit);
        }
        
        LOG_INFO("Engine initialized: {}x{}, VSync={}, FPS limit={}",
                 m_config.windowWidth, m_config.windowHeight,
                 m_config.vSync, m_config.frameRateLimit);
        
        return true;
    }

    void runMainLoop() {
        LOG_DEBUG("Entering main loop");
        
        m_lastTime = 0.0f;
        
        while (WINDOW().isOpen()) {
            f32 currentTime = m_lastTime + 0.016f;
            m_deltaTime = currentTime - m_lastTime;
            m_lastTime = currentTime;
            
            processEvents();
            onUpdate(m_deltaTime);
            
            WINDOW().clear(m_config.clearColor);
            onRender();
            WINDOW().display();
        }
        
        LOG_DEBUG("Exited main loop");
    }

    void processEvents() {
        InputEvent inputEvent;
        
        while (INPUT().pollEvent(inputEvent)) {
            if (inputEvent.type == InputEventType::Closed) {
                quit();
                continue;
            }
            
            if (inputEvent.type == InputEventType::KeyPressed) {
                if (inputEvent.key.code == KeyCode::Escape) {
                    LOG_INFO("Escape pressed - quitting application");
                    quit();
                    continue;
                }
            }
            
            Event novaEvent(inputEvent);
            onEvent(novaEvent);
        }
    }

    void shutdownEngine() {
        BACKEND().shutdown();
        LOG_DEBUG("Engine shut down");
    }
};

}
