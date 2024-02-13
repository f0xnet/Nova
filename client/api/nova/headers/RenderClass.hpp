// RenderClass.hpp
#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "../../draw/headers/DrawClass.hpp"
#include <SFML/Graphics/Shader.hpp>

class Render {
private:
    sf::RenderWindow renderer;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::RenderStates states;

public:
    sf::RenderWindow& getRenderer();

    Render();
    bool init(Draw *drawPtr);
    bool newWindow(int width, int height, const std::string& title, bool fullscreen, bool isSplashScreen, bool borderless);
    int enableTransparency();
    bool setWindowBackground(const std::string& imagePath);
    HWND windowHandle();
    void render(sf::RenderStates states);
    ~Render();
};