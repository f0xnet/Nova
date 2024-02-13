#include "headers/UIClass.hpp"

UIClass::UIClass() {
    std::cout << "UI Class Created!" << std::endl;
}

bool UIClass::Init() {
    //std::cout << UIID << std::endl;
    std::string jsonFile = "data/ui/json/" + UIID + ".json"; 

    std::ifstream file(jsonFile);
    if (!file.is_open()) {
        std::cerr << "Unable to open JSON file!" << std::endl;
        return false;
    }
    // Parsing du fichier JSON
    nlohmann::json jsonData;
    file >> jsonData;
    file.close();
    sf::Texture texture;

    std::string UIID = jsonData["UIID"];
    std::cout << "UIID: " << UIID << std::endl;

    // Exemple de parcours des boutons
    for (const auto& button : jsonData["buttons"]) {
        buttonHeap.emplace_back(); // Crée un nouvel objet UIButton dans le vecteur
        std::cout << "Button found!" << std::endl;

        std::string buttonID = button["ID"];
        std::string groupID = button["groupID"];
        bool haveText = button["haveText"];
        std::string text = button["text"];
        std::string fontPath = button["font"];
        int fontSize = button["fontSize"];
        std::string color = button["color"];
        int x = button["x"];
        int y = button["y"];
        int width = button["width"];
        int height = button["height"];
        std::string path = button["path"];
        std::string path_hover = button["path_hover"];
        std::string path_pressed = button["path_pressed"];
        std::string effect = button["effect"];
        std::string action = button["action"];
        std::string value = button["value"];
        int layer = button["layer"];
        bool isActive = button["isActive"];  

        if (!buttonHeap.back().button.newButton(buttonID, groupID, haveText, text, fontPath, fontSize, color, x, y, width, height, path, path_hover, path_pressed, effect, action, value, layer, isActive)) {
            std::cerr << "Failed to initialize UIButton for ID: " << buttonID << std::endl;
            buttonHeap.pop_back();
            continue; // Continue avec le prochain élément si celui-ci échoue
        }
    }

    // Parcours des textes
    for (const auto& text : jsonData["text"]) {
        stringHeap.emplace_back(); // Crée un nouvel objet UIString dans le vecteur

        std::string ID = text["ID"];
        std::string groupID = text["groupID"];
        std::string content = text["content"];
        int x = text["x"];
        int y = text["y"];
        int width = text["width"];
        int height = text["height"];
        std::string font = text["font"];
        int fontSize = text["fontSize"];
        std::string color = text["color"];
        std::string effect = text["effect"];
        int layer = text["layer"];
        bool isActive = text["isActive"];

        if (!stringHeap.back().string.newString(ID, groupID, content, x, y, width, height, font, fontSize, color, effect, layer, isActive)) {
            std::cerr << "Failed to initialize UIString for ID: " << ID << std::endl;
            stringHeap.pop_back();
            continue; // Continue avec le prochain élément si celui-ci échoue
        }
    }

    // Exemple de parcours des images
    for (const auto& image : jsonData["images"]) {
        std::string imageID = image.value("ID", "");
        std::string groupID = image.value("groupID", "");
        int x = image.value("x", 0);
        int y = image.value("y", 0);
        int width = image.value("width", 0);
        int height = image.value("height", 0);
        std::string path = image.value("image", "");
        std::string effect = image.value("effect", "");
        int layer = image.value("layer", 0);
        bool isActive = image.value("isActive", false);

        this->graphicHeap.emplace_back();

        if (!this->graphicHeap.back().graphic.initGraphic(imageID, groupID, x, y, width, height, path, effect, layer, isActive)) {
            std::cerr << "Failed to initialize UIGraphic for ID: " << imageID << std::endl;
            this->graphicHeap.pop_back();
            continue;
        }
    }
    return true;
}

bool UIClass::show() {
    int numberOfLayers = 5;

    for (int layer = 0; layer < numberOfLayers; ++layer) {
        for (size_t i = 0; i < this->graphicHeap.size();) {
            if (this->graphicHeap[i].graphic.getIsActive() && this->graphicHeap[i].graphic.getLayer() == layer) {
                if (!this->graphicHeap[i].graphic.show()) {
                    this->graphicHeap.erase(this->graphicHeap.begin() + i);
                    continue;
                }
            }
            ++i;
        }
        for (size_t i = 0; i < this->buttonHeap.size();) {
            if (this->buttonHeap[i].button.getIsActive() && this->buttonHeap[i].button.getLayer() == layer) {
                this->buttonHeap[i].button.event = this->event;
                if (!this->buttonHeap[i].button.show()) {
                    this->buttonHeap.erase(this->buttonHeap.begin() + i);
                    continue;
                }
                bool buttonState = this->buttonHeap[i].button.getButtonState();
                if (buttonState == true) {
                    std::cout << "ID: " << this->buttonHeap[i].button.getID() << " > i : " << i << std::endl;
                    std::cout << "Button pressed!" << std::endl;
                    this->buttonHeap[i].button.resetClick();
                    this->networkManager->Connect(); 
                }
            }
            ++i;
        }
        for (size_t i = 0; i < this->stringHeap.size();) {
            if (this->stringHeap[i].string.getIsActive() && this->stringHeap[i].string.getLayer() == layer) {
                if (!this->stringHeap[i].string.show()) {
                    this->stringHeap.erase(this->stringHeap.begin() + i);
                    continue;
                }
            }
            ++i;
        }
    }
    return true;
}

UIClass::~UIClass() {
}
