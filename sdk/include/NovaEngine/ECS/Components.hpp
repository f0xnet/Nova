#pragma once

#include "Component.hpp"
#include "../Core/Types.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Backend/Interfaces/IGraphicsBackend.hpp"
#include <vector>

namespace NovaEngine {

// ============================================================================
// TransformComponent - Position, rotation, scale
// ============================================================================

/**
 * @brief Transform component - position, rotation, and scale
 *
 * This component is required for any entity that has a position in the world.
 */
class TransformComponent : public Component {
public:
    Vec2f position = {0.0f, 0.0f};
    f32 rotation = 0.0f;
    Vec2f scale = {1.0f, 1.0f};
    Vec2f origin = {0.0f, 0.0f};

    COMPONENT_TYPE_ID(TransformComponent)

    void serialize(nlohmann::json& json) const override {
        json["position"] = {position.x, position.y};
        json["rotation"] = rotation;
        json["scale"] = {scale.x, scale.y};
        json["origin"] = {origin.x, origin.y};
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("position")) {
            position = Vec2f{json["position"][0], json["position"][1]};
        }
        if (json.contains("rotation")) {
            rotation = json["rotation"];
        }
        if (json.contains("scale")) {
            scale = Vec2f{json["scale"][0], json["scale"][1]};
        }
        if (json.contains("origin")) {
            origin = Vec2f{json["origin"][0], json["origin"][1]};
        }
    }
};

// ============================================================================
// SpriteComponent - 2D sprite rendering
// ============================================================================

/**
 * @brief Sprite component - 2D texture rendering
 */
class SpriteComponent : public Component {
public:
    ID textureID;                           // ID in ResourceManager (for definitions)
    TextureHandle textureHandle = INVALID_HANDLE;  // Actual texture handle for rendering
    IntRect textureRect = {0, 0, 0, 0};     // Sub-rectangle of texture (0,0,0,0 = full texture)
    Vec2f size = {0.0f, 0.0f};              // Display size (0,0 = native texture size)
    Color tint = Color::White;
    BlendMode blendMode = BlendMode::Alpha;
    i32 zOrder = 0;                         // Render order (lower = behind)
    bool visible = true;

    COMPONENT_TYPE_ID(SpriteComponent)

    void serialize(nlohmann::json& json) const override {
        json["textureID"] = textureID;
        json["textureRect"] = {textureRect.left, textureRect.top, textureRect.width, textureRect.height};
        json["size"] = {size.x, size.y};
        json["tint"] = {tint.r, tint.g, tint.b, tint.a};
        json["zOrder"] = zOrder;
        json["visible"] = visible;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("textureID")) {
            textureID = json["textureID"];
        }
        if (json.contains("textureRect")) {
            auto& rect = json["textureRect"];
            textureRect = IntRect{rect[0], rect[1], rect[2], rect[3]};
        }
        if (json.contains("size")) {
            size = Vec2f{json["size"][0], json["size"][1]};
        }
        if (json.contains("tint")) {
            auto& color = json["tint"];
            tint = Color{static_cast<u8>(color[0]), static_cast<u8>(color[1]),
                        static_cast<u8>(color[2]), static_cast<u8>(color[3])};
        }
        if (json.contains("zOrder")) {
            zOrder = json["zOrder"];
        }
        if (json.contains("visible")) {
            visible = json["visible"];
        }
    }
};

// ============================================================================
// LightComponent - Light source
// ============================================================================

/**
 * @brief Light component - point, directional, or spot light
 */
class LightComponent : public Component {
public:
    enum class LightType { Point, Directional, Spot };

    LightType type = LightType::Point;
    Color color = Color::White;
    f32 radius = 100.0f;           // For Point/Spot lights
    f32 intensity = 1.0f;          // 0.0 to 1.0+
    Vec2f direction = {0, 0};      // For Directional/Spot lights
    f32 angle = 45.0f;             // For Spot lights (in degrees)
    bool castShadows = false;
    bool enabled = true;

