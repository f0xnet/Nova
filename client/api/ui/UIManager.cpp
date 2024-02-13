#include "headers/UIManager.hpp"
#include <fstream>

UIManager::UIManager() {

}

bool UIManager::Init(NetworkManager* networkManager) {
    if (networkManager == nullptr) {
        std::cerr << "NetworkManager pointer is null" << std::endl;
        return false;
    }
    this->networkManager = networkManager;
    return true;
}

bool UIManager::newUI(const std::string& UIID) {
    uiHeap.emplace_back();
    uiHeap.back().ui.UIID = UIID;
    uiHeap.back().ui.networkManager = this->networkManager;

    // Initialisez l'objet UIClass nouvellement ajouté
    if (!uiHeap.back().ui.Init()) {
        std::cerr << "Failed to initialize UI for UIID: " << UIID << std::endl;
        uiHeap.pop_back(); // Supprimez l'objet si l'initialisation échoue
        return false;
    } else {
        std::cout << "UI created for UIID: " << UIID << std::endl;
        return true;
    }
}

void UIManager::show(sf::Event& event) {
    for (size_t i = 0; i < this->uiHeap.size(); ++i){
        this->uiHeap[i].ui.event = event;
        this->uiHeap[i].ui.show();
    }
}

UIManager::~UIManager() {
    std::cout << "UI destroyed!" << std::endl;
}