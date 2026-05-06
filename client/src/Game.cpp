#include "NovaEngine/Game.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"
#include "NovaEngine/Core/ConfigManager.hpp"
#include "NovaEngine/Scripting/Scripting.hpp"
#include "NovaEngine/UI/Components/Text.hpp"
#include "Dialogue/DialogueSystem.hpp"
#include "Player/PlayerController.hpp"

Game::Game()
    : Application(createConfig())
    , m_isConnected(false)
    , m_dialogueSystem(std::make_unique<NovaEngine::DialogueSystem>())
    , m_playerController(std::make_unique<NovaEngine::PlayerController>())
    , m_postProcessPipeline(nullptr)
    , m_lightingSystem(std::make_unique<NovaEngine::LightingSystem>())
    , m_ssaoEffect(nullptr)
    , m_bloomEffect(nullptr)
    , m_colorGradingEffect(nullptr)
    , m_dynamicLightingEffect(nullptr)
{
    LOG_TRACE("Game constructed");
}

Game::~Game() {
    LOG_TRACE("Game destroyed");
}

bool Game::onInitialize() {
    LOG_INFO("Initializing Game");

    // Configure viewport with letterboxing to preserve aspect ratio
    const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();
    float logicalWidth = static_cast<float>(displayConfig.nativeWidth);
    float logicalHeight = static_cast<float>(displayConfig.nativeHeight);
    float logicalAspectRatio = logicalWidth / logicalHeight;

    // Get actual window size
    NovaEngine::u32 windowWidth = WINDOW().getWidth();
    NovaEngine::u32 windowHeight = WINDOW().getHeight();
    float windowAspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Calculate viewport with letterboxing/pillarboxing
    NovaEngine::Rect viewportRect;
    if (windowAspectRatio > logicalAspectRatio) {
        // Pillarbox (vertical bars on sides)
        float viewportWidth = logicalAspectRatio / windowAspectRatio;
        float offsetX = (1.0f - viewportWidth) / 2.0f;
        viewportRect = NovaEngine::Rect(offsetX, 0.0f, viewportWidth, 1.0f);
    } else {
        // Letterbox (horizontal bars top/bottom)
        float viewportHeight = windowAspectRatio / logicalAspectRatio;
        float offsetY = (1.0f - viewportHeight) / 2.0f;
        viewportRect = NovaEngine::Rect(0.0f, offsetY, 1.0f, viewportHeight);
    }

    NovaEngine::ViewportData viewData;
    viewData.size = NovaEngine::Vec2f(logicalWidth, logicalHeight);
    viewData.center = NovaEngine::Vec2f(logicalWidth / 2.0f, logicalHeight / 2.0f);
    viewData.viewport = viewportRect;
    viewData.rotation = 0.0f;
    VIEWPORT().setView(viewData);

    LOG_INFO("Viewport: {}x{} logical, letterboxed on {}x{} window",
             logicalWidth, logicalHeight, windowWidth, windowHeight);

    // Initialize ECS Scene Manager
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Failed to initialize SceneManager");
        return false;
    }

    // Load game scene
    if (!m_sceneManager.loadScene("data/scenes/test.json", "test")) {
        LOG_ERROR("Failed to load test scene");
        return false;
    }
    m_sceneManager.setActiveScene("test");
    LOG_INFO("Test scene loaded and activated");

    // Player is defined in the scene JSON (test.json) with type "player"
    // Find the entity with tag "player"
    NovaEngine::Scene* scene = m_sceneManager.getActiveScene();
    if (scene) {
        using namespace NovaEngine;

        // Add scripting system to the scene — passe le SceneManager pour
        // que les scripts puissent changer de scène via Scene.setActive(...)
        m_scriptSystem = scene->addSystem<ScriptSystem>(&m_sceneManager);

        // Expose DialogueSystem to Lua :
        //   Dialogue.start(entityId)  — démarre un dialogue avec un NPC
        //   Dialogue.advance()        — avance le dialogue
        //   Dialogue.isActive()       — vrai si dialogue en cours
        //   Dialogue.reset()          — ferme le dialogue
        {
            auto& lua = m_scriptSystem->getLua();
            sol::table dlg = lua.create_table();
            dlg["start"] = [this](u64 entityId) {
                auto* sc = m_sceneManager.getActiveScene();
                if (!sc) return;
                auto* e = sc->getEntityRegistry().getEntity(entityId);
                if (e) m_dialogueSystem->startDialogue(e);
            };
            dlg["advance"]  = [this]() { m_dialogueSystem->advanceDialogue(); };
            dlg["isActive"] = [this]() -> bool { return m_dialogueSystem->isActive(); };
            dlg["reset"]    = [this]() { m_dialogueSystem->reset(); };
            lua["Dialogue"] = dlg;
        }

        // --- PostFX : effets visuels scriptables ---
        // PostFX.setBloom(0.8) / setSaturation(1.5) / setContrast / setBrightness
        // PostFX.setSSAO(enabled) / setLighting(enabled) / setAmbientDark(0.2)
        {
            auto& lua = m_scriptSystem->getLua();
            sol::table pfx = lua.create_table();
            pfx["setBloom"]      = [this](float v) {
                if (m_bloomEffect) m_bloomEffect->setIntensity(v);
            };
            pfx["setSaturation"] = [this](float v) {
                if (m_colorGradingEffect) m_colorGradingEffect->setSaturation(v);
            };
            pfx["setContrast"]   = [this](float v) {
                if (m_colorGradingEffect) m_colorGradingEffect->setContrast(v);
            };
            pfx["setBrightness"] = [this](float v) {
                if (m_colorGradingEffect) m_colorGradingEffect->setBrightness(v);
            };
            pfx["setSSAO"]       = [this](bool en) {
                if (m_ssaoEffect) m_ssaoEffect->setEnabled(en);
            };
            pfx["setLighting"]   = [this](bool en) {
                if (m_dynamicLightingEffect) m_dynamicLightingEffect->setEnabled(en);
            };
            pfx["setAmbientDark"] = [this](float v) {
                if (m_dynamicLightingEffect) m_dynamicLightingEffect->setAmbientDarkness(v);
            };
            lua["PostFX"] = pfx;
        }

        // --- UI : contrôle de l'interface depuis les scripts ---
        // UI.showGroup("hud") / hideGroup("dialogue")
        // UI.setText("label_id", "Score: 42")
        // UI.setVisible("component_id", false)
        // UI.setEnabled("button_id", false)
        {
            auto& lua = m_scriptSystem->getLua();
            sol::table ui = lua.create_table();
            ui["showGroup"]  = [this](const std::string& id) {
                m_uiManager.setGroupActive(id, true);
            };
            ui["hideGroup"]  = [this](const std::string& id) {
                m_uiManager.setGroupActive(id, false);
            };
            ui["setUIActive"] = [this](const std::string& id, bool active) {
                m_uiManager.setUIActive(id, active);
            };
            ui["setVisible"] = [this](const std::string& id, bool visible) {
                auto comp = m_uiManager.getComponent(id);
                if (comp) comp->setVisible(visible);
            };
            ui["setEnabled"] = [this](const std::string& id, bool enabled) {
                auto comp = m_uiManager.getComponent(id);
                if (comp) comp->setEnabled(enabled);
            };
            ui["setText"] = [this](const std::string& id, const std::string& text) {
                auto comp = m_uiManager.getComponent(id);
                if (!comp) return;
                auto* txt = dynamic_cast<NovaEngine::Text*>(comp.get());
                if (txt) txt->setString(text);
            };
            lua["UI"] = ui;
        }

        auto entities = scene->getEntityRegistry().getAllEntities();
        bool playerFound = false;
        Entity* playerEntity = nullptr;
        for (auto* entity : entities) {
            auto* tag = entity->getComponent<TagComponent>();
            if (tag && tag->tag == "player") {
                m_playerController->setPlayerID(entity->getID());
                playerEntity = entity;
                LOG_INFO("Player found and set to entity ID: {}", entity->getID());
                playerFound = true;
                break;
            }
        }
        if (!playerFound) {
            LOG_ERROR("No player entity found in scene! Make sure scene JSON has an entity with type=\"player\"");
        }

        // Attach the movement script to the player entity
        if (playerEntity) {
            auto sc = std::make_unique<ScriptComponent>();
            sc->scriptPath = "data/scripts/player.lua";
            playerEntity->addComponent(std::move(sc));
            scene->getEntityRegistry().invalidateQueryCache();
            LOG_INFO("Player script attached: data/scripts/player.lua");
        }
    }

    // Initialize UI system
    m_uiManager.setActionCallback([this](const std::string& action,
                                         const std::string& value,
                                         const NovaEngine::ID& componentID) {
        handleUIAction(action, value, componentID);
    });

    // Load dialogue UI
    if (!m_uiLoader.loadFromFile("data/ui/json/dialogue.json", m_uiManager)) {
        LOG_WARN("Failed to load dialogue UI");
    }

    // Initialize dialogue system with UI manager
    m_dialogueSystem->initialize(&m_uiManager);

    // Initialize post-processing pipeline
    m_postProcessPipeline = std::make_unique<NovaEngine::PostProcessPipeline>(&GRAPHICS());
    if (!m_postProcessPipeline->initialize(
        static_cast<NovaEngine::u32>(logicalWidth),
        static_cast<NovaEngine::u32>(logicalHeight))) {
        LOG_WARN("Failed to initialize PostProcessPipeline");
        m_postProcessPipeline.reset();
    } else {
        // Add post-processing effects in order
        // Each effect can be enabled/disabled independently

        // 1. SSAO - Ambient Occlusion (applied first, single-pass)
        m_ssaoEffect = m_postProcessPipeline->addEffect<NovaEngine::SSAOEffect>();
        if (m_ssaoEffect) {
            // COMMENTED FOR TESTING: Let constructor values take effect
            // m_ssaoEffect->setStrength(0.4f);
            // m_ssaoEffect->setRadius(12.0f);
            LOG_INFO("SSAO effect added successfully");
        }

        // 2. Bloom - Glow effect
        m_bloomEffect = m_postProcessPipeline->addEffect<NovaEngine::BloomEffect>();
        if (m_bloomEffect) {
            m_bloomEffect->setIntensity(0.4f);
            LOG_INFO("Bloom effect added successfully");
        }

        // 3. Color Grading - Saturation, contrast, etc.
        m_colorGradingEffect = m_postProcessPipeline->addEffect<NovaEngine::ColorGradingEffect>();
        if (m_colorGradingEffect) {
            m_colorGradingEffect->setSaturation(1.3f);
            m_colorGradingEffect->setContrast(1.0f);
            m_colorGradingEffect->setBrightness(0.0f);
            LOG_INFO("Color grading effect added successfully");
        }

        // 4. Dynamic Lighting - Multi-light system with colors (ECS-driven)
        m_dynamicLightingEffect = m_postProcessPipeline->addEffect<NovaEngine::DynamicLightingEffect>();
        if (m_dynamicLightingEffect) {
            m_dynamicLightingEffect->setEnabled(false);  // Désactivé par défaut
            m_dynamicLightingEffect->setAmbientDarkness(0.2f);  // Obscurité ambiante visible

            // Configure lighting system to manage lights from ECS
            m_lightingSystem->setLightingEffect(m_dynamicLightingEffect);

            // Démarrer à 6h du matin (0.25 = 6h, car 0.0 = minuit, 0.5 = midi)
            m_lightingSystem->setTimeOfDay(0.25f);

            LOG_INFO("Dynamic lighting effect added successfully (ECS-driven)");
            LOG_INFO("Time of day initialized to 6:00 AM");
        }
    }

    LOG_INFO("Game initialized successfully");
    LOG_INFO("=== Controls ===");
    LOG_INFO("  WASD / Arrow Keys - Move");
    LOG_INFO("  E - Talk to NPCs / Advance dialogue");
    LOG_INFO("  T - Advance time by 1 hour");
    LOG_INFO("  1 - Toggle SSAO");
    LOG_INFO("  2 - Toggle Bloom");
    LOG_INFO("  3 - Toggle Color Grading");
    LOG_INFO("  4 - Toggle Dynamic Lighting");
    LOG_INFO("  ESC - Quit");

    return true;
}

