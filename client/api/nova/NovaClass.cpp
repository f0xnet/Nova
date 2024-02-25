#include "headers/NovaClass.hpp"
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio.hpp>

sf::RenderWindow* renderWindow = nullptr;
EventHandler* eventHandlerPtr = nullptr;
NetworkManager* networkManagerPtr = nullptr;
UIManager* uiManagerPtr = nullptr;

Nova::Nova() {

    this->networkmanager = std::make_unique<NetworkManager>();
    this->render = std::make_unique<Render>();
    this->system = std::make_unique<System>();
    this->draw = std::make_unique<Draw>();
    this->uimanager = std::make_unique<UIManager>();
    this->eventHandler = std::make_unique<EventHandler>();
}

bool Nova::Init() { //We initialize all the classes Nova contains and check if they are initialized correctly

    std::string ipAddress = "127.0.0.1"; // Remplacez par l'adresse IP souhaitée
    unsigned short port = 53000; 

    if (!this->networkmanager || !this->networkmanager->Init(ipAddress, port)) return false;
    networkManagerPtr = this->networkmanager.get();

    if (!this->eventHandler) return false;
    eventHandlerPtr = this->eventHandler.get();

    if (!this->draw || !this->draw->Init()) return false;
    if (!this->system || !this->system->Init()) return false;
    if (!this->render || !this->render->init(this->draw.get())) return false;
    
    if (!this->uimanager || !this->uimanager->Init()) return false;
    uiManagerPtr = this->uimanager.get();
    
    renderWindow = &(this->render->getRenderer());     ///We assign the renderer pointer to our private pointer because we need to use it in other functions

    return true;
}

void Nova::Game() {
    // Charger les shaders
     sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

     std::cout << desktop.width << " " << desktop.height << std::endl;
     
    sf::Music music;
    
    // Chargez votre musique ici
    if (!music.openFromFile("data/music/main_menu.ogg")) {
        std::cerr << "Error loading music" << std::endl;
    }
    music.play();
    music.setVolume(5);
    music.setLoop(true);

    sf::Shader shader;
    if (!shader.loadFromFile("data/shaders/glow.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load shaders" << std::endl;
        return;
    }

    if (this->render->newWindow(3840, 2160, "Nova", false, false, false)) {
        renderWindow->setFramerateLimit(60);
        this->uimanager->newUI("loginmenu");

        sf::Texture texture;
        texture.create(3840, 2160);
        sf::Sprite sprite;

        while (renderWindow->isOpen()) {
            sf::Event event;
            while (renderWindow->pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    renderWindow->close();
            }

            renderWindow->clear(sf::Color::Transparent);

            this->uimanager->show(event);
            /*std::string uiAction = this->uimanager->getUIAction();
            if (uiAction != "") {
            }*/

            texture.update(*renderWindow);
            sprite.setTexture(texture);
            
            // Définir les uniformes pour le shader
            shader.setUniform("scene", texture);
            shader.setUniform("imageSize", sf::Glsl::Vec2(renderWindow->getSize()));
            shader.setUniform("imagePosition", sf::Glsl::Vec2(0, 0)); // Ajuster si nécessaire

            renderWindow->clear();
            renderWindow->draw(sprite, &shader);
            renderWindow->display();  
        }
    } else {
        std::cout << "Render is not initialized!" << std::endl;
    }
}

Nova::~Nova() {
    std::cout << "Nova destroyed!" << std::endl;
}

   /*if (clock.getElapsedTime().asMilliseconds() > 100) {
                std::array<int, 4> windowInfo = this->system->getFocusedWindowX();
                if(lastWindowX != windowInfo[0] && lastWindowY != windowInfo[1]) 
                {
                    lastWindowX = windowInfo[0];
                    lastWindowY = windowInfo[1];
                    int x = windowInfo[0] + windowInfo[2] - 390;
                    int y = windowInfo[1] - 390;

                    this->renderWindow->setPosition(sf::Vector2i(x, y));
                    clock.restart();
                }
            }*/