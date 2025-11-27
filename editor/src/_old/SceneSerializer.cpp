#include "../include/SceneSerializer.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace NovaEditor {

bool SceneSerializer::saveScene(NovaEngine::Scene* scene, const std::string& filepath) {
    if (!scene) {
        LOG_ERROR("Cannot save null scene");
        return false;
    }

    LOG_INFO("Saving scene to: {}", filepath);

    try {
        json sceneJson;
        sceneJson["version"] = "1.0";
        sceneJson["name"] = scene->getName();

        // Sérialiser toutes les entités
        json entitiesJson = json::array();
        auto entities = scene->getEntityRegistry().getAllEntities();

        for (auto* entity : entities) {
            entitiesJson.push_back(serializeEntity(entity));
        }

        sceneJson["entities"] = entitiesJson;
        sceneJson["entity_count"] = entities.size();

        // Écrire dans le fichier
        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for writing: {}", filepath);
            return false;
        }

        file << sceneJson.dump(2); // Pretty print avec indentation 2
        file.close();

        LOG_INFO("Scene saved successfully: {} entities", entities.size());
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save scene: {}", e.what());
        return false;
    }
}

bool SceneSerializer::loadScene(NovaEngine::Scene* scene, const std::string& filepath) {
    if (!scene) {
        LOG_ERROR("Cannot load into null scene");
        return false;
    }

    LOG_INFO("Loading scene from: {}", filepath);

    try {
        // Lire le fichier
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for reading: {}", filepath);
            return false;
        }

        json sceneJson;
        file >> sceneJson;
        file.close();

        // Vider la scène actuelle
        // Note: Dans un vrai éditeur, on demanderait confirmation
        auto& registry = scene->getEntityRegistry();
        auto entities = registry.getAllEntities();
        for (auto* entity : entities) {
            registry.destroyEntity(entity->getID());
        }

        // Charger les entités
        if (sceneJson.contains("entities")) {
            for (const auto& entityJson : sceneJson["entities"]) {
                auto* entity = registry.createEntity();
                deserializeEntity(entity, entityJson);
            }
        }

        LOG_INFO("Scene loaded successfully: {} entities", sceneJson["entity_count"].get<int>());
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load scene: {}", e.what());
        return false;
    }
}

std::vector<std::string> SceneSerializer::validateScene(NovaEngine::Scene* scene) {
    std::vector<std::string> issues;

    if (!scene) {
        issues.push_back("ERROR: Scene is null");
        return issues;
    }

    auto entities = scene->getEntityRegistry().getAllEntities();
    int lightCount = 0;

    for (auto* entity : entities) {
        NovaEngine::u64 id = entity->getID();

        // Vérifier Transform
        if (!entity->hasComponent<NovaEngine::TransformComponent>()) {
            issues.push_back("WARNING: Entity " + std::to_string(id) + " has no Transform");
        }

        // Vérifier Sprite
        if (entity->hasComponent<NovaEngine::SpriteComponent>()) {
            auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
            if (sprite->textureHandle == NovaEngine::INVALID_HANDLE && sprite->textureID.empty()) {
                issues.push_back("WARNING: Entity " + std::to_string(id) + " has invalid texture");
            }
        }

        // Compter les lumières (max 8 par scène dans le moteur)
        if (entity->hasComponent<NovaEngine::LightComponent>()) {
            lightCount++;
        }

        // Vérifier colliders
        if (entity->hasComponent<NovaEngine::ColliderComponent>()) {
            auto* collider = entity->getComponent<NovaEngine::ColliderComponent>();
            if (collider->size.x <= 0 || collider->size.y <= 0) {
                issues.push_back("WARNING: Entity " + std::to_string(id) + " has invalid collider size");
            }
        }
    }

    if (lightCount > 8) {
        issues.push_back("ERROR: Scene has " + std::to_string(lightCount) + " lights (max 8)");
    }

    if (entities.empty()) {
        issues.push_back("WARNING: Scene is empty");
    }

    if (issues.empty()) {
        LOG_INFO("Scene validation passed");
    } else {
        LOG_WARN("Scene validation found {} issues", issues.size());
    }

    return issues;
}

bool SceneSerializer::exportEntities(const std::vector<NovaEngine::Entity*>& entities, const std::string& filepath) {
    try {
        json exportJson;
        exportJson["version"] = "1.0";
        exportJson["entity_count"] = entities.size();

        json entitiesJson = json::array();
        for (auto* entity : entities) {
            entitiesJson.push_back(serializeEntity(entity));
        }
        exportJson["entities"] = entitiesJson;

        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for export: {}", filepath);
            return false;
        }

        file << exportJson.dump(2);
        file.close();

        LOG_INFO("Exported {} entities to: {}", entities.size(), filepath);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to export entities: {}", e.what());
        return false;
    }
}

std::vector<NovaEngine::Entity*> SceneSerializer::importEntities(NovaEngine::Scene* scene, const std::string& filepath) {
    std::vector<NovaEngine::Entity*> importedEntities;

    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for import: {}", filepath);
            return importedEntities;
        }

        json importJson;
        file >> importJson;
        file.close();

        if (importJson.contains("entities")) {
            auto& registry = scene->getEntityRegistry();
            for (const auto& entityJson : importJson["entities"]) {
                auto* entity = registry.createEntity();
                deserializeEntity(entity, entityJson);
                importedEntities.push_back(entity);
            }
        }

        LOG_INFO("Imported {} entities from: {}", importedEntities.size(), filepath);
        return importedEntities;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to import entities: {}", e.what());
        return importedEntities;
    }
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