void Game::onUpdate(float deltaTime) {
    using namespace NovaEngine;

    Scene* scene = m_sceneManager.getActiveScene();
    if (scene) {
        // Expose dialogue state so Lua scripts can block movement during dialogues
        if (m_scriptSystem) {
            m_scriptSystem->getLua()["dialogueActive"] = m_dialogueSystem->isActive();
        }

        // Update NPC detection (stays in C++)
        m_playerController->updateNPCDetection(scene);

        // Update camera to follow player
        Vec2f playerPos = m_playerController->getPlayerPosition(scene);
        VIEWPORT().setViewCenter(playerPos);

        // Update dynamic lighting system with ECS lights
        if (m_dynamicLightingEffect && m_lightingSystem) {
            // Update camera for world → screen coordinate conversion
            const auto& viewportData = VIEWPORT().getView();
            m_dynamicLightingEffect->setCamera(viewportData.center, viewportData.size);

            // Collect and update all lights from entities
            m_lightingSystem->update(deltaTime, scene->getEntityRegistry());
        }

        // Show/hide NPC indicator
        Entity* nearestNPC = m_playerController->getNearestNPC();
        m_dialogueSystem->showNPCIndicator(nearestNPC != nullptr && !m_dialogueSystem->isActive());
    }

    // Update ECS scene
    m_sceneManager.update(deltaTime);

    // Update UI
    m_uiManager.update(deltaTime);
}

