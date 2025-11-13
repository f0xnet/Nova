#include "NovaEngine/Events/EventDispatcher.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

void EventDispatcher::subscribe(EventHandler* handler) {
    if (handler) {
        m_handlers.push_back(handler);
        LOG_DEBUG("EventHandler subscribed");
    }
}

void EventDispatcher::unsubscribe(EventHandler* handler) {
    auto it = std::find(m_handlers.begin(), m_handlers.end(), handler);
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
        LOG_DEBUG("EventHandler unsubscribed");
    }
}

void EventDispatcher::dispatch(const Event& event) {
    for (auto* handler : m_handlers) {
        if (handler) {
            handler->onEvent(event);
        }
    }
}

void EventDispatcher::clear() {
    m_handlers.clear();
    LOG_DEBUG("EventDispatcher cleared");
}

}
