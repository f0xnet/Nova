#include "NovaEngine/Events/EventDispatcher.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <algorithm>

namespace NovaEngine {

void EventDispatcher::subscribe(EventHandler* handler) {
    if (!handler) return;
    m_handlers.push_back(handler);
    LOG_DEBUG("EventHandler subscribed ({} total)", m_handlers.size());
}

void EventDispatcher::unsubscribe(EventHandler* handler) {
    auto it = std::find(m_handlers.begin(), m_handlers.end(), handler);
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
        LOG_DEBUG("EventHandler unsubscribed ({} remaining)", m_handlers.size());
    }
}

EventDispatcher::SubscriptionID EventDispatcher::subscribeCallback(std::function<void(const Event&)> callback) {
    if (!callback) return INVALID_SUBSCRIPTION;
    SubscriptionID id = m_nextID++;
    m_callbacks.push_back({id, std::move(callback)});
    LOG_DEBUG("Callback subscribed (id={}, {} total)", id, m_callbacks.size());
    return id;
}

void EventDispatcher::unsubscribe(SubscriptionID id) {
    if (id == INVALID_SUBSCRIPTION) return;
    auto it = std::find_if(m_callbacks.begin(), m_callbacks.end(),
        [id](const CallbackEntry& e) { return e.id == id; });
    if (it != m_callbacks.end()) {
        m_callbacks.erase(it);
        LOG_DEBUG("Callback unsubscribed (id={})", id);
    }
}

void EventDispatcher::dispatch(const Event& event) {
    // Copie snapshot avant dispatch — protège contre subscribe/unsubscribe
    // appelés depuis un handler pendant l'itération (iterator invalidation)
    auto handlers  = m_handlers;
    auto callbacks = m_callbacks;

    for (auto* handler : handlers) {
        if (handler) handler->onEvent(event);
    }
    for (const auto& entry : callbacks) {
        if (entry.callback) entry.callback(event);
    }
}

void EventDispatcher::clear() {
    m_handlers.clear();
    m_callbacks.clear();
    LOG_DEBUG("EventDispatcher cleared");
}

}
