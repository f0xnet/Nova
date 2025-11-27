#include "../include/Gizmos.hpp"
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <cmath>

namespace NovaEditor {

Gizmos::Gizmos()
    : m_gizmoSize(50.0f)
    , m_gizmoColor{255, 255, 255, 255}
    , m_gizmoXColor{255, 0, 0, 255}    // Rouge pour X
    , m_gizmoYColor{0, 255, 0, 255}    // Vert pour Y
    , m_gizmoZColor{0, 0, 255, 255}    // Bleu pour Z
{
}

void Gizmos::renderForEntity(NovaEngine::Entity* entity, GizmoType type) {
    if (!entity) return;

    auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
    if (!transform) return;

    switch (type) {
        case GizmoType::Move:
            renderMoveGizmo(transform->position);
            break;
        case GizmoType::Rotate:
            renderRotateGizmo(transform->position);
            break;
        case GizmoType::Scale: {
            auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
            if (sprite) {
                renderScaleGizmo(transform->position, sprite->size);
            }
            break;
        }
        case GizmoType::Bounds: {
            auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
            if (sprite) {
                NovaEngine::Rect bounds{
                    transform->position.x - sprite->size.x / 2.0f,
                    transform->position.y - sprite->size.y / 2.0f,
                    sprite->size.x,
                    sprite->size.y
                };
                renderBounds(bounds, NovaEngine::Color{255, 255, 0, 200});
            }
            break;
        }
        default:
            break;
    }
}

void Gizmos::renderGrid(NovaEngine::f32 gridSize, const NovaEngine::Color& color) {
    using namespace NovaEngine;

    // Get viewport bounds
    Vec2f cameraCenter = VIEWPORT().getView().center;
    Vec2f viewportSize = VIEWPORT().getView().size;

    f32 left = cameraCenter.x - viewportSize.x / 2.0f;
    f32 right = cameraCenter.x + viewportSize.x / 2.0f;
    f32 top = cameraCenter.y - viewportSize.y / 2.0f;
    f32 bottom = cameraCenter.y + viewportSize.y / 2.0f;

    // Align to grid
    f32 startX = std::floor(left / gridSize) * gridSize;
    f32 startY = std::floor(top / gridSize) * gridSize;

    // Vertical lines
    for (f32 x = startX; x <= right; x += gridSize) {
        drawLine(Vec2f{x, top}, Vec2f{x, bottom}, color, 1.0f);
    }

    // Horizontal lines
    for (f32 y = startY; y <= bottom; y += gridSize) {
        drawLine(Vec2f{left, y}, Vec2f{right, y}, color, 1.0f);
    }

    // Axes (X=red, Y=green)
    if (left <= 0 && right >= 0) {
        drawLine(Vec2f{0, top}, Vec2f{0, bottom}, Color{255, 0, 0, 128}, 2.0f);
    }
    if (top <= 0 && bottom >= 0) {
        drawLine(Vec2f{left, 0}, Vec2f{right, 0}, Color{0, 255, 0, 128}, 2.0f);
    }
}

void Gizmos::renderBounds(const NovaEngine::Rect& bounds, const NovaEngine::Color& color) {
    using namespace NovaEngine;

    RectData rectData;
    rectData.position = Vec2f{bounds.left, bounds.top};
    rectData.size = Vec2f{bounds.width, bounds.height};
    rectData.fillColor = Color{0, 0, 0, 0};  // Transparent
    rectData.outlineColor = color;
    rectData.outlineThickness = 2.0f;

    GRAPHICS().drawRect(rectData);
}

void Gizmos::renderLight(const NovaEngine::Vec2f& position, NovaEngine::f32 radius, const NovaEngine::Color& color) {
    drawCircle(position, radius, color, false);
}

void Gizmos::renderActivator(const NovaEngine::Vec2f& position, const NovaEngine::Vec2f& size, const NovaEngine::Color& color) {
    using namespace NovaEngine;

    Rect bounds{
        position.x - size.x / 2.0f,
        position.y - size.y / 2.0f,
        size.x,
        size.y
    };
    renderBounds(bounds, color);
}

void Gizmos::renderWaypoint(const NovaEngine::Vec2f& position, const NovaEngine::Color& color) {
    drawCircle(position, 10.0f, color, true);
}

void Gizmos::renderMoveGizmo(const NovaEngine::Vec2f& position) {
    using namespace NovaEngine;

    // Axe X (rouge, horizontal)
    Vec2f xStart = position;
    Vec2f xEnd = Vec2f{position.x + m_gizmoSize, position.y};
    drawArrow(xStart, xEnd, m_gizmoXColor);

    // Axe Y (vert, vertical)
    Vec2f yStart = position;
    Vec2f yEnd = Vec2f{position.x, position.y + m_gizmoSize};
    drawArrow(yStart, yEnd, m_gizmoYColor);

    // Centre (blanc)
    drawCircle(position, 5.0f, m_gizmoColor, true);
}

void Gizmos::renderRotateGizmo(const NovaEngine::Vec2f& position) {
    // Arc circulaire pour rotation (simplifié)
    drawCircle(position, m_gizmoSize, NovaEngine::Color{100, 150, 255, 200}, false);
}

void Gizmos::renderScaleGizmo(const NovaEngine::Vec2f& position, const NovaEngine::Vec2f& size) {
    using namespace NovaEngine;

    // 4 poignées aux coins pour redimensionnement
    f32 halfW = size.x / 2.0f;
    f32 halfH = size.y / 2.0f;
    f32 handleSize = 8.0f;

    Vec2f corners[4] = {
        {position.x - halfW, position.y - halfH},  // Top-left
        {position.x + halfW, position.y - halfH},  // Top-right
        {position.x - halfW, position.y + halfH},  // Bottom-left
        {position.x + halfW, position.y + halfH}   // Bottom-right
    };

    for (const auto& corner : corners) {
        drawCircle(corner, handleSize, m_gizmoColor, true);
    }
}

Gizmos::GizmoAxis Gizmos::hitTest(const NovaEngine::Vec2f& worldPos,
                                  const NovaEngine::Vec2f& gizmoPos,
                                  GizmoType type) {
    using namespace NovaEngine;

    if (type == GizmoType::Move) {
        // Test axe X
        Vec2f xEnd{gizmoPos.x + m_gizmoSize, gizmoPos.y};
        f32 distToX = std::abs(worldPos.y - gizmoPos.y);
        if (worldPos.x >= gizmoPos.x && worldPos.x <= xEnd.x && distToX < 10.0f) {
            return GizmoAxis::X;
        }

        // Test axe Y
        Vec2f yEnd{gizmoPos.x, gizmoPos.y + m_gizmoSize};
        f32 distToY = std::abs(worldPos.x - gizmoPos.x);
        if (worldPos.y >= gizmoPos.y && worldPos.y <= yEnd.y && distToY < 10.0f) {
            return GizmoAxis::Y;
        }

        // Test centre (mouvement XY libre)
        f32 distToCenter = std::sqrt(
            (worldPos.x - gizmoPos.x) * (worldPos.x - gizmoPos.x) +
            (worldPos.y - gizmoPos.y) * (worldPos.y - gizmoPos.y)
        );
        if (distToCenter < 10.0f) {
            return GizmoAxis::XY;
        }
    }

    return GizmoAxis::None;
}

void Gizmos::drawArrow(const NovaEngine::Vec2f& start, const NovaEngine::Vec2f& end, const NovaEngine::Color& color) {
    using namespace NovaEngine;

    // Ligne principale
    drawLine(start, end, color, 3.0f);

    // Pointe de flèche (simplifié - juste un triangle)
    Vec2f dir = {end.x - start.x, end.y - start.y};
    f32 length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length > 0) {
        dir.x /= length;
        dir.y /= length;
    }

