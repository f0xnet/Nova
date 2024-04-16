#include "headers/UIGraphic.hpp"

extern sf::RenderWindow* renderWindow;
extern sf::RenderTexture* renderTexture;

UIGraphic::UIGraphic() {
    //std::cout << "UIGraphic created" << std::endl;
}

bool UIGraphic::initGraphic(const std::string& id, std::string& groupID, int x, int y, int width, int height, const std::string& path, const std::string& effect, int layer, bool isActive) {
    this->id = id;
    this->groupID = groupID;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->path = path;
    this->effect = effect;
    this->layer = layer;
    this->isActive = isActive;

    std::cout << "Graphic created with ID: " << id << std::endl;

    return true;
}

bool UIGraphic::setTexture() {
    return true;
}

bool UIGraphic::setPosition() {
    return true;
}

bool UIGraphic::setEffect() {
    return true;
}

bool UIGraphic::setLayer() {
    return true;
}

std::string UIGraphic::getGroupID() const {
    return this->groupID;
}

bool UIGraphic::setGroupID(const std::string& groupID) {
    return true;
}

bool UIGraphic::setIsActive(bool isActive) {
    this->isActive = isActive;
    return true;
}

bool UIGraphic::getIsActive() const{
    return this->isActive; 
}

std::string UIGraphic::getID() const {
    return this->id;
}

int UIGraphic::getLayer() const {
    return this->layer;
}

sf::Sprite UIGraphic::getSprite() const {
    return this->sprite;
}

void UIGraphic::rescale(sf::Sprite& sprite, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition) 
{
    float scaleX = static_cast<float>(resolution.x) / nativeResolution.x;
    float scaleY = static_cast<float>(resolution.y) / nativeResolution.y;

    float scale;
    if (resolution.x > nativeResolution.x || resolution.y > nativeResolution.y) {
        scale = std::min(scaleX, scaleY);
    } else if (resolution.x < nativeResolution.x || resolution.y < nativeResolution.y) {
        scale = std::min(scaleX, scaleY);
    } else {
        scale = 1.0f;
    }

    sprite.setScale(scale, scale);
    sf::Vector2f adjustedPosition = initialPosition;

    if (resolution.x < nativeResolution.x || resolution.y < nativeResolution.y) {
        adjustedPosition.x *= scaleX;
        adjustedPosition.y *= scaleY;
    }
    sprite.setPosition(adjustedPosition);

    this->activeX = adjustedPosition.x;
    this->activeY = adjustedPosition.y;
    this->activeWidth = sprite.getLocalBounds().width * sprite.getScale().x;
    this->activeHeight = sprite.getLocalBounds().height * sprite.getScale().y;
}

bool UIGraphic::show() {
    if (!this->isInit) {
        if (this->texture.loadFromFile(path)) {
            if (this->texture.getSize().x > 0 && this->texture.getSize().y > 0) {
                this->sprite.setTexture(this->texture);
                sf::Vector2u nativeResolution(3840, 2160); // Remplacez ceci par votre résolution native
                sf::Vector2u resolution = renderWindow->getSize();
                sf::Vector2f initialPosition(this->x, this->y); // Remplacez ceci par la position initiale de l'image
                this->rescale(this->sprite, resolution, nativeResolution, initialPosition);
                sf::Vector2f spriteSize = this->sprite.getScale();
                std::cout << "Taille du sprite après redimensionnement: " << spriteSize.x << ", " << spriteSize.y << std::endl;
                std::cout << "Texture loaded from file: " << path << std::endl;
            } else {
                std::cerr << "Texture invalide - Dimensions nulles pour path: " << path << std::endl;
                return false;
            }
        } else {
            std::cerr << "Error loading texture from file: " << path << std::endl;
            return false;
        }
        this->isInit = 1;
    }
    renderWindow->draw(this->sprite);
    return true;
}

UIGraphic::~UIGraphic() {
}