    COMPONENT_TYPE_ID(LightComponent)

    void serialize(nlohmann::json& json) const override {
        std::string typeStr;
        switch (type) {
            case LightType::Point: typeStr = "point"; break;
            case LightType::Directional: typeStr = "directional"; break;
            case LightType::Spot: typeStr = "spot"; break;
        }
        json["type"] = typeStr;
        json["color"] = {color.r, color.g, color.b, color.a};
        json["radius"] = radius;
        json["intensity"] = intensity;
        json["direction"] = {direction.x, direction.y};
        json["angle"] = angle;
        json["castShadows"] = castShadows;
        json["enabled"] = enabled;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("type")) {
            std::string typeStr = json["type"];
            if (typeStr == "point") type = LightType::Point;
            else if (typeStr == "directional") type = LightType::Directional;
            else if (typeStr == "spot") type = LightType::Spot;
        }
        if (json.contains("color")) {
            auto& c = json["color"];
            color = Color{static_cast<u8>(c[0]), static_cast<u8>(c[1]),
                         static_cast<u8>(c[2]), static_cast<u8>(c[3])};
        }
        if (json.contains("radius")) radius = json["radius"];
        if (json.contains("intensity")) intensity = json["intensity"];
        if (json.contains("direction")) {
            auto& dir = json["direction"];
            direction = Vec2f{dir[0], dir[1]};
        }
        if (json.contains("angle")) angle = json["angle"];
        if (json.contains("castShadows")) castShadows = json["castShadows"];
        if (json.contains("enabled")) enabled = json["enabled"];
    }
};

// ============================================================================
// AnimationComponent - Frame-based animation
// ============================================================================

/**
 * @brief Animation component - frame-based sprite animation
 */
class AnimationComponent : public Component {
public:
    ID animationID;                         // Reference to animation definition
    std::vector<IntRect> frames;            // Frame rectangles
    f32 frameDuration = 0.1f;               // Time per frame in seconds
    f32 currentTime = 0.0f;                 // Current animation time
    u32 currentFrame = 0;                   // Current frame index
    bool loop = true;
    bool playing = true;

    COMPONENT_TYPE_ID(AnimationComponent)

    void serialize(nlohmann::json& json) const override {
        json["animationID"] = animationID;
        nlohmann::json framesJson = nlohmann::json::array();
        for (const auto& frame : frames) {
            framesJson.push_back({frame.left, frame.top, frame.width, frame.height});
        }
        json["frames"] = framesJson;
        json["frameDuration"] = frameDuration;
        json["currentFrame"] = currentFrame;
        json["loop"] = loop;
        json["playing"] = playing;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("animationID")) {
            animationID = json["animationID"];
        }
        if (json.contains("frames")) {
            frames.clear();
            for (const auto& frame : json["frames"]) {
                frames.push_back(IntRect{frame[0], frame[1], frame[2], frame[3]});
            }
        }
        if (json.contains("frameDuration")) frameDuration = json["frameDuration"];
        if (json.contains("currentFrame")) currentFrame = json["currentFrame"];
        if (json.contains("loop")) loop = json["loop"];
        if (json.contains("playing")) playing = json["playing"];
    }
};

// ============================================================================
// ColliderComponent - Physics collision
// ============================================================================

/**
 * @brief Collider component - box or circle collision
 */
class ColliderComponent : public Component {
public:
    enum class ColliderType { Box, Circle };

    ColliderType type = ColliderType::Box;
    Vec2f size = {0.0f, 0.0f};     // For Box collider
    f32 radius = 0.0f;             // For Circle collider
    Vec2f offset = {0.0f, 0.0f};   // Offset from entity position
    bool isTrigger = false;        // Trigger (no physics) or solid collider
    bool enabled = true;

    COMPONENT_TYPE_ID(ColliderComponent)

