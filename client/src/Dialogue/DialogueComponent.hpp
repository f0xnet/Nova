#pragma once

#include <NovaEngine/ECS/Component.hpp>
#include <string>
#include <vector>

namespace NovaEngine {

/**
 * @brief Dialogue Component - Stores NPC dialogue data
 *
 * This custom component extends the ECS system to support dialogue interactions
 */
class DialogueComponent : public Component {
public:
    std::string npcName;
    std::vector<std::string> dialogueLines;
    int currentLine = 0;

    COMPONENT_TYPE_ID(DialogueComponent)

    void serialize(nlohmann::json& json) const override {
        json["npcName"] = npcName;
        json["dialogueLines"] = dialogueLines;
    }

    void deserialize(const nlohmann::json& json) override {
        npcName = json.value("npcName", "NPC");
        if (json.contains("dialogueLines")) {
            dialogueLines = json["dialogueLines"].get<std::vector<std::string>>();
        }
    }
};

} // namespace NovaEngine
