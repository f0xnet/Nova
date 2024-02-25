#ifndef UICLASS_HPP
#define UICLASS_HPP
#include <iostream>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

#include "UIButton.hpp"
#include "UIGraphic.hpp"
#include "UIString.hpp"
#include "../../event/headers/EventHandler.hpp"

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
    bool buttonPressed = false;
    std::string clickedbButtonData;
    std::shared_ptr<EventHandler> eventHandler;
    
    
public:
    std::string UIID;
    int layer = 0;
    sf::Event event;
    // Déclaration du constructeur
    UIClass();

    bool Init(); // Déclaration de la fonction
    bool show();
    bool getIsActive() const;
    std::string getButtonData();
    bool setGroupID(const std::string& groupID);

    ~UIClass(); // Déclaration du destructeur
};
#endif // UICLASS_HPP