    void serialize(nlohmann::json& json) const override {
        json["type"] = (type == ColliderType::Box) ? "box" : "circle";
        json["size"] = {size.x, size.y};
        json["radius"] = radius;
        json["offset"] = {offset.x, offset.y};
        json["isTrigger"] = isTrigger;
        json["enabled"] = enabled;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("type")) {
            type = (json["type"] == "box") ? ColliderType::Box : ColliderType::Circle;
        }
        if (json.contains("size")) {
            size = Vec2f{json["size"][0], json["size"][1]};
        }
        if (json.contains("radius")) radius = json["radius"];
        if (json.contains("offset")) {
            offset = Vec2f{json["offset"][0], json["offset"][1]};
        }
        if (json.contains("isTrigger")) isTrigger = json["isTrigger"];
        if (json.contains("enabled")) enabled = json["enabled"];
    }
};

// ============================================================================
// AudioComponent - Sound/Music playback
// ============================================================================

/**
 * @brief Audio component - sound or music playback
 */
class AudioComponent : public Component {
public:
    ID soundID;                    // ID in ResourceManager (for definitions)
    SoundHandle soundHandle = INVALID_HANDLE;  // Actual sound handle for playback
    bool playOnStart = false;
    bool loop = false;
    f32 volume = 100.0f;
    f32 pitch = 1.0f;
    bool playing = false;

    COMPONENT_TYPE_ID(AudioComponent)

    void serialize(nlohmann::json& json) const override {
        json["soundID"] = soundID;
        json["playOnStart"] = playOnStart;
        json["loop"] = loop;
        json["volume"] = volume;
        json["pitch"] = pitch;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("soundID")) soundID = json["soundID"];
        if (json.contains("playOnStart")) playOnStart = json["playOnStart"];
        if (json.contains("loop")) loop = json["loop"];
        if (json.contains("volume")) volume = json["volume"];
        if (json.contains("pitch")) pitch = json["pitch"];
    }
};

// ============================================================================
// ActivatorComponent - Trigger zone activation
// ============================================================================

/**
 * @brief Activator component - trigger zone that activates on entry
 *
 * This component creates a zone that can be activated when another entity
 * (typically the player) enters its area. Common uses: switches, doors,
 * pressure plates, trigger zones, interactive objects.
 */
class ActivatorComponent : public Component {
public:
    enum class ActivatorType {
        Proximity,      // Activates when entity enters zone
        Manual,         // Requires manual activation (key press)
        Automatic       // Activates continuously while entity is in zone
    };

    enum class ActivatorShape {
        Box,            // Rectangular activation zone
        Circle          // Circular activation zone
    };

    ActivatorType type = ActivatorType::Proximity;
    ActivatorShape shape = ActivatorShape::Box;

    Vec2f size = {100.0f, 100.0f};     // For Box shape
    f32 radius = 50.0f;                // For Circle shape
    Vec2f offset = {0.0f, 0.0f};       // Offset from entity position

    bool isActive = false;             // Current activation state
    bool canReactivate = true;         // Can be activated multiple times
    f32 cooldownTime = 0.0f;           // Time before can reactivate (seconds)
    f32 currentCooldown = 0.0f;        // Current cooldown timer

    std::string targetTag = "player";  // Which entity tag can activate this
    std::string actionID;              // ID of action to trigger (for game logic)

    // Visual feedback
    bool showDebugZone = false;        // Draw activation zone for debugging
    Color debugColor = Color{0, 255, 0, 100};  // Debug visualization color

    // Callback data (for custom logic)
    std::string onActivateEvent;       // Event name to fire on activation
    std::string onDeactivateEvent;     // Event name to fire on deactivation

    COMPONENT_TYPE_ID(ActivatorComponent)

