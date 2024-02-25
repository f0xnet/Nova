#include "headers/UIManager.hpp"
#include <fstream>

extern EventHandler* eventHandlerPtr;

UIManager::UIManager() {
}

bool UIManager::Init() {
    this->eventHandler = std::shared_ptr<EventHandler>(eventHandlerPtr);
    this->eventHandler->addEvent(this->eventHandler->button_click);
    //this->eventHandler->addEvent("prompt_submit");
    return true;
}

bool UIManager::newUI(const std::string& UIID) {
    uiHeap.emplace_back();
    uiHeap.back().ui.UIID = UIID;

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
        if(this->uiHeap[i].ui.getIsActive()){
           this->uiHeap[i].ui.event = event;
           this->uiHeap[i].ui.show();
        }
    }
}

bool UIManager::setGroupID(const std::string& UUIID, const std::string& groupID) {
    for (size_t i = 0; i < this->uiHeap.size(); ++i) {
        if (this->uiHeap[i].ui.UIID == UUIID) {
            this->uiHeap[i].ui.setGroupID(groupID);
            return true;
        }
    }
    return false;
}

UIManager::~UIManager() {
    std::cout << "UI destroyed!" << std::endl;
}