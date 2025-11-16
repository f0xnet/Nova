#pragma once

#include "NovaEngine/Core/Application.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/UI/UIManager.hpp"
#include "NovaEngine/UI/UILoader.hpp"
#include "NovaEngine/Events/Event.hpp"
#include "NovaEngine/ECS/ECS.hpp"

class Game : public NovaEngine::Application {
private:
    NovaEngine::UIManager m_uiManager;
    NovaEngine::UILoader m_uiLoader;
    NovaEngine::SceneManager m_sceneManager;
    bool m_isConnected;

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

public:
    bool isConnected() const;
};
