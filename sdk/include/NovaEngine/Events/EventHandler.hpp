#pragma once

#include "Event.hpp"

namespace NovaEngine {

class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void onEvent(const Event& event) = 0;
};

}