    f32 arrowSize = 10.0f;
    Vec2f perp = {-dir.y, dir.x};

    Vec2f p1 = {end.x - dir.x * arrowSize + perp.x * arrowSize / 2.0f,
                end.y - dir.y * arrowSize + perp.y * arrowSize / 2.0f};
    Vec2f p2 = {end.x - dir.x * arrowSize - perp.x * arrowSize / 2.0f,
                end.y - dir.y * arrowSize - perp.y * arrowSize / 2.0f};

    drawLine(end, p1, color, 3.0f);
    drawLine(end, p2, color, 3.0f);
}

void Gizmos::drawCircle(const NovaEngine::Vec2f& center, NovaEngine::f32 radius,
                       const NovaEngine::Color& color, bool filled) {
    using namespace NovaEngine;

    RectData rectData;
    rectData.position = Vec2f{center.x - radius, center.y - radius};
    rectData.size = Vec2f{radius * 2.0f, radius * 2.0f};

    if (filled) {
        rectData.fillColor = color;
        rectData.outlineThickness = 0.0f;
    } else {
        rectData.fillColor = Color{0, 0, 0, 0};
        rectData.outlineColor = color;
        rectData.outlineThickness = 2.0f;
    }

    GRAPHICS().drawRect(rectData);
}

void Gizmos::drawLine(const NovaEngine::Vec2f& start, const NovaEngine::Vec2f& end,
                     const NovaEngine::Color& color, NovaEngine::f32 thickness) {
    using namespace NovaEngine;

    Vec2f dir = {end.x - start.x, end.y - start.y};
    f32 length = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (length < 0.01f) return;

    f32 angle = std::atan2(dir.y, dir.x) * 180.0f / 3.14159265f;

    RectData lineData;
    lineData.position = start;
    lineData.size = Vec2f{length, thickness};
    lineData.fillColor = color;
    lineData.rotation = angle;
    lineData.origin = Vec2f{0, thickness / 2.0f};

    GRAPHICS().drawRect(lineData);
}

} // namespace NovaEditor
