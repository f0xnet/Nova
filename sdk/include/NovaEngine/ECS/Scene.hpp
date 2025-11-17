#pragma once

#include "EntityRegistry.hpp"
#include "Systems.hpp"
#include "DefinitionManager.hpp"
#include "WaypointGraph.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Backend/BackendManager.hpp"
#include "../Core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>
#include <string>

namespace NovaEngine {

/**
 * @brief A scene contains entities and systems
 *
 * Scenes can be interior rooms, exterior areas, or any logical grouping.
 * They are loaded from JSON files that reference entity definitions.
 */
class Scene {
private:
    std::string m_name;
    std::string m_type;  // "interior" or "exterior"
    Color m_backgroundColor = Color::Black;

    EntityRegistry m_entityRegistry;
    std::vector<std::unique_ptr<System>> m_systems;
    WaypointGraph m_waypointGraph;  // Waypoint-based pathfinding for NPCs

public:
    /**
     * @brief Constructor
     * @param name Scene name
     */
    explicit Scene(const std::string& name) : m_name(name) {
        // Create default systems
        // Order matters: Animation updates before Render, Light renders before Render
        m_systems.push_back(std::make_unique<AnimationSystem>());
        m_systems.push_back(std::make_unique<PhysicsSystem>());
        m_systems.push_back(std::make_unique<ActivatorSystem>());  // Activator system
        m_systems.push_back(std::make_unique<AudioSystem>());
        m_systems.push_back(std::make_unique<LightSystem>());
        m_systems.push_back(std::make_unique<RenderSystem>());  // Render last

        LOG_DEBUG("Created scene: {}", m_name);
    }

    /**
     * @brief Get the scene name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get the scene type
     */
    const std::string& getType() const { return m_type; }

    /**
     * @brief Get the background color
     */
    const Color& getBackgroundColor() const { return m_backgroundColor; }

    /**
     * @brief Load scene from JSON
     * @param sceneData The JSON scene data
     * @param defManager The definition manager to use for entity definitions
     * @return true if successful
     */
    bool loadFromJSON(const nlohmann::json& sceneData, const DefinitionManager& defManager) {
        try {
            // Load scene metadata
            if (sceneData.contains("name")) {
                m_name = sceneData["name"];
            }

            if (sceneData.contains("type")) {
                m_type = sceneData["type"];
            }

            if (sceneData.contains("backgroundColor")) {
                auto& bg = sceneData["backgroundColor"];
                m_backgroundColor = Color{
                    static_cast<u8>(bg[0].get<int>()),
                    static_cast<u8>(bg[1].get<int>()),
                    static_cast<u8>(bg[2].get<int>()),
                    static_cast<u8>(bg[3].get<int>())
                };
            }

            // Load waypoint graph for pathfinding
            if (sceneData.contains("pathfinding")) {
                if (m_waypointGraph.loadFromJSON(sceneData["pathfinding"])) {
                    LOG_INFO("Scene '{}': Waypoint graph loaded ({} waypoints)",
                            m_name, m_waypointGraph.getWaypoints().size());
                }
            }

            // Load entities
            if (sceneData.contains("entities") && sceneData["entities"].is_array()) {
                for (const auto& entityData : sceneData["entities"]) {
                    createEntityFromJSON(entityData, defManager);
                }
            }

            LOG_INFO("Scene '{}' loaded successfully ({} entities)",
                    m_name, m_entityRegistry.getEntityCount());
            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load scene '{}': {}", m_name, e.what());
            return false;
        }
    }

    /**
     * @brief Update all systems except rendering
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime) {
        // Update logic systems only (not RenderSystem)
        for (size_t i = 0; i < m_systems.size() - 1; ++i) {  // -1 to skip RenderSystem (last)
            m_systems[i]->update(deltaTime, m_entityRegistry);
        }
    }

    /**
     * @brief Render the scene
     */
    void render() {
        WINDOW().clear(m_backgroundColor);
        // Call RenderSystem (last system in the list)
        if (!m_systems.empty()) {
            m_systems.back()->update(0.0f, m_entityRegistry);  // RenderSystem is last
        }
    }

