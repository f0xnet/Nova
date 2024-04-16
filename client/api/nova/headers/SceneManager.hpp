#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

struct Condition {
    std::unordered_map<std::string, std::string> conditions;
};

struct Effect {
    std::unordered_map<std::string, std::string> effects;
};

struct Dialogue {
    std::string text;
    Condition condition;
};

struct Response {
    std::string text;
    Condition conditions;
    Effect effect;
    std::string nextScene;
};

struct SceneData {
    std::string character;
    std::vector<Dialogue> dialogues;
    std::unordered_map<std::string, std::vector<Response>> responses;
};

class SceneManager {
public:
    bool LoadScenes(const std::string& filename);

    SceneData GetScene(const std::string& sceneID) const {
        return scenes_.at(sceneID);
    }

private:
    std::unordered_map<std::string, SceneData> scenes_;

    Condition ParseCondition(const nlohmann::json& conditionData);
    Effect ParseEffect(const nlohmann::json& effectData);
};

#endif // SCENE_MANAGER_HPP
