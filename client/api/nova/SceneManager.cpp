#include "headers/SceneManager.hpp"
#include <fstream>
#include <iostream>

void SceneManager::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " + filename << std::endl;
        return;
    }
    json j;
    file >> j;

    for (const auto& obj : j["scenes"]) {
        Scene scene;
        scene.from_json(obj);
        scenes.push_back(scene);
    }
    
}

void SceneManager::print_scene_details() const {
    for (const auto& scene : scenes) {
        std::cout << "Scene ID: " << scene.id << std::endl;
        std::cout << "Character: " << scene.character << std::endl;

        // Parcours de tous les dialogues de la scène
        for (const auto& dialogue : scene.dialogues) {
            std::cout << "Dialogue: " << dialogue.text << std::endl;
            std::cout << "Condition Attribute: " << dialogue.condition.attribute << std::endl;
            std::cout << "Condition Operator: " << dialogue.condition.operator_ << std::endl;
            std::cout << "Condition Value: " << dialogue.condition.value << std::endl;
        }

        // Parcours de toutes les catégories de réponses de la scène
        for (const auto& [category, responseCategory] : scene.responseCategories) {
            std::cout << "Response Category: " << category << std::endl;
            
            // Parcours de toutes les réponses de la catégorie
            for (const auto& response : responseCategory.responses) {
                std::cout << "Response Text: " << response.text << std::endl;

                // Parcours de toutes les conditions de la réponse
                for (const auto& condition : response.conditions) {
                    std::cout << "Condition Attribute: " << condition.attribute << std::endl;
                    std::cout << "Condition Operator: " << condition.operator_ << std::endl;
                    std::cout << "Condition Value: " << condition.value << std::endl;
                }

                // Parcours de tous les effets de la réponse
                for (const auto& effect : response.effects) {
                    std::cout << "Effect Attribute: " << effect.attribute << std::endl;
                    std::cout << "Effect Change: " << effect.change << std::endl;
                }
            }
        }
    }
}

std::string SceneManager::select_dialogue(const std::string& scene_id, int love_value) const {
    for (const auto& scene : scenes) {
        if (scene.id == scene_id) {
            for (const auto& dialogue : scene.dialogues) {
                bool conditions_met = false; // On initialise à false, puisqu'on va évaluer chaque condition
                if (dialogue.condition.attribute == "love") {
                    if (dialogue.condition.operator_ == ">") {
                        conditions_met = love_value >= dialogue.condition.value;
                    }
                    if (dialogue.condition.operator_ == "<") {
                        conditions_met = love_value <= dialogue.condition.value;
                    }
                    if (dialogue.condition.operator_ == "=") {
                        conditions_met = love_value == dialogue.condition.value;
                    }
                    if (dialogue.condition.operator_ == "!=") {
                        conditions_met = love_value != dialogue.condition.value;
                    }
                    if (dialogue.condition.operator_ == ">=") {
                        conditions_met = love_value >= dialogue.condition.value;
                    }
                    if (dialogue.condition.operator_ == "<=") {
                        conditions_met = love_value <= dialogue.condition.value;
                    }
                }
                // Ajoutez d'autres conditions selon vos besoins

                // Si toutes les conditions sont satisfaites, sélectionnez le dialogue
                if (conditions_met) {
                    return dialogue.text;
                }
            }
        }
    }
    // Si aucune condition n'est satisfaite pour aucun dialogue, retournez une chaîne vide
    return "";
}
//