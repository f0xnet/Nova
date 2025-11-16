#pragma once

#include "NovaEngine/Core/Application.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/UI/UIManager.hpp"
#include "NovaEngine/UI/UILoader.hpp"
#include "NovaEngine/Events/Event.hpp"
#include "NovaEngine/ECS/ECS.hpp"

// Forward declarations for game modules
namespace NovaEngine {
    class DialogueSystem;
    class PlayerController;
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
    static Config createConfig();
    void handleUIAction(const std::string& action,
                       const std::string& value,
                       const NovaEngine::ID& componentID);
    void toggleConnectionState();

    // Game methods
    void renderScene();
    void renderNPCIndicator();

public:
    bool isConnected() const;
};