void Game::onRender() {
    // Begin post-processing (render scene to texture)
    if (m_postProcessPipeline) {
        m_postProcessPipeline->beginSceneRender();
    }

    // Render ECS scene (to texture if pipeline enabled)
    m_sceneManager.render();

    // End scene rendering and apply post-processing effects
    if (m_postProcessPipeline) {
        m_postProcessPipeline->endSceneRender(0.016f); // ~60fps delta
    }

    // Reset view to default before rendering UI (avoid camera offset)
    VIEWPORT().resetView();

    // Render UI directly to screen (no shader applied)
    m_uiManager.render();
}

void Game::onEvent(const NovaEngine::Event& event) {
    using namespace NovaEngine;

    // Dispatch to UI first
    m_uiManager.dispatchEvent(event);

    // Handle game-specific input
    if (event.type == EventType::Input &&
        event.inputEvent.type == InputEventType::KeyPressed) {

        if (event.inputEvent.key.code == KeyCode::Escape) {
            quit();
        }
        else if (event.inputEvent.key.code == KeyCode::E) {
            // Handle dialogue interaction
            if (m_dialogueSystem->isActive()) {
                m_dialogueSystem->advanceDialogue();
            } else {
                Entity* nearestNPC = m_playerController->getNearestNPC();
                if (nearestNPC) {
                    m_dialogueSystem->startDialogue(nearestNPC);
                }
            }
        }
        else if (event.inputEvent.key.code == KeyCode::Num1) {
            // Toggle SSAO effect
            if (m_ssaoEffect) {
                bool newState = !m_ssaoEffect->isEnabled();
                m_ssaoEffect->setEnabled(newState);
                LOG_INFO("SSAO effect {}", newState ? "enabled" : "disabled");
            }
        }
        else if (event.inputEvent.key.code == KeyCode::Num2) {
            // Toggle Bloom effect
            if (m_bloomEffect) {
                bool newState = !m_bloomEffect->isEnabled();
                m_bloomEffect->setEnabled(newState);
                LOG_INFO("Bloom effect {}", newState ? "enabled" : "disabled");
            }
        }
        else if (event.inputEvent.key.code == KeyCode::Num3) {
            // Toggle Color Grading effect
            if (m_colorGradingEffect) {
                bool newState = !m_colorGradingEffect->isEnabled();
                m_colorGradingEffect->setEnabled(newState);
                LOG_INFO("Color grading effect {}", newState ? "enabled" : "disabled");
            }
        }
        else if (event.inputEvent.key.code == KeyCode::Num4) {
            // Toggle Dynamic Lighting effect
            if (m_dynamicLightingEffect) {
                bool newState = !m_dynamicLightingEffect->isEnabled();
                m_dynamicLightingEffect->setEnabled(newState);
                LOG_INFO("Dynamic lighting effect {}", newState ? "enabled" : "disabled");
            }
        }
        else if (event.inputEvent.key.code == KeyCode::T) {
            // Advance time by 1 hour
            if (m_lightingSystem && m_dynamicLightingEffect) {
                // 1 hour = 1/24 of a full day (1.0)
                const float oneHour = 1.0f / 24.0f;
                float currentTime = m_lightingSystem->getTimeOfDay();
                float newTime = currentTime + oneHour;

                // Wrap around if we exceed 1.0 (midnight)
                if (newTime >= 1.0f) {
                    newTime -= 1.0f;
                }

                m_lightingSystem->setTimeOfDay(newTime);

                // Convert timeOfDay to hours (0.0 = 0h, 0.5 = 12h, 1.0 = 24h)
                int hours = static_cast<int>(newTime * 24.0f);
                LOG_INFO("Time advanced to {}:00 ({}h)", hours < 10 ? "0" + std::to_string(hours) : std::to_string(hours), hours);
            }
        }
    }
}

