#include "headers/EventHandler.hpp"

EventHandler::EventHandler() {
    this->buttonEvent = std::make_unique<ButtonEvent>();
}

void EventHandler::addEvent(const std::string& event) {
    this->events[event] = [](const nlohmann::json&) {};
}

void EventHandler::addCallbackEvent(const std::string& event, const std::function<void(const nlohmann::json&)>& callback) {
    this->events[event] = callback;
    /*Exemple d'utilisation : 
    
    EventHandler handler;

    // Enregistrement d'un événement
    handler.addCallbackEvent("monEvenement", [](const nlohmann::json& data) {
        std::cout << "Événement reçu avec les données : " << data.dump() << std::endl;
    });

    // Déclenchement de l'événement avec des données JSON
    nlohmann::json data = {{"cle", "valeur"}};
    handler.handleCallbackEvent("monEvenement", data);
*/
}

void EventHandler::handleCallbackEvent(const std::string& event, const nlohmann::json& data) {
    if (this->events.find(event) != this->events.end()) {
        this->events[event](data);
    } else {
        std::cerr << "Event not found: " << event << std::endl;
    }
}

void EventHandler::handleEvent(const std::string& event, const std::string& data) {
    if (this->events.find(event) != this->events.end()) {
        if(event == this->button_click){
           this->buttonEvent->parseEvent(data);
        }
    } else {
        std::cerr << "Event not found: " << event << std::endl;
    }
}

void EventHandler::removeEvent(const std::string& event) {
    if (this->events.find(event) != this->events.end()) {
        this->events.erase(event);
    } else {
        std::cerr << "Event not found: " << event << std::endl;
    }
}

void EventHandler::clearEvents() {
    this->events.clear();
}

bool EventHandler::isEventRegistered(const std::string& event) const {
    return this->events.find(event) != this->events.end();
}

std::vector<std::string> EventHandler::getRegisteredEvents() const {
    std::vector<std::string> eventList;
    for (const auto& event : this->events) {
        eventList.push_back(event.first);
    }
    return eventList;
}

EventHandler::~EventHandler() {
    std::cout << "EventHandler destroyed" << std::endl;
}