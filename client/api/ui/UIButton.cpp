#include "headers/UIButton.hpp"

extern sf::RenderWindow* renderWindow;

UIButton::UIButton() {
    std::cout << "UIButton created" << std::endl;
} 

bool UIButton::newButton(std::string& id, std::string& groupID, bool haveText, std::string& text, std::string& fontPath, int fontSize, std::string& color, int x, int y, int width, int height, const std::string& path, const std::string& path_hover, 
                         const std::string& path_pressed, const std::string& effect, const std::string& action, const std::string& value, int layer, bool isActive) {
    this->id = id;
    this->groupID = groupID;
    this->haveText = haveText;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->path = path;
    this->path_hover = path_hover;
    this->path_pressed = path_pressed;
    this->effect = effect;
    this->action = action;
    this->value = value;
    this->layer = layer;
    this->isActive = isActive;
    this->buttonState = 0;
    this->isInit = false;
    this->font = fontPath;
    this->fontSize = fontSize;
    this->color = color;
    this->content = text;

    return true; 
}

bool UIButton::Init() {
    if(!this->InitTexture(this->path, this->sprite, this->texture)) {
        std::cout << "Error while setting texture" << std::endl;
        return false;
    }
    if(!this->InitTexture(this->path_hover, this->sprite_hover, this->texture_hover)) {
        std::cout << "Error while setting texture" << std::endl;
        return false;
    }
    if(!this->InitTexture(this->path_pressed, this->sprite_pressed, this->texture_pressed)) {
        std::cout << "Error while setting texture" << std::endl;
        return false;
    }
    if(!this->InitText()) {
        std::cout << "Error while setting text" << std::endl;
        return false;
    }
    this->isInit = 1;
    return true;
}

bool UIButton::InitTexture(std::string& pathTexture, sf::Sprite& sprite, sf::Texture& texture) {
    if (texture.loadFromFile(pathTexture)) {
        if (texture.getSize().x > 0 && texture.getSize().y > 0) {
            sprite.setTexture(texture);
            sf::Vector2u nativeResolution(3840, 2160); // Remplacez ceci par votre résolution native
            sf::Vector2u resolution = renderWindow->getSize();
            sf::Vector2f initialPosition(this->x, this->y); // Remplacez ceci par la position initiale de l'image
            this->rescaleBackground(sprite, resolution, nativeResolution, initialPosition);
        } else {
            std::cerr << "Texture invalide - Dimensions nulles pour path: " << pathTexture << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error loading texture from file: " << pathTexture << std::endl;
        return false;
    }
    return true;
}

void UIButton::centerTextOnImage() {
    sf::FloatRect textRect = this->text.getLocalBounds();
    this->text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

    float x = this->activeX + (this->activeWidth / 2.0f);
    float y = this->activeY + (this->activeHeight / 2.0f) - (this->text.getLocalBounds().height / 8.0f);
    this->text.setPosition(x, y);
}

bool UIButton::InitText() {
    if (!this->fontObj.loadFromFile(this->font)) {
            std::cerr << "Error loading font from file: " << this->font << std::endl;
            return false;
    }
    this->text.setFont(this->fontObj);

    this->text.setString(this->content);
    this->text.setCharacterSize(this->fontSize);
    this->text.setFillColor(sf::Color::White);
    this->text.setPosition(this->x, this->y);

    sf::Vector2u nativeResolution(3840, 2160);
    sf::Vector2u resolution = renderWindow->getSize();
    sf::Vector2f initialPosition(this->x, this->y);
    
    this->rescaleText(this->text, resolution, nativeResolution, initialPosition);
    this->centerTextOnImage();
    return true;
} 

bool UIButton::setPosition() {
    return true;
}

bool UIButton::setEffect() {
    return true;
}

bool UIButton::setLayer() {
    return true;
}

bool UIButton::setGroupID(const std::string& groupID) {
    return true;
}

bool UIButton::setIsActive(bool isActive) {
    return true;
}

bool UIButton::getIsActive() const{
    return this->isActive; 
}

std::string UIButton::getID() const {
    return this->id;
}

int UIButton::getLayer() const {
    return this->layer;
}   

sf::Sprite UIButton::getSprite() const {
    return this->sprite;
}

void UIButton::rescaleBackground(sf::Sprite& sprite, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition) {

    float scaleX = static_cast<float>(resolution.x) / nativeResolution.x;
    float scaleY = static_cast<float>(resolution.y) / nativeResolution.y;

    float scale;
    if (resolution.x > nativeResolution.x || resolution.y > nativeResolution.y) {
        // Utilisez le facteur d'échelle minimum pour agrandir l'image
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

bool UIButton::rescaleText(sf::Text& text, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition) {
    float scaleX = static_cast<float>(resolution.x) / nativeResolution.x;
    float scaleY = static_cast<float>(resolution.y) / nativeResolution.y;

    float scale;
    if (resolution.x > nativeResolution.x || resolution.y > nativeResolution.y) {
        // Utilisez le facteur d'échelle minimum pour agrandir l'image
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

    this->textActiveX = adjustedPosition.x;
    this->textActiveY = adjustedPosition.y;
    this->textActiveWidth = text.getLocalBounds().width * text.getScale().x;
    this->textActiveHeight = text.getLocalBounds().height * text.getScale().y;
    this->text.setString(this->content);
    return true;
}

bool UIButton::CheckMouseEvent()
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(*renderWindow);
    bool isMouseOnButton = (mousePos.x >= activeX && mousePos.x <= activeX + activeWidth &&
                            mousePos.y >= activeY && mousePos.y <= activeY + activeHeight);

    switch (this->event.type)
    {
        case sf::Event::MouseMoved:
            if (!buttonPressed)
                buttonState = isMouseOnButton ? HOVER : NOT_HOVER;
            break;

        case sf::Event::MouseButtonPressed:
            if (this->event.mouseButton.button == sf::Mouse::Left)
            {
                if (isMouseOnButton && !buttonPressed)
                {
                    buttonState = PRESSED;
                }
            }
            break;

        case sf::Event::MouseButtonReleased:
            if (this->event.mouseButton.button == sf::Mouse::Left)
            {
                if (isMouseOnButton && buttonState == PRESSED)
                {
                    buttonPressed = true;
                }
                buttonState = isMouseOnButton ? HOVER : NOT_HOVER;
            }
            break;

        default:
            break;
    }
    return true;
}

/*bool UIButton::executeAction() {
    if(this->button->id == "login") {
        std::cout << "Login button pressed" << std::endl;
    }
    return true;
}*/

bool UIButton::getButtonState() const {
    return this->buttonPressed;
}

void UIButton::resetClick() {
    this->buttonPressed = false;
}

bool UIButton::show() {
    if(this->isActive) {
        if(!this->isInit) {
           if(!this->Init()) {
               std::cout << "Error while initializing button" << std::endl;
               return false;
           }
        }
        this->CheckMouseEvent();
        if (buttonState == NOT_HOVER) renderWindow->draw(sprite);
        else if (buttonState == HOVER) renderWindow->draw(sprite_hover);
        else if (buttonState == PRESSED) renderWindow->draw(sprite_pressed);
        renderWindow->draw(this->text);
    }
    return true;
}

UIButton::~UIButton() {
}