void Game::onShutdown() {
    LOG_INFO("Game shutting down");
    if (m_postProcessPipeline) {
        m_postProcessPipeline->shutdown();
        m_postProcessPipeline.reset();
    }
    // Effects are owned by pipeline, already deleted
    m_ssaoEffect = nullptr;
    m_bloomEffect = nullptr;
    m_colorGradingEffect = nullptr;
    m_sceneManager.shutdown();
}

Game::Config Game::createConfig() {
    Config config;

    if (!NovaEngine::ConfigManager::initializeGlobalConfig("config/engine.ini")) {
        LOG_WARN("Failed to load config file, using default values");

        config.windowTitle = "NovaEngine - RPG Game";
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.fullscreen = false;
        config.frameRateLimit = 60;
        config.vSync = true;
        config.clearColor = NovaEngine::Color(30, 30, 40);
    } else {
        const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();

        config.windowTitle = "NovaEngine - RPG Game";
        config.windowWidth = displayConfig.width;
        config.windowHeight = displayConfig.height;
        config.fullscreen = displayConfig.fullscreen;
        config.frameRateLimit = displayConfig.frameRateLimit;
        config.vSync = displayConfig.vsync;
        config.clearColor = NovaEngine::Color(30, 30, 40);

        LOG_INFO("Configuration loaded: {}x{}", displayConfig.width, displayConfig.height);
    }

    return config;
}

void Game::handleUIAction(const std::string& action,
                         const std::string& value,
                         const NovaEngine::ID& componentID)
{
    LOG_INFO("UI Action: '{}' (value: '{}', component: '{}')", action, value, componentID);

    if (action == "connect") {
        toggleConnectionState();
    }
}

void Game::toggleConnectionState() {
    m_isConnected = !m_isConnected;
    LOG_INFO("Connection state: {}", m_isConnected ? "Connected" : "Disconnected");
}

bool Game::isConnected() const {
    return m_isConnected;
}
