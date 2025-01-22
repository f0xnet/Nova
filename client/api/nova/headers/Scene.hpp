#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Scene {
public:
    struct Condition {
        std::string attribute;
        std::string operator_;
        int value;

        void from_json(const json& j);
    };

    struct Effect {
        std::string attribute;
        int change;

        void from_json(const json& j);
    };

    struct Response {
        std::string text;
        std::vector<Condition> conditions;
        std::vector<Effect> effects;

        void from_json(const json& j);
    };

    struct ResponseCategory {
        std::vector<Response> responses;

        void from_json(const json& j);
    };

    struct Dialogue {
        std::string text;
        Condition condition;

        void from_json(const json& j);
    };

    std::string id;
    std::string character;
    std::vector<Dialogue> dialogues;
    std::unordered_map<std::string, ResponseCategory> responseCategories;

    void from_json(const json& j);
};
#endif // SCENE_HPP