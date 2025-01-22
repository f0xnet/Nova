#include "headers/Scene.hpp"
#include <iostream>

void Scene::Condition::from_json(const json& j) {
    attribute = j.begin().key();
    auto val = j.begin().value();
    if (val.is_array() && val.size() == 2 && val[0].is_string() && val[1].is_number()) {
        operator_ = val[0].get<std::string>();
        value = val[1].get<int>();
    } else {
        std::cerr << "Error: Condition values are not formatted correctly for '" << attribute << "'.\n";
    }
}

void Scene::Effect::from_json(const json& j) {
    attribute = j.begin().key();
    auto val = j.begin().value();
    if (val.is_array() && val.size() == 2 && val[1].is_number()) {
        change = val[1].get<int>();
    } else {
        std::cerr << "Error: Effect values are not formatted correctly for '" << attribute << "'.\n";
    }
}

void Scene::Response::from_json(const json& j) {
    j.at("text").get_to(text);
    if (j.contains("conditions") && j["conditions"].is_array()) {
        for (const auto& item : j["conditions"]) {
            Condition condition;
            condition.from_json(item);
            conditions.push_back(condition);
        }
    }

    if (j.contains("effects") && j["effects"].is_array()) {
        for (const auto& item : j["effects"]) {
            Effect effect;
            effect.from_json(item);
            effects.push_back(effect);
        }
    }
}

void Scene::ResponseCategory::from_json(const json& j) {
    if (j.is_array()) {
        for (const auto& resp : j) {
            Response response;
            response.from_json(resp);
            responses.push_back(response);
        }
    }
}

void Scene::Dialogue::from_json(const json& j) {
    j.at("text").get_to(text);
    condition.from_json(j.at("condition"));
}

void Scene::from_json(const json& j) {
    j.at("id").get_to(id);
    j.at("character").get_to(character);

    for (const auto& dlg : j["dialogues"]) {
        Dialogue dialogue;
        dialogue.from_json(dlg);
        dialogues.push_back(dialogue);
    }

    if (j.contains("responses") && j["responses"].is_object()) {
        for (const auto& [key, category] : j["responses"].items()) {
            ResponseCategory responseCategory;
            responseCategory.from_json(category);
            responseCategories[key] = responseCategory;
        }
    }
}