#pragma once

#include "../ECS/Component.hpp"
#include <sol/sol.hpp>
#include <string>

namespace NovaEngine {

class ScriptComponent : public Component {
public:
    std::string scriptPath;
    bool        enabled        = true;
    float       updateInterval = 0.0f;  // 0 = every frame, 0.1 = 10fps, etc.

    // Managed by ScriptSystem — not serialized
    sol::environment        env;
    sol::protected_function fnInit;
    sol::protected_function fnUpdate;
    bool  loaded       = false;
    bool  errored      = false;
    float m_updateAccum = 0.0f;

    COMPONENT_TYPE_ID(ScriptComponent)

    void serialize(nlohmann::json& json) const override {
        json["scriptPath"]     = scriptPath;
        json["enabled"]        = enabled;
        json["updateInterval"] = updateInterval;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("scriptPath"))     scriptPath     = json["scriptPath"];
        if (json.contains("enabled"))        enabled        = json["enabled"];
        if (json.contains("updateInterval")) updateInterval = json["updateInterval"];
        loaded       = false;
        errored      = false;
        m_updateAccum = 0.0f;
    }
};

} // namespace NovaEngine
