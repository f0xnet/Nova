#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include <string>

namespace NovaEngine {

enum class EventType {
    Unknown,
    Input,
    UI,
    Engine,
    Custom
};

struct Event {
    EventType type = EventType::Unknown;
    
    // Pour les événements Input (du backend)
    InputEvent inputEvent;
    
    // Pour d'autres types (UI, Custom, etc.)
    std::string name;
    std::string payload;
    
    Event() = default;
    
    Event(const InputEvent& evt)
        : type(EventType::Input), inputEvent(evt) {}
    
    Event(EventType type, const std::string& name, const std::string& payload = "")
        : type(type), name(name), payload(payload) {}
};

}
