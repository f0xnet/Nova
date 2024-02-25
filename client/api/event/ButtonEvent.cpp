#include "headers/ButtonEvent.hpp"
#include "../ui/headers/UIManager.hpp"

extern NetworkManager* networkManagerPtr;
extern UIManager* uiManagerPtr;

ButtonEvent::ButtonEvent() {
}

bool ButtonEvent::Init() {
    return true;
}

bool ButtonEvent::parseEvent(const std::string& data) {
    nlohmann::json jsonData = nlohmann::json::parse(data);
    std::string action_id = jsonData["action_id"];
    std::string uiid = jsonData["uiid"];

    if(action_id == "connect") {
       networkManagerPtr->Connect();
       uiManagerPtr->setGroupID(uiid, "login_menu_connected");
    }
    if(action_id == "login") {
        networkManagerPtr->Connect();
    }
    return true;
}

ButtonEvent::~ButtonEvent() {
}