    void serialize(nlohmann::json& json) const override {
        std::string typeStr;
        switch (type) {
            case ActivatorType::Proximity: typeStr = "proximity"; break;
            case ActivatorType::Manual: typeStr = "manual"; break;
            case ActivatorType::Automatic: typeStr = "automatic"; break;
        }

        std::string shapeStr = (shape == ActivatorShape::Box) ? "box" : "circle";

        json["type"] = typeStr;
        json["shape"] = shapeStr;
        json["size"] = {size.x, size.y};
        json["radius"] = radius;
        json["offset"] = {offset.x, offset.y};
        json["isActive"] = isActive;
        json["canReactivate"] = canReactivate;
        json["cooldownTime"] = cooldownTime;
        json["targetTag"] = targetTag;
        json["actionID"] = actionID;
        json["showDebugZone"] = showDebugZone;
        json["onActivateEvent"] = onActivateEvent;
        json["onDeactivateEvent"] = onDeactivateEvent;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("type")) {
            std::string typeStr = json["type"];
            if (typeStr == "proximity") type = ActivatorType::Proximity;
            else if (typeStr == "manual") type = ActivatorType::Manual;
            else if (typeStr == "automatic") type = ActivatorType::Automatic;
        }

        if (json.contains("shape")) {
            shape = (json["shape"] == "box") ? ActivatorShape::Box : ActivatorShape::Circle;
        }

        if (json.contains("size")) {
            size = Vec2f{json["size"][0], json["size"][1]};
        }
        if (json.contains("radius")) radius = json["radius"];
        if (json.contains("offset")) {
            offset = Vec2f{json["offset"][0], json["offset"][1]};
        }
        if (json.contains("isActive")) isActive = json["isActive"];
        if (json.contains("canReactivate")) canReactivate = json["canReactivate"];
        if (json.contains("cooldownTime")) cooldownTime = json["cooldownTime"];
        if (json.contains("targetTag")) targetTag = json["targetTag"];
        if (json.contains("actionID")) actionID = json["actionID"];
        if (json.contains("showDebugZone")) showDebugZone = json["showDebugZone"];
        if (json.contains("onActivateEvent")) onActivateEvent = json["onActivateEvent"];
        if (json.contains("onDeactivateEvent")) onDeactivateEvent = json["onDeactivateEvent"];
    }
};

// ============================================================================
// TagComponent - Simple tag for entity identification
// ============================================================================

/**
 * @brief Tag component - identifies entity type/role
 *
 * Used to identify entities (e.g., "player", "enemy", "npc")
 * for triggering activators and other game logic.
 */
class TagComponent : public Component {
public:
    std::string tag = "default";

    COMPONENT_TYPE_ID(TagComponent)

    void serialize(nlohmann::json& json) const override {
        json["tag"] = tag;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("tag")) tag = json["tag"];
    }
};

// ============================================================================
// SceneTransitionComponent - Scene transition state
// ============================================================================

/**
 * @brief Scene transition component - manages entity transitions between scenes
 *
 * This component is used to mark entities that need to be transferred from
 * one scene to another. SceneManager monitors this component and handles
 * the actual transfer.
 */
class SceneTransitionComponent : public Component {
public:
    std::string targetScene;     // Scene to transition to
    Vec2f targetPosition;        // Position in target scene
    bool isTransitioning = false; // Currently transitioning?

    COMPONENT_TYPE_ID(SceneTransitionComponent)

    void serialize(nlohmann::json& json) const override {
        json["targetScene"] = targetScene;
        json["targetPosition"] = {targetPosition.x, targetPosition.y};
        json["isTransitioning"] = isTransitioning;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("targetScene")) {
            targetScene = json["targetScene"].get<std::string>();
        }
        if (json.contains("targetPosition")) {
            auto& pos = json["targetPosition"];
            targetPosition = Vec2f{pos[0], pos[1]};
        }
        if (json.contains("isTransitioning")) {
            isTransitioning = json["isTransitioning"];
        }
    }
};

// ============================================================================
// ShaderComponent - Custom shader per entity
// ============================================================================
/**
 * @brief Shader component - applies custom shader to entity sprite
 */
class ShaderComponent : public Component {
public:
    ShaderHandle shader = INVALID_HANDLE;
    bool enabled = true;

    COMPONENT_TYPE_ID(ShaderComponent)

    void serialize(nlohmann::json& json) const override {
        json["enabled"] = enabled;
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("enabled")) enabled = json["enabled"];
    }
};

