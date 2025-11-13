#pragma once

#include "Event.hpp"
#include "EventHandler.hpp"
#include <vector>
#include <memory>

namespace NovaEngine {

class EventDispatcher {
public:
    void subscribe(EventHandler* handler);
    void unsubscribe(EventHandler* handler);
    void dispatch(const Event& event);
    void clear();
    
private:
    std::vector<EventHandler*> m_handlers;
};

}
