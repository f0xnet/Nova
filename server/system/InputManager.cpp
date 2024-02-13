#include "headers/InputManager.hpp"
#include <iostream>
#include <chrono>

// Pour Windows
#ifdef _WIN32
#include <conio.h>
#else
// Pour Linux/Unix
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Vérifie si une touche a été pressée sans bloquer
int _kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

InputManager::InputManager() : listening(false) {}

InputManager::~InputManager() {
    stopListening();
}

void InputManager::startListening() {
    listening = true;
    inputThread = std::thread(&InputManager::processInput, this);
}

void InputManager::stopListening() {
    listening = false;
    if (inputThread.joinable()) {
        inputThread.join();
    }
}

void InputManager::processInput() {
    std::cout << "[ Press Q to exit, R for restart, S for saving, B for backup ]" << std::endl; // Affiche le message une fois

    while (listening) {
        if (_kbhit()) {
            char c = getchar(); // Utilise getchar() qui est standard en C/C++
            switch (c) {
                case 'q':
                    this->key = c;
                    break;
                // Ajoutez d'autres cas au besoin
                default:
                    std::cout << "Unknown key" << std::endl;
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

