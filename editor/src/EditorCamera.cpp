#include "../include/EditorCamera.hpp"
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <algorithm>

namespace NovaEditor {

EditorCamera::EditorCamera()
    : m_position(0.0f, 0.0f)
    , m_zoom(1.0f)
    , m_movementSpeed(500.0f)
    , m_zoomSpeed(0.1f)
    , m_minZoom(0.1f)
    , m_maxZoom(5.0f)
    , m_movingUp(false)
    , m_movingDown(false)
    , m_movingLeft(false)
    , m_movingRight(false)
{
    LOG_INFO("EditorCamera initialized at position ({}, {}), zoom {}",
             m_position.x, m_position.y, m_zoom);
}

void EditorCamera::update(float deltaTime) {
    handleKeyboard(deltaTime);
    updateViewport();
}

void EditorCamera::setZoom(NovaEngine::f32 zoom) {
    m_zoom = zoom;
    clampZoom();
    updateViewport();
    LOG_DEBUG("Camera zoom set to {}", m_zoom);
}

void EditorCamera::move(const NovaEngine::Vec2f& delta) {
    m_position += delta;
    updateViewport();
}

void EditorCamera::zoom(NovaEngine::f32 zoomDelta) {
    m_zoom += zoomDelta * m_zoomSpeed;
    clampZoom();
    updateViewport();
}

void EditorCamera::handleKeyboard(float deltaTime) {
    using namespace NovaEngine;

    NovaEngine::Vec2f movement{0.0f, 0.0f};

    // WASD movement
    if (INPUT().isKeyPressed(KeyCode::W) || INPUT().isKeyPressed(KeyCode::Up)) {
        movement.y -= m_movementSpeed * deltaTime / m_zoom;
    }
    if (INPUT().isKeyPressed(KeyCode::S) || INPUT().isKeyPressed(KeyCode::Down)) {
        movement.y += m_movementSpeed * deltaTime / m_zoom;
    }
    if (INPUT().isKeyPressed(KeyCode::A) || INPUT().isKeyPressed(KeyCode::Left)) {
        movement.x -= m_movementSpeed * deltaTime / m_zoom;
    }
    if (INPUT().isKeyPressed(KeyCode::D) || INPUT().isKeyPressed(KeyCode::Right)) {
        movement.x += m_movementSpeed * deltaTime / m_zoom;
    }

    if (movement.x != 0.0f || movement.y != 0.0f) {
        move(movement);
    }
}

void EditorCamera::handleMouseWheel(NovaEngine::f32 delta) {
    zoom(delta);
}

void EditorCamera::handleMouseDrag(const NovaEngine::Vec2f& delta) {
    // Invert delta for natural drag (move camera opposite to mouse)
    move(NovaEngine::Vec2f{-delta.x / m_zoom, -delta.y / m_zoom});
}

NovaEngine::Vec2f EditorCamera::screenToWorld(const NovaEngine::Vec2i& screenPos) const {
    using namespace NovaEngine;

    // Get viewport's conversion
    Vec2f viewportWorld = VIEWPORT().screenToWorld(screenPos);

    // Calculate manual conversion for comparison
    u32 windowWidth = WINDOW().getWidth();
    u32 windowHeight = WINDOW().getHeight();

    Vec2f manualWorld;
    manualWorld.x = (static_cast<f32>(screenPos.x) - static_cast<f32>(windowWidth) / 2.0f) / m_zoom + m_position.x;
    manualWorld.y = (static_cast<f32>(screenPos.y) - static_cast<f32>(windowHeight) / 2.0f) / m_zoom + m_position.y;

    // Always log both for debugging
    LOG_INFO("Screen->World conversion:");
    LOG_INFO("  Screen: ({}, {})", screenPos.x, screenPos.y);
    LOG_INFO("  Viewport result: ({}, {})", viewportWorld.x, viewportWorld.y);
    LOG_INFO("  Manual calc: ({}, {})", manualWorld.x, manualWorld.y);
    LOG_INFO("  Camera: ({}, {}), zoom: {}, window: {}x{}",
             m_position.x, m_position.y, m_zoom, windowWidth, windowHeight);

    // Use manual calculation (viewport might not be set up correctly)
    return manualWorld;
}

NovaEngine::Vec2i EditorCamera::worldToScreen(const NovaEngine::Vec2f& worldPos) const {
    return VIEWPORT().worldToScreen(worldPos);
}

void EditorCamera::setZoomLimits(NovaEngine::f32 minZoom, NovaEngine::f32 maxZoom) {
    m_minZoom = minZoom;
    m_maxZoom = maxZoom;
    clampZoom();
}

void EditorCamera::reset() {
    m_position = NovaEngine::Vec2f{0.0f, 0.0f};
    m_zoom = 1.0f;
    updateViewport();
    LOG_INFO("Camera reset to origin");
}

void EditorCamera::focusOn(const NovaEngine::Vec2f& position) {
    m_position = position;
    updateViewport();
    LOG_DEBUG("Camera focused on ({}, {})", position.x, position.y);
}

void EditorCamera::updateViewport() {
    using namespace NovaEngine;

    ViewportData viewData;
    viewData.center = m_position;

    // Adjust size based on zoom
    u32 windowWidth = WINDOW().getWidth();
    u32 windowHeight = WINDOW().getHeight();
    viewData.size = Vec2f{
        static_cast<f32>(windowWidth) / m_zoom,
        static_cast<f32>(windowHeight) / m_zoom
    };

    viewData.viewport = Rect{0.0f, 0.0f, 1.0f, 1.0f};
    viewData.rotation = 0.0f;

    VIEWPORT().setView(viewData);
}

void EditorCamera::clampZoom() {
    m_zoom = std::clamp(m_zoom, m_minZoom, m_maxZoom);
}

} // namespace NovaEditor