    /**
     * @brief Get the entity registry
     */
    EntityRegistry& getEntityRegistry() { return m_entityRegistry; }

    /**
     * @brief Get the entity registry (const)
     */
    const EntityRegistry& getEntityRegistry() const { return m_entityRegistry; }

    /**
     * @brief Get the waypoint graph
     */
    WaypointGraph& getWaypointGraph() { return m_waypointGraph; }

    /**
     * @brief Get the waypoint graph (const)
     */
    const WaypointGraph& getWaypointGraph() const { return m_waypointGraph; }

    /**
     * @brief Find a path between two positions in this scene using waypoints
     * @param startPos Starting position
     * @param endPos Ending position
     * @param preferredTags Preferred path tags for personality (optional)
     * @return Vector of waypoint positions forming the path
     */
    std::vector<Vec2f> findPath(const Vec2f& startPos, const Vec2f& endPos,
                               const std::vector<std::string>& preferredTags = {}) {
        return m_waypointGraph.findPath(startPos, endPos, preferredTags);
    }

private:
    /**
     * @brief Create an entity from JSON data
     */
    void createEntityFromJSON(const nlohmann::json& entityData, const DefinitionManager& defManager) {
        if (!entityData.contains("type")) {
            LOG_ERROR("Entity missing 'type' field");
            return;
        }

        std::string type = entityData["type"];

        // Create entity
        Entity* entity = m_entityRegistry.createEntity();

        // Always add Transform component (required for all entities)
        auto transform = std::make_unique<TransformComponent>();
        if (entityData.contains("position")) {
            transform->position = Vec2f{
                entityData["position"][0].get<f32>(),
                entityData["position"][1].get<f32>()
            };
        }
        if (entityData.contains("rotation")) {
            transform->rotation = entityData["rotation"].get<f32>();
        }
        if (entityData.contains("scale")) {
            transform->scale = Vec2f{
                entityData["scale"][0].get<f32>(),
                entityData["scale"][1].get<f32>()
            };
        }
        entity->addComponent(std::move(transform));

        // Create type-specific components
        if (type == "sprite") {
            createSpriteEntity(entity, entityData, defManager);
        }
        else if (type == "light") {
            createLightEntity(entity, entityData, defManager);
        }
        else if (type == "animated_sprite") {
            createAnimatedSpriteEntity(entity, entityData, defManager);
        }
        else if (type == "audio") {
            createAudioEntity(entity, entityData, defManager);
        }
        else if (type == "activator") {
            createActivatorEntity(entity, entityData, defManager);
        }
        else if (type == "player") {
            createPlayerEntity(entity, entityData, defManager);
        }
        else {
            LOG_WARN("Unknown entity type: {}", type);
        }
    }

