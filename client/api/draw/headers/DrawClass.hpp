#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

class Draw {
    // Constructor

    private:
        // Structure pour stocker les propriétés de l'objet à dessiner
        struct TextureHeap {
            std::string UIID;
            std::string ID;
            std::string groupID;
            int x, y, width, height, layer, isActive;
            std::string effect;
            sf::Texture texture;

            TextureHeap(const std::string& UIID, const std::string& id, std::string& grpID, int posX, int posY, int w, int h, const std::string& eff, int lay, int active, const sf::Texture& texture)
            : UIID(UIID), ID(id), groupID(grpID), x(posX), y(posY), width(w), height(h), layer(lay), isActive(active), effect(eff), texture(texture) {}
        };
        std::vector<TextureHeap> textureHeap;
        sf::RenderWindow* window;

    public:

    // Déclaration du constructeur
    Draw();

    bool Init(); // Déclaration de la fonction
    void setScale(sf::Vector2f& position, sf::Vector2f& scale, const sf::Vector2u& currentResolution);
    void addTexture(const std::string& UIID, const std::string& id, std::string& groupID, int x, int y, int width, int height, const std::string& path, const std::string& effect, int layer, int isActive);
    void renderTextures(sf::RenderWindow& window);

    ~Draw(); // Déclaration du destructeur
};
