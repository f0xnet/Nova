#include "headers/NovaClass.hpp"
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio.hpp>
#include "../animation/headers/AnimationManager.hpp"


sf::RenderWindow* renderWindow = nullptr;
EventHandler* eventHandlerPtr = nullptr;
NetworkManager* networkManagerPtr = nullptr;
UIManager* uiManagerPtr = nullptr;

const int NUM_LIGHTS = 3;


Nova::Nova() {

    this->networkmanager = std::make_unique<NetworkManager>();
    this->render = std::make_unique<Render>();
    this->system = std::make_unique<System>();
    this->draw = std::make_unique<Draw>();
    this->uimanager = std::make_unique<UIManager>();
    this->eventHandler = std::make_unique<EventHandler>();
}

//We initialize all the classes Nova contains and check if they are initialized correctly
bool Nova::Init() 
{
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
    
    SceneManager sceneManager;
    sceneManager.load_from_file("data/dialogs/rin/rin_care_01.json");
    sceneManager.print_scene_details();
    std::string selected_dialogue = sceneManager.select_dialogue("scene1", 10);

    // Affichez le dialogue sélectionné (ou un message approprié si aucun dialogue n'est sélectionné)
    if (!selected_dialogue.empty()) {
        std::cout << "Selected Dialogue: " << selected_dialogue << std::endl;
    } else {
        std::cout << "No dialogue selected." << std::endl;
    }

    // Chargez votre musique ici
    if (!music.openFromFile("data/music/mainmenu.ogg")) {
        std::cerr << "Error loading music" << std::endl;
    }
    music.play();
    music.setVolume(5);
    music.setLoop(true);

    sf::Shader shader;
    if (!shader.loadFromFile("data/shaders/hdr.frag", sf::Shader::Fragment)) {
        std::cerr << "Failed to load shaders" << std::endl;
        return;
    }

    AnimationManager animationManager;
    if (!animationManager.addEntityAnimationsFromJson("entity1", "data/characters/animation.json")) {
        std::cerr << "Failed to load animations for entity1." << std::endl;
        printf("Failed to load animations for entity1.\n");
    }
    animationManager.pushAnimation("entity1", "walk");
    sf::Sprite animatedSprite;
    sf::Clock clock;

if (this->render->newWindow(3840, 2160, "Nova", true, false, false)) {
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

        shader.setUniform("texture", sf::Shader::CurrentTexture);
        shader.setUniform("exposure", 6.0f); // Modifier si nécessaire
        shader.setUniform("gamma", 0.30f); // Modifier si nécessaire
        shader.setUniform("saturationStrength", 1.4f); // Modifier si nécessaire
        shader.setUniform("auraStrength", 0.5f); // Modifier si nécessair
        shader.setUniform("auraBoost", 0.1f); // Modifier si nécessaire

        texture.update(*renderWindow);
        sprite.setTexture(texture);

        // Mise à jour de l'animation
        float deltaTime = clock.restart().asSeconds();
        animationManager.update(deltaTime);

        // Obtenir la texture de la frame actuelle
        const sf::Texture& currentFrame = animationManager.getCurrentFrame();
        animatedSprite.setTexture(currentFrame);

        // Positionner le sprite (optionnel)
        sprite.setPosition(animationManager.getAnimationPosition());
        

        // Appliquer le shader lors du rendu du sprite
        renderWindow->draw(sprite, &shader);
        renderWindow->draw(animatedSprite);

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

//