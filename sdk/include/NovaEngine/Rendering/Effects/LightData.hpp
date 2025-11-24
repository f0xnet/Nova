#pragma once

#include "../../Backend/Core/BackendTypes.hpp"

struct LightData {
    enum class LightType : NovaEngine::i32 {
        Point = 0,        // Omnidirectional light
        Directional = 1,  // Parallel light (sun/moon)
        Spot = 2          // Cone light (spotlight)
    };

    NovaEngine::Vec2f position;     // Position monde (pixels)
    NovaEngine::Vec3f color;        // Couleur RGB (0-1)
    NovaEngine::f32 radius;         // Rayon monde (pixels)
    NovaEngine::f32 intensity;      // Intensité de teinte (0-1)
    NovaEngine::Vec2f direction;    // Direction for Directional/Spot (normalized)
    NovaEngine::f32 angle;          // Cone angle for Spot lights (degrees)
    LightType type;                 // Type de lumière
    bool enabled;                   // Lumière active ou non

    LightData()
        : position(640.0f, 360.0f)  // Centre écran par défaut
        , color(1.0f, 1.0f, 1.0f)
        , radius(200.0f)             // 200 pixels
        , intensity(0.5f)
        , direction(0.0f, 1.0f)      // Down by default
        , angle(45.0f)               // 45 degrees cone
        , type(LightType::Point)
        , enabled(true)
    {}

    LightData(const NovaEngine::Vec2f& pos, const NovaEngine::Vec3f& col,
              NovaEngine::f32 rad, NovaEngine::f32 intens)
        : position(pos)
        , color(col)
        , radius(rad)
        , intensity(intens)
        , direction(0.0f, 1.0f)
        , angle(45.0f)
        , type(LightType::Point)
        , enabled(true)
    {}
};
