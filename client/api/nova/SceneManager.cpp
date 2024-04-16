#include "headers/SceneManager.hpp"

bool SceneManager::LoadScenes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open JSON file!" << std::endl;
        return false;
    }

    nlohmann::json jsonData;
    file >> jsonData;
    file.close();

    for (const auto& scene : jsonData["scenes"]) {
        std::string sceneID = scene["id"];
        std::string character = scene["character"];
        std::vector<Dialogue> dialogues;
        std::unordered_map<std::string, std::vector<Response>> responses;

        for (const auto& dialogue : scene["dialogues"]) {
            std::string text = dialogue["text"];
            Condition condition = ParseCondition(dialogue["condition"]);
            dialogues.push_back({text, condition});
        }

        for (const auto& response : scene["responses"].items()) {
            std::vector<Response> responseList;
            for (const auto& option : response.value()) {
                std::string text = option["text"];
                Condition condition = ParseCondition(option["conditions"]);
                Effect effect = ParseEffect(option["effects"]);
                std::string nextScene = option.value("nextScene", "");
                responseList.push_back({text, condition, effect, nextScene});
            }
            responses[response.key()] = responseList;
        }

        scenes_[sceneID] = SceneData{character, dialogues, responses};
    }

    return true;
}

Condition SceneManager::ParseCondition(const nlohmann::json& conditionData) {
    Condition condition;
    for (const auto& item : conditionData.items()) {
        std::string key = item.key();
        std::string value = item.value();
        condition.conditions[key] = value;
    }
    return condition;
}

Effect SceneManager::ParseEffect(const nlohmann::json& effectData) {
    Effect effect;
    for (const auto& item : effectData.items()) {
        std::string key = item.key();
        std::string value = item.value();
        effect.effects[key] = value;
    }
    return effect;
}