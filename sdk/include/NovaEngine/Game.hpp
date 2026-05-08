#pragma once

#include "NovaEngine/Core/Application.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/UI/UIManager.hpp"
#include "NovaEngine/UI/UILoader.hpp"
#include "NovaEngine/Events/Event.hpp"
#include "NovaEngine/ECS/ECS.hpp"
#include "NovaEngine/Rendering/PostProcessPipeline.hpp"
#include "NovaEngine/Systems/LightingSystem.hpp"

// Forward declarations — Game.hpp ne stocke que des pointeurs vers ces types,
// les includes complets sont dans Game.cpp uniquement.
namespace NovaEngine {
    class DialogueSystem;
    class PlayerController;
    class ScriptSystem;
    class SSAOEffect;
    class BloomEffect;
    class ColorGradingEffect;
    class DynamicLightingEffect;
}

class Game : public NovaEngine::Application {
private:
    NovaEngine::UIManager m_uiManager;
    NovaEngine::UILoader m_uiLoader;
    NovaEngine::SceneManager m_sceneManager;
    bool m_isConnected;

    // Game modules
    std::unique_ptr<NovaEngine::DialogueSystem> m_dialogueSystem;
    std::unique_ptr<NovaEngine::PlayerController> m_playerController;
    std::unique_ptr<NovaEngine::PostProcessPipeline> m_postProcessPipeline;
    std::unique_ptr<NovaEngine::LightingSystem> m_lightingSystem;

    // Post-processing effects (owned by pipeline)
    NovaEngine::SSAOEffect* m_ssaoEffect;
    NovaEngine::BloomEffect* m_bloomEffect;
    NovaEngine::ColorGradingEffect* m_colorGradingEffect;
    NovaEngine::DynamicLightingEffect* m_dynamicLightingEffect;

    // Scripting — global system, not tied to any specific scene
    std::unique_ptr<NovaEngine::ScriptSystem> m_scriptSystem;
    NovaEngine::Scene* m_lastActiveScene = nullptr;

    NovaEngine::FontHandle m_font;
    float m_lastDeltaTime = 0.016f;

public:
    Game();
    virtual ~Game();

protected:
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onEvent(const NovaEngine::Event& event) override;
    void onShutdown() override;

private:
    void handleUIAction(const std::string& action,
                       const std::string& value,
                       const NovaEngine::ID& componentID);
    void toggleConnectionState();

    // Helper methods
    Config createConfig();

public:
    bool isConnected() const;
};
