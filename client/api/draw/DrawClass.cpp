#include "headers/DrawClass.hpp"

Draw::Draw() {
    //std::cout << "Draw Class Created!" << std::endl;
}

bool Draw::Init() {
    return true;
} 

void Draw::setScale(sf::Vector2f& position, sf::Vector2f& scale, const sf::Vector2u& currentResolution) {
    // Chargez les valeurs de position et d'échelle à partir du fichier de configuration pour 1080p

    // Obtenez la résolution actuelle de l'écran
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

    // Calculez les ratios de mise à l'échelle
    float scaleX = static_cast<float>(currentResolution.x) / 1920.0f; // 1920p est la largeur de référence
    float scaleY = static_cast<float>(currentResolution.y) / 1080.0f;  // 1080p est la hauteur de référence

    // Appliquez les ratios de mise à l'échelle aux valeurs de position et d'échelle
    position.x *= scaleX;
    position.y *= scaleY;
    scale.x *= scaleX;
    scale.y *= scaleY;
}

void Draw::addTexture(const std::string& UIID, const std::string& ID, std::string& groupID, int x, int y, int width, int height, const std::string& path, const std::string& effect, int layer, int isActive) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }
    TextureHeap newTexture(UIID, ID, groupID, x, y, width, height, effect, layer, isActive, texture);
    this->textureHeap.push_back(newTexture);
    std::cout << "Texture added!" << std::endl;
}

Draw::~Draw() {
    std::cout << "Draw destroyed!" << std::endl;
}

void Draw::renderTextures(sf::RenderWindow& window) {
    for (const auto& textureHeapItem : textureHeap) {
        if (textureHeapItem.isActive) { // Vérifie si l'élément est actif
            sf::Sprite sprite;
            sprite.setTexture(textureHeapItem.texture);
            sprite.setPosition(textureHeapItem.x, textureHeapItem.y);
            sprite.setScale(
                static_cast<float>(textureHeapItem.width) / textureHeapItem.texture.getSize().x,
                static_cast<float>(textureHeapItem.height) / textureHeapItem.texture.getSize().y
            );
            // Ajoutez ici toute autre configuration nécessaire, comme les effets

            window.draw(sprite); // Dessine le sprite sur la fenêtre
        }
    }
}
//