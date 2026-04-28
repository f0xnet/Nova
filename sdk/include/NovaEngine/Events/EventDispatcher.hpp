#pragma once

#include "Event.hpp"
#include "EventHandler.hpp"
#include "../Core/Types.hpp"

#include <vector>
#include <functional>

namespace NovaEngine {

class EventDispatcher {
public:
    using SubscriptionID = u64;
    static constexpr SubscriptionID INVALID_SUBSCRIPTION = 0;

    // Interface C++ — pour les systèmes qui implémentent EventHandler
    void subscribe(EventHandler* handler);
    void unsubscribe(EventHandler* handler);

    // Callback — pour les lambdas et (futur) scripting
    // Retourne un ID stable pour désabonnement sans pointeur raw
    SubscriptionID subscribeCallback(std::function<void(const Event&)> callback);
    void unsubscribe(SubscriptionID id);

    void dispatch(const Event& event);
    void clear();

private:
    struct CallbackEntry {
        SubscriptionID id;
        std::function<void(const Event&)> callback;
    };

    std::vector<EventHandler*> m_handlers;
    std::vector<CallbackEntry>  m_callbacks;
    SubscriptionID              m_nextID = 1;
};

}
