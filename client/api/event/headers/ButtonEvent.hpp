#ifndef BUTTONEVENT_HPP
#define BUTTONEVENT_HPP

#include <nlohmann/json.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include "../../network/headers/NetworkManager.hpp"
//#include "../../ui/headers/UIManager.hpp"

class ButtonEvent {
public:

    ButtonEvent();
    bool Init();
    bool parseEvent(const std::string& data);
    ~ButtonEvent();
private:
    NetworkManager* networkManager;
};

#endif // BUTTONEVENT_HPP