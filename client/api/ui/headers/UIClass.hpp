#pragma once
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

#include "UIButton.hpp"
#include "UIGraphic.hpp"
#include "UIString.hpp"
#include "../../network/headers/NetworkManager.hpp"

class UIClass {
    // Constructor

private:

    struct GraphicHeap {
        UIGraphic graphic;
        GraphicHeap() : graphic() {}
    };
    std::vector<GraphicHeap> graphicHeap; 

    struct StringHeap {
        UIString string;
        StringHeap() : string() {}
    };
    std::vector<StringHeap> stringHeap;

    struct ButtonHeap {
        UIButton button;
        ButtonHeap() : button() {}
    };
    std::vector<ButtonHeap> buttonHeap;

    int isActive;
    int HOVER = 1, PRESSED = 2, NOT_HOVER = 0;
    
    
public:
    NetworkManager* networkManager;
    std::string UIID;
    sf::Event event;
    // Déclaration du constructeur
    UIClass();

    bool Init(); // Déclaration de la fonction
    bool show();

    ~UIClass(); // Déclaration du destructeur
};