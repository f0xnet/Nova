#pragma once
#include <memory> // Inclusion de <memory> pour std::unique_ptr
#include <Windows.h> 
#include <iostream>
#include <thread>
#include <mutex>
#include <array>

class System {
private:
    static std::mutex windowMutex;
    static HWND currentWindow;
    static RECT currentRect;
    static TCHAR currentTitle[256];

    static int focusedWindowWidth;
    static int focusedWindowHeight;
    static int focusedWindowX;
    static int focusedWindowY;

public: // Utilisation de std::unique_ptr pour Render

    System(); // Déclaration du constructeur

    bool Init(); // Déclaration de la fonction
    static void getWindowInfo();
    static std::tuple<HWND, RECT, TCHAR*> getCurrentWindowInfo();
    std::array<int, 4> getFocusedWindowX();

    ~System(); // Déclaration du destructeur
};