    /**
     * @brief Create a sprite entity
     */
    void createSpriteEntity(Entity* entity, const nlohmann::json& entityData,
                            const DefinitionManager& defManager) {
        if (!entityData.contains("spriteID")) {
            LOG_ERROR("Sprite entity missing 'spriteID'");
            return;
        }

        ID spriteID = entityData["spriteID"];
        const nlohmann::json* spriteDef = defManager.getSpriteDefinition(spriteID);

        if (!spriteDef) {
            LOG_ERROR("Sprite definition not found: {}", spriteID);
            return;
        }

        // Create sprite component from definition
        auto sprite = std::make_unique<SpriteComponent>();
        sprite->textureID = (*spriteDef)["texture"];

        // Load texture and get handle
        // Note: The texture path should be specified in the definition
        if (spriteDef->contains("texturePath")) {
            std::string texturePath = (*spriteDef)["texturePath"];
            sprite->textureHandle = RESOURCES().loadTexture(texturePath);
            if (sprite->textureHandle == INVALID_HANDLE) {
                LOG_WARN("Failed to load texture for sprite '{}': {}", spriteID, texturePath);
            }
        }

        if (spriteDef->contains("textureRect")) {
            auto& rect = (*spriteDef)["textureRect"];
            sprite->textureRect = IntRect{
                rect[0].get<i32>(), rect[1].get<i32>(),
                rect[2].get<i32>(), rect[3].get<i32>()
            };
        }

        if (spriteDef->contains("size")) {
            auto& size = (*spriteDef)["size"];
            sprite->size = Vec2f{size[0].get<f32>(), size[1].get<f32>()};
        }

        if (spriteDef->contains("scale")) {
            auto& scale = (*spriteDef)["scale"];
            auto* transform = entity->getComponent<TransformComponent>();
            transform->scale = Vec2f{scale[0].get<f32>(), scale[1].get<f32>()};
        }

        if (spriteDef->contains("origin")) {
            auto& origin = (*spriteDef)["origin"];
            auto* transform = entity->getComponent<TransformComponent>();
            transform->origin = Vec2f{origin[0].get<f32>(), origin[1].get<f32>()};
        }

        if (spriteDef->contains("zOrder")) {
            sprite->zOrder = (*spriteDef)["zOrder"].get<i32>();
        }

        // Allow scene-specific overrides
        if (entityData.contains("zOrder")) {
            sprite->zOrder = entityData["zOrder"].get<i32>();
        }
        if (entityData.contains("size")) {
            auto& size = entityData["size"];
            sprite->size = Vec2f{size[0].get<f32>(), size[1].get<f32>()};
        }
        if (entityData.contains("scale")) {
            auto& scale = entityData["scale"];
            auto* transform = entity->getComponent<TransformComponent>();
            transform->scale = Vec2f{scale[0].get<f32>(), scale[1].get<f32>()};
        }

        entity->addComponent(std::move(sprite));
        LOG_DEBUG("Created sprite entity (ID: {}, sprite: {})", entity->getID(), spriteID);
    }

    /**
     * @brief Create a light entity
     */
    void createLightEntity(Entity* entity, const nlohmann::json& entityData,
                           const DefinitionManager& defManager) {
        if (!entityData.contains("lightID")) {
            LOG_ERROR("Light entity missing 'lightID'");
            return;
        }

        ID lightID = entityData["lightID"];
        const nlohmann::json* lightDef = defManager.getLightDefinition(lightID);

        if (!lightDef) {
            LOG_ERROR("Light definition not found: {}", lightID);
            return;
        }

        // Create light component from definition
        auto light = std::make_unique<LightComponent>();

        std::string typeStr = (*lightDef)["type"];
        if (typeStr == "point") light->type = LightComponent::LightType::Point;
        else if (typeStr == "directional") light->type = LightComponent::LightType::Directional;
        else if (typeStr == "spot") light->type = LightComponent::LightType::Spot;

        if (lightDef->contains("color")) {
            auto& color = (*lightDef)["color"];
            light->color = Color{
                static_cast<u8>(color[0].get<int>()),
                static_cast<u8>(color[1].get<int>()),
                static_cast<u8>(color[2].get<int>()),
                static_cast<u8>(color[3].get<int>())
            };
        }

        if (lightDef->contains("radius")) {
            light->radius = (*lightDef)["radius"].get<f32>();
        }

        if (lightDef->contains("intensity")) {
            light->intensity = (*lightDef)["intensity"].get<f32>();
        }

        if (lightDef->contains("direction")) {
            auto& dir = (*lightDef)["direction"];
            light->direction = Vec2f{dir[0].get<f32>(), dir[1].get<f32>()};
        }

        if (lightDef->contains("angle")) {
            light->angle = (*lightDef)["angle"].get<f32>();
        }

        if (lightDef->contains("castShadows")) {
            light->castShadows = (*lightDef)["castShadows"].get<bool>();
        }

        entity->addComponent(std::move(light));
        LOG_DEBUG("Created light entity (ID: {}, light: {})", entity->getID(), lightID);
    }

