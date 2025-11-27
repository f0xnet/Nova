#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <NovaEngine/ECS/Entity.hpp>

namespace NovaEditor {

/**
 * @brief Type de gizmo à afficher
 */
enum class GizmoType {
    None,
    Move,       // Flèches de déplacement
    Rotate,     // Arc de rotation
    Scale,      // Carrés de redimensionnement
    Bounds      // Rectangle de sélection
};

/**
 * @brief Système de gizmos visuels pour manipulation d'entités
 *
 * Affiche des contrôles visuels (flèches, poignées) pour manipuler
 * les entités sélectionnées dans l'éditeur.
 */
class Gizmos {
public:
    Gizmos();
    ~Gizmos() = default;

    // Render
    void renderForEntity(NovaEngine::Entity* entity, GizmoType type);
    void renderGrid(NovaEngine::f32 gridSize, const NovaEngine::Color& color);
    void renderBounds(const NovaEngine::Rect& bounds, const NovaEngine::Color& color);
    void renderLight(const NovaEngine::Vec2f& position, NovaEngine::f32 radius, const NovaEngine::Color& color);
    void renderActivator(const NovaEngine::Vec2f& position, const NovaEngine::Vec2f& size, const NovaEngine::Color& color);
    void renderWaypoint(const NovaEngine::Vec2f& position, const NovaEngine::Color& color);

    // Gizmo de déplacement (flèches X/Y)
    void renderMoveGizmo(const NovaEngine::Vec2f& position);

    // Gizmo de rotation (arc circulaire)
    void renderRotateGizmo(const NovaEngine::Vec2f& position);

    // Gizmo de redimensionnement (poignées aux coins)
    void renderScaleGizmo(const NovaEngine::Vec2f& position, const NovaEngine::Vec2f& size);

    // Hit testing (détection si on clique sur un gizmo)
    enum class GizmoAxis { None, X, Y, XY };
    GizmoAxis hitTest(const NovaEngine::Vec2f& worldPos, const NovaEngine::Vec2f& gizmoPos, GizmoType type);

    // Configuration
    void setGizmoSize(NovaEngine::f32 size) { m_gizmoSize = size; }
    void setGizmoColor(const NovaEngine::Color& color) { m_gizmoColor = color; }

private:
    void drawArrow(const NovaEngine::Vec2f& start, const NovaEngine::Vec2f& end, const NovaEngine::Color& color);
    void drawCircle(const NovaEngine::Vec2f& center, NovaEngine::f32 radius, const NovaEngine::Color& color, bool filled = false);
    void drawLine(const NovaEngine::Vec2f& start, const NovaEngine::Vec2f& end, const NovaEngine::Color& color, NovaEngine::f32 thickness = 2.0f);

private:
    NovaEngine::f32 m_gizmoSize;
    NovaEngine::Color m_gizmoColor;
    NovaEngine::Color m_gizmoXColor;  // Rouge pour axe X
    NovaEngine::Color m_gizmoYColor;  // Vert pour axe Y
    NovaEngine::Color m_gizmoZColor;  // Bleu pour axe Z (futur)
};

} // namespace NovaEditor