// ============================================================================
// JourneyComponent - Multi-scene travel management
// ============================================================================

/**
 * @brief Journey component - manages multi-scene travel for NPCs
 *
 * This component tracks an entity's journey through multiple scenes.
 * The entity will PHYSICALLY traverse all intermediate scenes instead
 * of teleporting, ensuring the player can see NPCs passing through.
 *
 * Now includes waypoint-based pathfinding within scenes for realistic movement.
 */
class JourneyComponent : public Component {
public:
    // Multi-scene journey data
    std::vector<std::string> scenePath;    // Full path of scenes to traverse
    int currentSceneIndex = 0;             // Current index in scenePath

    Vec2f currentDestination;              // Final destination in current scene
    bool reachedCurrentDestination = false;

    // Waypoint path within current scene
    std::vector<Vec2f> localWaypointPath;  // Path of waypoints to follow in current scene
    int currentLocalWaypointIndex = 0;     // Current waypoint being approached

    // Personality: preferred path tags for waypoint selection
    std::vector<std::string> preferredPathTags; // Ex: ["main_road"], ["shortcut"], ["scenic"]

    bool isOnJourney = false;              // Journey in progress?
    std::string finalDestinationScene;     // Final destination scene
    Vec2f finalDestinationPos;             // Final destination position

    COMPONENT_TYPE_ID(JourneyComponent)

    void serialize(nlohmann::json& json) const override {
        json["scenePath"] = scenePath;
        json["currentSceneIndex"] = currentSceneIndex;
        json["currentDestination"] = {currentDestination.x, currentDestination.y};
        json["reachedCurrentDestination"] = reachedCurrentDestination;

        // Serialize local waypoint path
        nlohmann::json waypointsArray = nlohmann::json::array();
        for (const auto& wp : localWaypointPath) {
            waypointsArray.push_back({wp.x, wp.y});
        }
        json["localWaypointPath"] = waypointsArray;
        json["currentLocalWaypointIndex"] = currentLocalWaypointIndex;

        json["preferredPathTags"] = preferredPathTags;
        json["isOnJourney"] = isOnJourney;
        json["finalDestinationScene"] = finalDestinationScene;
        json["finalDestinationPos"] = {finalDestinationPos.x, finalDestinationPos.y};
    }

    void deserialize(const nlohmann::json& json) override {
        if (json.contains("scenePath")) {
            scenePath = json["scenePath"].get<std::vector<std::string>>();
        }
        if (json.contains("currentSceneIndex")) currentSceneIndex = json["currentSceneIndex"];
        if (json.contains("currentDestination")) {
            auto& dest = json["currentDestination"];
            currentDestination = Vec2f{dest[0], dest[1]};
        }
        if (json.contains("reachedCurrentDestination")) {
            reachedCurrentDestination = json["reachedCurrentDestination"];
        }

        // Deserialize local waypoint path
        if (json.contains("localWaypointPath") && json["localWaypointPath"].is_array()) {
            localWaypointPath.clear();
            for (const auto& wpData : json["localWaypointPath"]) {
                if (wpData.is_array() && wpData.size() >= 2) {
                    localWaypointPath.push_back(Vec2f{wpData[0], wpData[1]});
                }
            }
        }
        if (json.contains("currentLocalWaypointIndex")) {
            currentLocalWaypointIndex = json["currentLocalWaypointIndex"];
        }

        if (json.contains("preferredPathTags")) {
            preferredPathTags = json["preferredPathTags"].get<std::vector<std::string>>();
        }
        if (json.contains("isOnJourney")) isOnJourney = json["isOnJourney"];
        if (json.contains("finalDestinationScene")) {
            finalDestinationScene = json["finalDestinationScene"];
        }
        if (json.contains("finalDestinationPos")) {
            auto& pos = json["finalDestinationPos"];
            finalDestinationPos = Vec2f{pos[0], pos[1]};
        }
    }
};

} // namespace NovaEngine
