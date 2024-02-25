#ifndef UIMANAGER_HPP
#define UIMANAGER_HPP
#include <iostream>
#include <fstream>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../../draw/headers/DrawClass.hpp"
#include "../../ui/headers/UIClass.hpp"
#include "../../network/headers/NetworkManager.hpp"
#include "../../event/headers/EventHandler.hpp"

class UIManager {
private:
    struct UIHeap {
        UIClass ui;
        UIHeap() : ui() {}
    };
    std::vector<UIHeap> uiHeap; 
    std::shared_ptr<EventHandler> eventHandler;

public:
    UIManager();
    ~UIManager();

    bool Init();
    bool newUI(const std::string& UIID);
    void show(sf::Event& event);
    bool setGroupID(const std::string& UUIID, const std::string& groupID);
    

private:
};
#endif // UIMANAGER_HPP