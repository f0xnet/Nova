#pragma once

#include "../ECS/Component.hpp"
#include <sol/sol.hpp>
#include <string>

namespace NovaEngine {

class ScriptComponent : public Component {
public:
    std::string scriptPath;
    bool        enabled = true;

    // Managed by ScriptSystem — not serialized
    sol::environment        env;
    sol::protected_function fnInit;
    sol::protected_function fnUpdate;
    bool loaded  = false;
    bool errored = false;

    COMPONENT_TYPE_ID(ScriptComponent)

    void serialize(nlohmann::json& json) const override {
        json["scriptPath"] = scriptPath;
        json["enabled"]    = enabled;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("scriptPath")) scriptPath = json["scriptPath"];
        if (json.contains("enabled"))    enabled    = json["enabled"];
        loaded  = false;
        errored = false;
    }
};

} // namespace NovaEngine