json SceneSerializer::serializeEntity(NovaEngine::Entity* entity) {
    json entityJson;
    entityJson["id"] = entity->getID();

    // Sérialiser tous les composants
    json componentsJson;

    // Transform
    if (entity->hasComponent<NovaEngine::TransformComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::TransformComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Transform"] = compJson;
    }

    // Sprite
    if (entity->hasComponent<NovaEngine::SpriteComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::SpriteComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Sprite"] = compJson;
    }

    // Light
    if (entity->hasComponent<NovaEngine::LightComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::LightComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Light"] = compJson;
    }

    // Animation
    if (entity->hasComponent<NovaEngine::AnimationComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::AnimationComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Animation"] = compJson;
    }

    // Collider
    if (entity->hasComponent<NovaEngine::ColliderComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::ColliderComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Collider"] = compJson;
    }

    // Audio
    if (entity->hasComponent<NovaEngine::AudioComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::AudioComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Audio"] = compJson;
    }

    // Activator
    if (entity->hasComponent<NovaEngine::ActivatorComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::ActivatorComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Activator"] = compJson;
    }

    // Tag
    if (entity->hasComponent<NovaEngine::TagComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::TagComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Tag"] = compJson;
    }

    // SceneTransition
    if (entity->hasComponent<NovaEngine::SceneTransitionComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::SceneTransitionComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["SceneTransition"] = compJson;
    }

    // Shader
    if (entity->hasComponent<NovaEngine::ShaderComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::ShaderComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Shader"] = compJson;
    }

    // Journey
    if (entity->hasComponent<NovaEngine::JourneyComponent>()) {
        auto* comp = entity->getComponent<NovaEngine::JourneyComponent>();
        json compJson;
        comp->serialize(compJson);
        componentsJson["Journey"] = compJson;
    }

    entityJson["components"] = componentsJson;
    return entityJson;
}

void SceneSerializer::deserializeEntity(NovaEngine::Entity* entity, const json& json) {
    if (!json.contains("components")) return;

    const auto& componentsJson = json["components"];

    // Transform
    if (componentsJson.contains("Transform")) {
        auto comp = std::make_unique<NovaEngine::TransformComponent>();
        comp->deserialize(componentsJson["Transform"]);
        entity->addComponent(std::move(comp));
    }

    // Sprite
    if (componentsJson.contains("Sprite")) {
        auto comp = std::make_unique<NovaEngine::SpriteComponent>();
        comp->deserialize(componentsJson["Sprite"]);
        entity->addComponent(std::move(comp));
    }

    // Light
    if (componentsJson.contains("Light")) {
        auto comp = std::make_unique<NovaEngine::LightComponent>();
        comp->deserialize(componentsJson["Light"]);
        entity->addComponent(std::move(comp));
    }

    // Animation
    if (componentsJson.contains("Animation")) {
        auto comp = std::make_unique<NovaEngine::AnimationComponent>();
        comp->deserialize(componentsJson["Animation"]);
        entity->addComponent(std::move(comp));
    }

    // Collider
    if (componentsJson.contains("Collider")) {
        auto comp = std::make_unique<NovaEngine::ColliderComponent>();
        comp->deserialize(componentsJson["Collider"]);
        entity->addComponent(std::move(comp));
    }

    // Audio
    if (componentsJson.contains("Audio")) {
        auto comp = std::make_unique<NovaEngine::AudioComponent>();
        comp->deserialize(componentsJson["Audio"]);
        entity->addComponent(std::move(comp));
    }

    // Activator
    if (componentsJson.contains("Activator")) {
        auto comp = std::make_unique<NovaEngine::ActivatorComponent>();
        comp->deserialize(componentsJson["Activator"]);
        entity->addComponent(std::move(comp));
    }

    // Tag
    if (componentsJson.contains("Tag")) {
        auto comp = std::make_unique<NovaEngine::TagComponent>();
        comp->deserialize(componentsJson["Tag"]);
        entity->addComponent(std::move(comp));
    }

    // SceneTransition
    if (componentsJson.contains("SceneTransition")) {
        auto comp = std::make_unique<NovaEngine::SceneTransitionComponent>();
        comp->deserialize(componentsJson["SceneTransition"]);
        entity->addComponent(std::move(comp));
    }

    // Shader
    if (componentsJson.contains("Shader")) {
        auto comp = std::make_unique<NovaEngine::ShaderComponent>();
        comp->deserialize(componentsJson["Shader"]);
        entity->addComponent(std::move(comp));
    }

    // Journey
    if (componentsJson.contains("Journey")) {
        auto comp = std::make_unique<NovaEngine::JourneyComponent>();
        comp->deserialize(componentsJson["Journey"]);
        entity->addComponent(std::move(comp));
    }
}

} // namespace NovaEditor
