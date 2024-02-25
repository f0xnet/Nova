#pragma once
#include "RenderClass.hpp"
#include "SystemClass.hpp"
#include "../../draw/headers/DrawClass.hpp"
#include "../../ui/headers/UIManager.hpp"
#include "../../network/headers/NetworkManager.hpp"
#include "../../event/headers/EventHandler.hpp"
#include <memory> // Inclusion de <memory> pour std::unique_ptr
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

class Nova {
private:

public:
    std::unique_ptr<Render> render; // Utilisation de std::unique_ptr pour Render
    std::unique_ptr<System> system; // Utilisation de std::unique_ptr pour System
    std::unique_ptr<Draw> draw; // Utilisation de std::unique_ptr pour Draw
    std::unique_ptr<UIManager> uimanager; // Utilisation de std::unique_ptr pour UI
    std::unique_ptr<NetworkManager> networkmanager; // Utilisation de std::unique_ptr pour NetworkManager
    std::unique_ptr<EventHandler> eventHandler; // Utilisation de std::unique_ptr pour EventHandler


    Nova(); // Déclaration du constructeur

    bool Init(); // Déclaration de la fonction
    void Game();

    ~Nova(); // Déclaration du destructeur
};