#include "headers/UIString.hpp"

extern sf::RenderWindow* renderWindow;

UIString::UIString() {
   
}

bool UIString::newString(const std::string& id, std::string& groupID, std::string& content, int x, int y, int width, int height, const std::string& font, int fontSize, const std::string& color, const std::string& effect, int layer, bool isActive) {

        this->id = id;
        this->groupID = groupID;
        this->content = content;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->font = font;
        this->fontSize = fontSize;
        this->color = color;
        this->effect = effect;
        this->layer = layer;
        this->isActive = isActive;
        this->isInit = false;

    return true;
}

bool UIString::setPosition() {
    return true;
}

bool UIString::setEffect() {
    return true;
}

bool UIString::setLayer() {
    return true;
}

std::string UIString::getGroupID() const {
    return this->groupID;
}

bool UIString::setGroupID(const std::string& groupID) {
    return true;
}

bool UIString::setIsActive(bool isActive) {
    this->isActive = isActive;
    return true;
}

bool UIString::getIsActive() const {
    return this->isActive;
}

std::string UIString::getID() const {
    return this->id;
}

int UIString::getLayer() const {
    return this->layer;
}

sf::Text UIString::getText() const {
    return this->text;
}

void UIString::rescale(sf::Text& text, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition) {
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

    text.setCharacterSize(static_cast<unsigned int>(fontSize * scale)); // Ajustement de la taille du caractère

    sf::Vector2f adjustedPosition = initialPosition;
    if (resolution.x < nativeResolution.x || resolution.y < nativeResolution.y) {
        adjustedPosition.x *= scaleX;
        adjustedPosition.y *= scaleY;
    }
    text.setPosition(adjustedPosition);
}

bool UIString::show() {
    if (!this->isInit) {
        // Load the font with a specific character size
        if (!this->fontObj.loadFromFile(this->font)) {
            std::cerr << "Error loading font from file: " << this->font << std::endl;
            return false;
        }
        // Set the font smoothing
        this->text.setFont(this->fontObj);

        // Set other properties of the sf::Text object
        this->text.setString(this->content);
        this->text.setCharacterSize(this->fontSize);
        this->text.setFillColor(sf::Color::White);
        this->text.setPosition(this->x, this->y);

        sf::Vector2u nativeResolution(3840, 2160);
        sf::Vector2u resolution = renderWindow->getSize();
        sf::Vector2f initialPosition(this->x, this->y);

        this->rescale(this->text, resolution, nativeResolution, initialPosition);

        this->isInit = true;
    }
    renderWindow->draw(this->text);
    return true;
}


UIString::~UIString() {
}