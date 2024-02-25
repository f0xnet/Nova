#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>

#include "../../draw/headers/DrawClass.hpp"

class UIGraphic {

private:
    std::string id;
    std::string groupID;
    std::string path;
    int x, y, width, height, layer;
    bool isActive;
    std::string effect;
    sf::Texture texture;
    sf::Sprite sprite;
    int isInit = 0;
    int activeX, activeY, activeWidth, activeHeight;

public:
    std::string UIID;

    UIGraphic();

    bool initGraphic(const std::string& id, std::string& groupID, int x, int y, int width, int height, const std::string& path, const std::string& effect, int layer, bool isActive);
    bool setTexture();
    bool setPosition();
    bool setEffect();
    bool setIsActive(bool isActive);
    bool setLayer();
    std::string getGroupID() const;
    bool setGroupID(const std::string& groupID);
    bool getIsActive() const;
    std::string getID() const;
    int getLayer() const;
    sf::Sprite getSprite() const;
    bool show();
    void rescale(sf::Sprite& sprite, sf::Vector2u resolution, sf::Vector2u nativeResolution, sf::Vector2f initialPosition);

    ~UIGraphic();
};