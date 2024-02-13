#include "headers/SystemClass.hpp"

std::mutex System::windowMutex;
HWND System::currentWindow = nullptr;
RECT System::currentRect = {0};
TCHAR System::currentTitle[256] = {0};

int System::focusedWindowWidth = 0;
int System::focusedWindowHeight = 0;
int System::focusedWindowX = 0;
int System::focusedWindowY = 0;

System::System() {
   // std::cout << "System Class Created!" << std::endl;
}

bool System::Init() {
    std::thread getWindowThread([this]() {
        while (true) {
            getWindowInfo();
            std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Attendre un peu entre chaque vérification
        }
    });
    getWindowThread.detach(); // Détacher la thread pour qu'elle s'exécute en arrière-plan
    return true;
}  

void System::getWindowInfo() {
    std::lock_guard<std::mutex> lock(windowMutex);

    HWND foreground = GetForegroundWindow(); 

    if (foreground != NULL) {
        GetWindowRect(foreground, &currentRect); 

        //int width = currentRect.right - currentRect.left;
        //int height = currentRect.bottom - currentRect.top;

        GetWindowText(foreground, currentTitle, sizeof(currentTitle)); 
        //std::wcout << "Current window: " << currentTitle << std::endl;
        currentWindow = foreground;
    }
    focusedWindowWidth = currentRect.right - currentRect.left;
    focusedWindowHeight = currentRect.bottom - currentRect.top;
    focusedWindowX = currentRect.left;
    focusedWindowY = currentRect.top;
}

std::array<int, 4> System::getFocusedWindowX() {
    return { focusedWindowX, focusedWindowY, focusedWindowWidth, focusedWindowHeight };
}

System::~System() {
    std::cout << "System destroyed!" << std::endl;
}