    /**
     * @brief Create an animated sprite entity
     */
    void createAnimatedSpriteEntity(Entity* entity, const nlohmann::json& entityData,
                                    const DefinitionManager& defManager) {
        // First create the sprite
        createSpriteEntity(entity, entityData, defManager);

        // Then add animation
        if (!entityData.contains("animationID")) {
            LOG_ERROR("Animated sprite entity missing 'animationID'");
            return;
        }

        ID animID = entityData["animationID"];
        const nlohmann::json* animDef = defManager.getAnimationDefinition(animID);

        if (!animDef) {
            LOG_ERROR("Animation definition not found: {}", animID);
            return;
        }

        auto animation = std::make_unique<AnimationComponent>();
        animation->animationID = animID;

        if (animDef->contains("frames")) {
            for (const auto& frame : (*animDef)["frames"]) {
                animation->frames.push_back(IntRect{
                    frame[0].get<i32>(), frame[1].get<i32>(),
                    frame[2].get<i32>(), frame[3].get<i32>()
                });
            }
        }

        if (animDef->contains("frameDuration")) {
            animation->frameDuration = (*animDef)["frameDuration"].get<f32>();
        }

        if (animDef->contains("loop")) {
            animation->loop = (*animDef)["loop"].get<bool>();
        }

        entity->addComponent(std::move(animation));
        LOG_DEBUG("Created animated sprite entity (ID: {}, animation: {})",
                 entity->getID(), animID);
    }

    /**
     * @brief Create an audio entity
     */
    void createAudioEntity(Entity* entity, const nlohmann::json& entityData,
                           const DefinitionManager& defManager) {
        if (!entityData.contains("audioID")) {
            LOG_ERROR("Audio entity missing 'audioID'");
            return;
        }

        ID audioID = entityData["audioID"];
        const nlohmann::json* audioDef = defManager.getAudioDefinition(audioID);

        if (!audioDef) {
            LOG_ERROR("Audio definition not found: {}", audioID);
            return;
        }

        auto audio = std::make_unique<AudioComponent>();
        audio->soundID = audioID;

        if (audioDef->contains("loop")) {
            audio->loop = (*audioDef)["loop"].get<bool>();
        }

        if (audioDef->contains("volume")) {
            audio->volume = (*audioDef)["volume"].get<f32>();
        }

        if (audioDef->contains("playOnStart")) {
            audio->playOnStart = (*audioDef)["playOnStart"].get<bool>();
        }

        // Allow scene overrides
        if (entityData.contains("playOnStart")) {
            audio->playOnStart = entityData["playOnStart"].get<bool>();
        }

        entity->addComponent(std::move(audio));
        LOG_DEBUG("Created audio entity (ID: {}, audio: {})", entity->getID(), audioID);
    }

