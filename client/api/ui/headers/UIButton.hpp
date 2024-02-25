#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <thread>

class UIButton {
    // Class for UI graphics :: UI graphics are the images that are displayed on the screen
private:

    std::string id, groupID, path, path_hover, path_pressed, effect, action, value, font, color, content, 
    buttonData; // buttonData is the data that will be sent to the server Or event manager when the button is clicked
    bool haveText, isActive, isInit = false, buttonPressed = false;
    int x, y, width, height, layer, fontSize, buttonState, textActiveHeight, textActiveWidth, textActiveX, textActiveY;
    int activeX, activeY, activeWidth, activeHeight;
    int HOVER = 1, PRESSED = 2, NOT_HOVER = 0;
    sf::Texture texture, texture_hover, texture_pressed;
    sf::Sprite sprite, sprite_hover, sprite_pressed;
    sf::Text text;
    sf::Font fontObj;

    sf::Vector2i mousePos;

    std::string generateButtonData() const;

public:

    std::string UIID;
    sf::Event event;

    UIButton();

    bool newButton(std::string& uid, std::string& id, std::string& groupID, bool haveText, std::string& text, std::string& fontPath, int fontSize, std::string& color, int x, int y, int width, int height, const std::string& path, const std::string& path_hover, 
                         const std::string& path_pressed, const std::string& effect, const std::string& action, const std::string& value, int layer, bool isActive);

    bool InitTexture(std::string& pathTexture, sf::Sprite& sprite, sf::Texture& texture);
    bool InitText();
    void centerTextOnImage();
    bool setPosition();
    bool setEffect();
    bool setIsActive(bool isActive);
    bool setLayer();
    std::string getGroupID() const;
    bool setGroupID(const std::string& groupID);
    bool getIsActive() const;
    std::string getButtonAction() const;
    int getLayer() const;
    sf::Sprite getSprite() const;
    bool Init();
    bool show();
    void rescaleBackground(sf::Sprite& sprite, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition);
    bool rescaleText(sf::Text& text, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition);
    bool CheckMouseEvent();
    bool getButtonState() const;
    void resetClick();
    std::string getButtonData() const;

    ~UIButton(); // Déclaration du destructeur
};