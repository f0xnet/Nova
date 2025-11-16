#pragma once

#include "Component.hpp"
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

} // namespace NovaEngine