    /**
     * @brief Create an activator entity
     */
    void createActivatorEntity(Entity* entity, const nlohmann::json& entityData,
                               const DefinitionManager& defManager) {
        if (!entityData.contains("activatorID")) {
            LOG_ERROR("Activator entity missing 'activatorID'");
            return;
        }

        ID activatorID = entityData["activatorID"];
        const nlohmann::json* activatorDef = defManager.getActivatorDefinition(activatorID);

        if (!activatorDef) {
            LOG_ERROR("Activator definition not found: {}", activatorID);
            return;
        }

        // Create activator component from definition
        auto activator = std::make_unique<ActivatorComponent>();
        activator->actionID = activatorID;

        // Load type
        if (activatorDef->contains("type")) {
            std::string typeStr = (*activatorDef)["type"];
            if (typeStr == "proximity") activator->type = ActivatorComponent::ActivatorType::Proximity;
            else if (typeStr == "manual") activator->type = ActivatorComponent::ActivatorType::Manual;
            else if (typeStr == "automatic") activator->type = ActivatorComponent::ActivatorType::Automatic;
        }

        // Load shape
        if (activatorDef->contains("shape")) {
            std::string shapeStr = (*activatorDef)["shape"];
            activator->shape = (shapeStr == "box") ?
                ActivatorComponent::ActivatorShape::Box :
                ActivatorComponent::ActivatorShape::Circle;
        }

        // Load size/radius
        if (activatorDef->contains("size")) {
            auto& size = (*activatorDef)["size"];
            activator->size = Vec2f{size[0].get<f32>(), size[1].get<f32>()};
        }
        if (activatorDef->contains("radius")) {
            activator->radius = (*activatorDef)["radius"].get<f32>();
        }

        // Load properties
        if (activatorDef->contains("canReactivate")) {
            activator->canReactivate = (*activatorDef)["canReactivate"].get<bool>();
        }
        if (activatorDef->contains("cooldownTime")) {
            activator->cooldownTime = (*activatorDef)["cooldownTime"].get<f32>();
        }
        if (activatorDef->contains("targetTag")) {
            activator->targetTag = (*activatorDef)["targetTag"];
        }
        if (activatorDef->contains("showDebugZone")) {
            activator->showDebugZone = (*activatorDef)["showDebugZone"].get<bool>();
        }
        if (activatorDef->contains("onActivateEvent")) {
            activator->onActivateEvent = (*activatorDef)["onActivateEvent"];
        }
        if (activatorDef->contains("onDeactivateEvent")) {
            activator->onDeactivateEvent = (*activatorDef)["onDeactivateEvent"];
        }

        // Allow scene overrides
        if (entityData.contains("targetTag")) {
            activator->targetTag = entityData["targetTag"];
        }

        entity->addComponent(std::move(activator));
        LOG_DEBUG("Created activator entity (ID: {}, activator: {})", entity->getID(), activatorID);
    }

    /**
     * @brief Create a player entity
     */
    void createPlayerEntity(Entity* entity, const nlohmann::json& entityData,
                            const DefinitionManager& defManager) {
        // Add player tag
        auto tag = std::make_unique<TagComponent>();
        tag->tag = "player";
        entity->addComponent(std::move(tag));

        // If player also has a sprite, load it
        if (entityData.contains("spriteID")) {
            createSpriteEntity(entity, entityData, defManager);
        }

        LOG_DEBUG("Created player entity (ID: {})", entity->getID());
    }
};

// ============================================================================
// JourneySystem::calculateLocalWaypointPath implementation
// ============================================================================

inline void JourneySystem::calculateLocalWaypointPath(Entity* entity, Scene* currentScene) {
    if (!entity || !currentScene) return;

    auto* journey = entity->getComponent<JourneyComponent>();
    auto* transform = entity->getComponent<TransformComponent>();

    if (!journey || !transform) return;
    if (!journey->isOnJourney) return;

    // Obtenir le waypoint graph de la scène
    auto& waypointGraph = currentScene->getWaypointGraph();

    if (waypointGraph.isEmpty()) {
        // Pas de waypoints dans cette scène, utiliser navigation directe
        journey->localWaypointPath.clear();
        journey->currentLocalWaypointIndex = 0;
        LOG_DEBUG("NPC {}: No waypoint graph in scene, using direct navigation", entity->getID());
        return;
    }

    // Calculer le chemin de waypoints
    journey->localWaypointPath = waypointGraph.findPath(
        transform->position,
        journey->currentDestination,
        journey->preferredPathTags
    );

    journey->currentLocalWaypointIndex = 0;

    if (!journey->localWaypointPath.empty()) {
        LOG_INFO("NPC {}: Calculated waypoint path with {} waypoints (tags: {})",
                entity->getID(),
                journey->localWaypointPath.size(),
                journey->preferredPathTags.empty() ? "none" : journey->preferredPathTags[0]);

        // Log le chemin pour debug
        for (size_t i = 0; i < journey->localWaypointPath.size(); ++i) {
            LOG_DEBUG("  Waypoint {}: ({:.1f}, {:.1f})",
                     i, journey->localWaypointPath[i].x, journey->localWaypointPath[i].y);
        }
    } else {
        LOG_WARN("NPC {}: Could not find waypoint path, using direct navigation",
                entity->getID());
    }
}

} // namespace NovaEngine
