#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include <SFML/Network.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "ButtonEvent.hpp"

class EventHandler {
public:
    std::string button_click = "ui_button_click";
    std::string input_prompt = "ui_input_prompt";


    EventHandler();
    void addEvent(const std::string& event);
    void addCallbackEvent(const std::string& event, const std::function<void(const nlohmann::json&)>& callback);
    void handleEvent(const std::string& event, const std::string& data);
    void handleCallbackEvent(const std::string& event, const nlohmann::json& data);
    void removeEvent(const std::string& event);
    void clearEvents();
    bool isEventRegistered(const std::string& event) const;
    std::vector<std::string> getRegisteredEvents() const;
    ~EventHandler();
private:
    std::map<std::string, std::function<void(const nlohmann::json&)>> events;
    std::unique_ptr<ButtonEvent> buttonEvent;
};

#endif // EVENTHANDLER_HPP
