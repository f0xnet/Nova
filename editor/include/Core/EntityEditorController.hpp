#pragma once

#include "EditorState.hpp"
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>

namespace NovaEditor {

/**
 * @brief Controller for entity editing operations
 *
 * Responsibilities:
 * - Entity placement (sprites, lights, NPCs)
 * - Entity selection
 * - Entity dragging
 * - Placement mode management
 *
 * Benefits:
 * - Single Responsibility: Only handles entity operations
 * - Clear separation from EditorApplication
 * - Easier to test and maintain
 */
class EntityEditorController {
public:
    using OnEntityModifiedCallback = std::function<void()>;

    EntityEditorController(
        EditorState* editorState,
        NovaEngine::SceneManager* sceneManager,
        OnEntityModifiedCallback onModified = nullptr
    )
        : m_editorState(editorState)
        , m_sceneManager(sceneManager)
        , m_onEntityModified(onModified)
        , m_draggedEntity(nullptr)
        , m_isDragging(false)
        , m_isPlacementMode(false)
    {}

    /**
     * @brief Load NPC definitions from JSON
     */
    bool loadNPCDefinitions() {
        std::string npcFilePath = "editor/data/definitions/NPCs.json";
        std::ifstream file(npcFilePath);

        if (!file.is_open()) {
            LOG_WARN("Could not open NPC definitions file: {}", npcFilePath);
            return false;
        }

        try {
            nlohmann::json npcData;
            file >> npcData;

            if (npcData.contains("npcs") && npcData["npcs"].is_object()) {
                for (auto& [id, def] : npcData["npcs"].items()) {
                    m_npcDefinitions[id] = def;
                    LOG_DEBUG("Loaded NPC definition: {}", id);
                }
                LOG_INFO("Loaded {} NPC definitions", m_npcDefinitions.size());
                return true;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse NPC definitions: {}", e.what());
        }

        return false;
    }

    /**
     * @brief Enter placement mode for entity type
     */
    void enterPlacementMode(const std::string& entityType, const std::string& entityId) {
        m_placementEntityType = entityType;
        m_placementEntityId = entityId;
        m_isPlacementMode = true;
        LOG_INFO("Entered placement mode: {} ({})", entityType, entityId);
    }

    /**
     * @brief Exit placement mode
     */
    void exitPlacementMode() {
        m_isPlacementMode = false;
        m_placementEntityType.clear();
        m_placementEntityId.clear();
        LOG_INFO("Exited placement mode");
    }

    /**
     * @brief Check if in placement mode
     */
    bool isPlacementMode() const {
        return m_isPlacementMode;
    }

    /**
     * @brief Place entity at world position
     */
    void placeEntity(const NovaEngine::Vec2f& position, NovaEngine::Scene* scene) {
        using namespace NovaEngine;

        if (!scene || !m_isPlacementMode) {
            return;
        }

        try {
            if (m_placementEntityType == "sprites") {
                placeSprite(position, scene);
            }
            else if (m_placementEntityType == "lights") {
                placeLight(position, scene);
            }
            else if (m_placementEntityType == "npcs") {
                placeNPC(position, scene);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while placing entity: {}", e.what());
        }
    }

    /**
     * @brief Select entity
     */
    void selectEntity(NovaEngine::Entity* entity) {
        m_editorState->setSelectedEntity(entity);
        LOG_INFO("Entity selected: ID {}", entity ? entity->getID() : -1);
    }

    /**
     * @brief Start dragging selected entity
     */
    void startDragging(const NovaEngine::Vec2f& worldPos) {
        using namespace NovaEngine;

        Entity* selectedEntity = m_editorState->getSelectedEntity();
        if (!selectedEntity) {
            return;
        }

        auto* transform = selectedEntity->getComponent<TransformComponent>();
        if (!transform) {
            return;
        }

        m_draggedEntity = selectedEntity;
        m_dragOffset = Vec2f{
            transform->position.x - worldPos.x,
            transform->position.y - worldPos.y
        };
        m_isDragging = true;

        LOG_INFO("Started dragging entity ID {}", selectedEntity->getID());
    }

    /**
     * @brief Update dragged entity position
     */
    void updateDragging(const NovaEngine::Vec2f& worldPos) {
        using namespace NovaEngine;

        if (!m_isDragging || !m_draggedEntity) {
            return;
        }

        auto* transform = m_draggedEntity->getComponent<TransformComponent>();
        if (transform) {
            transform->position.x = worldPos.x + m_dragOffset.x;
            transform->position.y = worldPos.y + m_dragOffset.y;
            m_editorState->setSceneModified(true);
        }
    }

    /**
     * @brief Stop dragging entity
     */
    void stopDragging() {
        if (m_isDragging) {
            LOG_INFO("Stopped dragging entity");
            m_isDragging = false;
            m_draggedEntity = nullptr;
            m_dragOffset = NovaEngine::Vec2f{0.0f, 0.0f};
        }
    }

    /**
     * @brief Check if currently dragging
     */
    bool isDragging() const {
        return m_isDragging;
    }

private:
    void placeSprite(const NovaEngine::Vec2f& position, NovaEngine::Scene* scene) {
        using namespace NovaEngine;

        Entity* entity = scene->getEntityRegistry().createEntity();

        auto transform = std::make_unique<TransformComponent>();
        transform->position = position;

        auto sprite = std::make_unique<SpriteComponent>();
        sprite->textureID = m_placementEntityId;

        // Load texture from sprite definition
        const auto* definition = m_sceneManager->getDefinitionManager().getSpriteDefinition(m_placementEntityId);
        if (definition && definition->contains("texture")) {
            std::string texturePath = (*definition)["texture"].get<std::string>();
            sprite->textureHandle = RESOURCES().loadTexture(texturePath);
        }

        sprite->zOrder = m_editorState->getCurrentLayer();

        entity->addComponent(std::move(transform));
        entity->addComponent(std::move(sprite));

        LOG_INFO("Sprite entity created successfully (ID: {})", entity->getID());
        m_editorState->setSceneModified(true);

        if (m_onEntityModified) {
            m_onEntityModified();
        }

        exitPlacementMode();
    }

    void placeLight(const NovaEngine::Vec2f& position, NovaEngine::Scene* scene) {
        using namespace NovaEngine;

        Entity* entity = scene->getEntityRegistry().createEntity();

        auto transform = std::make_unique<TransformComponent>();
        transform->position = position;

        auto light = std::make_unique<LightComponent>();
        light->type = LightComponent::LightType::Point;
        light->color = Color::White;
        light->radius = 200.0f;
        light->intensity = 1.0f;

        // Load from light definition if available
        const auto* definition = m_sceneManager->getDefinitionManager().getLightDefinition(m_placementEntityId);
        if (definition) {
            if (definition->contains("radius")) light->radius = (*definition)["radius"].get<f32>();
            if (definition->contains("intensity")) light->intensity = (*definition)["intensity"].get<f32>();
            if (definition->contains("castShadows")) light->castShadows = (*definition)["castShadows"].get<bool>();
        }

        entity->addComponent(std::move(transform));
        entity->addComponent(std::move(light));

        LOG_INFO("Light entity created successfully (ID: {})", entity->getID());
        m_editorState->setSceneModified(true);

        if (m_onEntityModified) {
            m_onEntityModified();
        }

        exitPlacementMode();
    }

    void placeNPC(const NovaEngine::Vec2f& position, NovaEngine::Scene* scene) {
        using namespace NovaEngine;

        auto it = m_npcDefinitions.find(m_placementEntityId);
        if (it == m_npcDefinitions.end()) {
            LOG_ERROR("NPC definition not found: {}", m_placementEntityId);
            return;
        }

        const auto& definition = it->second;
        Entity* entity = scene->getEntityRegistry().createEntity();

        auto transform = std::make_unique<TransformComponent>();
        transform->position = position;

        auto sprite = std::make_unique<SpriteComponent>();
        sprite->textureID = m_placementEntityId;

        if (definition.contains("sprite") && definition["sprite"].contains("texture")) {
            std::string texturePath = definition["sprite"]["texture"].get<std::string>();
            sprite->textureHandle = RESOURCES().loadTexture(texturePath);
        }

        sprite->zOrder = m_editorState->getCurrentLayer();

        entity->addComponent(std::move(transform));
        entity->addComponent(std::move(sprite));

        LOG_INFO("NPC entity created successfully (ID: {})", entity->getID());
        m_editorState->setSceneModified(true);

        if (m_onEntityModified) {
            m_onEntityModified();
        }

        exitPlacementMode();
    }

private:
    EditorState* m_editorState;
    NovaEngine::SceneManager* m_sceneManager;
    OnEntityModifiedCallback m_onEntityModified;

    // Placement mode
    std::string m_placementEntityType;
    std::string m_placementEntityId;

    // Entity dragging
    NovaEngine::Entity* m_draggedEntity;
    NovaEngine::Vec2f m_dragOffset;
    bool m_isDragging;
    bool m_isPlacementMode;

    // NPC definitions
    std::unordered_map<std::string, nlohmann::json> m_npcDefinitions;
};

} // namespace NovaEditor
