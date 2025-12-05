#pragma once

#include "ComponentRegistry.hpp"
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <string>
#include <vector>
#include <memory>

namespace NovaEditor {

/**
 * @brief Adapter pattern for engine integration
 *
 * PROBLEM SOLVED:
 * The editor was tightly coupled to NovaEngine's API. When the engine's API
 * changed, the editor needed updates in many places. This adapter creates
 * a stable interface between editor and engine.
 *
 * BENEFITS:
 * - Engine API changes → Update adapter → Editor keeps working
 * - Single point of contact with engine
 * - Easy to add new engine features
 * - Can swap engine implementation without changing editor code
 *
 * USAGE:
 * Instead of:
 *   scene->getEntityRegistry().createEntity()
 *   RESOURCES().loadTexture(path)
 *
 * Use:
 *   adapter.createEntity(scene)
 *   adapter.loadTexture(path)
 */
class EngineAdapter {
public:
    EngineAdapter(NovaEngine::SceneManager* sceneManager)
        : m_sceneManager(sceneManager)
    {}

    // ===== ENTITY OPERATIONS =====

    /**
     * @brief Create entity in scene
     */
    NovaEngine::Entity* createEntity(NovaEngine::Scene* scene) {
        if (!scene) return nullptr;
        return scene->getEntityRegistry().createEntity();
    }

    /**
     * @brief Destroy entity from scene
     */
    bool destroyEntity(NovaEngine::Scene* scene, NovaEngine::Entity* entity) {
        if (!scene || !entity) return false;
        scene->getEntityRegistry().destroyEntity(entity);
        return true;
    }

    /**
     * @brief Get all entities from scene
     */
    std::vector<NovaEngine::Entity*> getAllEntities(NovaEngine::Scene* scene) {
        if (!scene) return {};
        return scene->getEntityRegistry().getAllEntities();
    }

    // ===== COMPONENT OPERATIONS =====

    /**
     * @brief Add component to entity by type name
     */
    template<typename T>
    T* addComponent(NovaEngine::Entity* entity, std::unique_ptr<T> component) {
        if (!entity) return nullptr;
        auto* ptr = component.get();
        entity->addComponent(std::move(component));
        return ptr;
    }

    /**
     * @brief Get component from entity
     */
    template<typename T>
    T* getComponent(NovaEngine::Entity* entity) {
        if (!entity) return nullptr;
        return entity->getComponent<T>();
    }

    /**
     * @brief Check if entity has component
     */
    template<typename T>
    bool hasComponent(NovaEngine::Entity* entity) {
        if (!entity) return false;
        return entity->getComponent<T>() != nullptr;
    }

    // ===== RESOURCE OPERATIONS =====

    /**
     * @brief Load texture resource
     */
    NovaEngine::TextureHandle loadTexture(const std::string& path) {
        return NovaEngine::RESOURCES().loadTexture(path);
    }

    /**
     * @brief Load font resource
     */
    NovaEngine::FontHandle loadFont(const std::string& path) {
        return NovaEngine::FONTS().loadFont(path);
    }

    /**
     * @brief Load sound resource
     */
    NovaEngine::SoundHandle loadSound(const std::string& path) {
        return NovaEngine::AUDIO().loadSound(path);
    }

    // ===== DEFINITION OPERATIONS =====

    /**
     * @brief Get sprite definition
     */
    const nlohmann::json* getSpriteDefinition(const std::string& id) {
        if (!m_sceneManager) return nullptr;
        return m_sceneManager->getDefinitionManager().getSpriteDefinition(id);
    }

    /**
     * @brief Get light definition
     */
    const nlohmann::json* getLightDefinition(const std::string& id) {
        if (!m_sceneManager) return nullptr;
        return m_sceneManager->getDefinitionManager().getLightDefinition(id);
    }

    /**
     * @brief Get all sprite definition IDs
     */
    std::vector<std::string> getAllSpriteIDs() {
        if (!m_sceneManager) return {};
        return m_sceneManager->getDefinitionManager().getAllSpriteIDs();
    }

    /**
     * @brief Get all light definition IDs
     */
    std::vector<std::string> getAllLightIDs() {
        if (!m_sceneManager) return {};
        return m_sceneManager->getDefinitionManager().getAllLightIDs();
    }

    // ===== SCENE OPERATIONS =====

    /**
     * @brief Load scene from file
     */
    bool loadScene(const std::string& path, const std::string& name) {
        if (!m_sceneManager) return false;
        return m_sceneManager->loadScene(path, name);
    }

    /**
     * @brief Get loaded scene
     */
    NovaEngine::Scene* getScene(const std::string& name) {
        if (!m_sceneManager) return nullptr;
        return m_sceneManager->getScene(name);
    }

    /**
     * @brief Create new scene
     */
    NovaEngine::Scene* createScene(const std::string& name) {
        if (!m_sceneManager) return nullptr;
        // TODO: Add createScene to SceneManager
        return new NovaEngine::Scene(name);
    }

    // ===== RENDERING OPERATIONS =====

    /**
     * @brief Get viewport dimensions
     */
    NovaEngine::IntRect getViewport() {
        return NovaEngine::VIEWPORT();
    }

    /**
     * @brief Convert screen to world coordinates
     */
    NovaEngine::Vec2f screenToWorld(const NovaEngine::Vec2i& screenPos, const NovaEngine::Vec2f& cameraPos, float zoom) {
        auto viewport = getViewport();
        float worldX = (screenPos.x - viewport.width / 2.0f) / zoom + cameraPos.x;
        float worldY = (screenPos.y - viewport.height / 2.0f) / zoom + cameraPos.y;
        return NovaEngine::Vec2f{worldX, worldY};
    }

    // ===== COMPONENT CREATION HELPERS =====

    /**
     * @brief Create Transform component
     */
    std::unique_ptr<NovaEngine::TransformComponent> createTransformComponent(const NovaEngine::Vec2f& position) {
        auto transform = std::make_unique<NovaEngine::TransformComponent>();
        transform->position = position;
        return transform;
    }

    /**
     * @brief Create Sprite component
     */
    std::unique_ptr<NovaEngine::SpriteComponent> createSpriteComponent(
        const std::string& textureID,
        NovaEngine::i32 zOrder = 0
    ) {
        auto sprite = std::make_unique<NovaEngine::SpriteComponent>();
        sprite->textureID = textureID;
        sprite->zOrder = zOrder;

        // Auto-load texture if definition exists
        const auto* def = getSpriteDefinition(textureID);
        if (def && def->contains("texture")) {
            std::string texturePath = (*def)["texture"].get<std::string>();
            sprite->textureHandle = loadTexture(texturePath);
        }

        return sprite;
    }

    /**
     * @brief Create Light component
     */
    std::unique_ptr<NovaEngine::LightComponent> createLightComponent(
        const std::string& lightID = ""
    ) {
        auto light = std::make_unique<NovaEngine::LightComponent>();
        light->type = NovaEngine::LightComponent::LightType::Point;
        light->color = NovaEngine::Color::White;
        light->radius = 200.0f;
        light->intensity = 1.0f;

        // Load from definition if provided
        if (!lightID.empty()) {
            const auto* def = getLightDefinition(lightID);
            if (def) {
                if (def->contains("radius")) light->radius = (*def)["radius"].get<float>();
                if (def->contains("intensity")) light->intensity = (*def)["intensity"].get<float>();
                if (def->contains("castShadows")) light->castShadows = (*def)["castShadows"].get<bool>();
            }
        }

        return light;
    }

private:
    NovaEngine::SceneManager* m_sceneManager;
};

} // namespace NovaEditor
