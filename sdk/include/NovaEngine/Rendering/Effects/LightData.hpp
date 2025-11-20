#pragma once

#include "../../Backend/Core/BackendTypes.hpp"

struct LightData {
    NovaEngine::Vec2f position;     // Position normalisée (0-1)
    NovaEngine::Vec3f color;        // Couleur RGB (0-1)
    NovaEngine::f32 radius;         // Rayon normalisé (0-1)
    NovaEngine::f32 intensity;      // Intensité de teinte (0-1)
    bool enabled;                   // Lumière active ou non

    LightData()
        : position(0.5f, 0.5f)
        , color(1.0f, 1.0f, 1.0f)
        , radius(0.3f)
        , intensity(0.5f)
        , enabled(true)
    {}

    LightData(const NovaEngine::Vec2f& pos, const NovaEngine::Vec3f& col,
              NovaEngine::f32 rad, NovaEngine::f32 intens)
        : position(pos)
        , color(col)
        , radius(rad)
        , intensity(intens)
        , enabled(true)
    {}
};
