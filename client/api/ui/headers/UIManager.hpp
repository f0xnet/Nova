#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../../draw/headers/DrawClass.hpp"
#include "../../ui/headers/UIClass.hpp"
#include "../../network/headers/NetworkManager.hpp"

class UIManager {
private:
    struct UIHeap {
        UIClass ui;
        UIHeap() : ui() {}
    };
    std::vector<UIHeap> uiHeap; 

    NetworkManager* networkManager;

public:
    UIManager();
    ~UIManager();

    bool Init(NetworkManager* networkManagerPtr);
    bool newUI(const std::string& UIID);
    void show(sf::Event& event);

private:
};