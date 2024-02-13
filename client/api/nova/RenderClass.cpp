#include "headers/RenderClass.hpp"

Render::Render() {
    std::cout << "Render Class Created!" << std::endl;

}

bool Render::init(Draw *drawPtr) {
    /// SFML s'initialise avec la création de la fenêtre
    return true;
}

bool Render::newWindow(int width, int height, const std::string& title, bool fullscreen, bool isSplashScreen, bool borderless) {
    sf::Uint32 style;
    if (fullscreen) {
        style = sf::Style::Fullscreen;
    } else if (borderless) {
        style = sf::Style::None; // Style pour une fenêtre sans bordures
    } else {
        style = sf::Style::Default;
    }

    // Utiliser la résolution spécifiée que ce soit pour le mode plein écran ou fenêtré
    sf::VideoMode videoMode(width, height);

    renderer.create(videoMode, title, style);

    if (fullscreen) {
        // Ajuster la vue pour le mode plein écran
        sf::View view(sf::FloatRect(0.f, 0.f, static_cast<float>(videoMode.width), static_cast<float>(videoMode.height)));
        renderer.setView(view);
    }

    if (isSplashScreen) {
        HWND hwnd = windowHandle();
        LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
        lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
        SetWindowLong(hwnd, GWL_STYLE, lStyle);
        LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        this->setWindowBackground("image.png");
        this->enableTransparency();
    }
    return true;
}

void Render::render(sf::RenderStates states) {
    renderer.clear(sf::Color::Transparent);
    //renderer.draw(backgroundSprite, states);
    renderer.draw(backgroundSprite);
    //renderer.display();
}

int Render::enableTransparency() {
    HWND handle = windowHandle();

    #ifdef _WIN64
        // Version 64 bits
        LONG_PTR result = SetWindowLongPtr(handle, GWL_EXSTYLE, GetWindowLongPtr(handle, GWL_EXSTYLE) | WS_EX_LAYERED);
    #else
        // Version 32 bits
        LONG result = SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED);
    #endif

    if (result == 0 && GetLastError() != ERROR_SUCCESS) {
        std::cerr << "SetWindowLong Error: " << GetLastError() << std::endl;
        return 0;
    }
    if (!SetLayeredWindowAttributes(handle, RGB(0, 0, 0), 0, LWA_COLORKEY)) {
        std::cerr << "SetLayeredWindowAttributes Error: " << GetLastError() << std::endl;
        return 0;
    }
    return 1;
}

sf::RenderWindow& Render::getRenderer() {
    return renderer;
}

bool Render::setWindowBackground(const std::string& imagePath) {
    if (!backgroundTexture.loadFromFile(imagePath)) {
        std::cerr << "Image loading failed" << std::endl;
        return false;
    }
    backgroundSprite.setTexture(backgroundTexture);
     backgroundSprite.setScale(
        static_cast<float>(renderer.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(renderer.getSize().y) / backgroundTexture.getSize().y
    );
    return true;
}

HWND Render::windowHandle() {
    return renderer.getSystemHandle();
}

Render::~Render() {
    std::cout << "Render destroyed!" << std::endl;
}