#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>

namespace NovaEditor {

/**
 * @brief Caméra libre pour l'éditeur
 *
 * Permet de naviguer librement dans la scène avec :
 * - WASD ou flèches pour se déplacer
 * - Molette ou +/- pour zoomer
 * - Click molette + drag pour pan
 */
class EditorCamera {
public:
    EditorCamera();
    ~EditorCamera() = default;

    // Update
    void update(float deltaTime);

    // Position et vue
    const NovaEngine::Vec2f& getPosition() const { return m_position; }
    void setPosition(const NovaEngine::Vec2f& position) { m_position = position; }

    NovaEngine::f32 getZoom() const { return m_zoom; }
    void setZoom(NovaEngine::f32 zoom);

    // Mouvement
    void move(const NovaEngine::Vec2f& delta);
    void zoom(NovaEngine::f32 zoomDelta);

    // Input
    void handleKeyboard(float deltaTime);
    void handleMouseWheel(NovaEngine::f32 delta);
    void handleMouseDrag(const NovaEngine::Vec2f& delta);

    // Conversions
    NovaEngine::Vec2f screenToWorld(const NovaEngine::Vec2i& screenPos) const;
    NovaEngine::Vec2i worldToScreen(const NovaEngine::Vec2f& worldPos) const;

    // Limites
    void setMovementSpeed(NovaEngine::f32 speed) { m_movementSpeed = speed; }
    void setZoomSpeed(NovaEngine::f32 speed) { m_zoomSpeed = speed; }
    void setZoomLimits(NovaEngine::f32 minZoom, NovaEngine::f32 maxZoom);

    // Reset
    void reset();
    void focusOn(const NovaEngine::Vec2f& position);

private:
    void updateViewport();
    void clampZoom();

private:
    NovaEngine::Vec2f m_position;
    NovaEngine::f32 m_zoom;

    NovaEngine::f32 m_movementSpeed;
    NovaEngine::f32 m_zoomSpeed;

    NovaEngine::f32 m_minZoom;
    NovaEngine::f32 m_maxZoom;

    // État input
    bool m_movingUp;
    bool m_movingDown;
    bool m_movingLeft;
    bool m_movingRight;
};

} // namespace NovaEditor
