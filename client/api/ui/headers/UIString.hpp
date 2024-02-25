#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include "../../draw/headers/DrawClass.hpp"


class UIString {
    // Class for UI graphics :: UI graphics are the images that are displayed on the screen
private:

    std::string id;
    std::string groupID;
    std::string content;
    int x, y, width, height, layer;
    bool isActive;
    std::string font;
    int fontSize;
    std::string color;
    std::string effect;
    sf::Text text;
    sf::Font fontObj;
    int isInit = 0;
    
public:

    // Déclaration du constructeur
    UIString();

    bool newString(const std::string& id, std::string& groupID, std::string& content, int x, int y, int width, int height, const std::string& font, int fontSize, const std::string& color, const std::string& effect, int layer, bool isActive);
    bool setPosition();
    bool setEffect();
    bool setLayer();
    std::string getGroupID() const;
    bool setGroupID(const std::string& groupID);
    bool setIsActive(bool isActive);
    bool getIsActive() const;
    std::string getID() const;
    int getLayer() const;
    sf::Text getText() const;
    bool show();
    void rescale(sf::Text& text, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition);

    ~UIString(); // Déclaration du destructeur
};