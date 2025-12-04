#pragma once

#include "EditorState.hpp"
#include "EditorCamera.hpp"
#include <NovaEngine/Events/Event.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <functional>

namespace NovaEditor {

/**
 * @brief Handler for editor input events
 *
 * Responsibilities:
 * - Process keyboard input
 * - Process mouse input
 * - Translate input to editor commands
 * - Manage input modes (placement, selection, dragging)
 *
 * Benefits:
 * - Single Responsibility: Only handles input
 * - Decouples input from application logic
 * - Easy to modify input mappings
 */
class EditorInputHandler {
public:
    // Input callbacks
    using GridToggleCallback = std::function<void()>;
    using LayersPanelToggleCallback = std::function<void()>;
    using PropertiesPanelToggleCallback = std::function<void()>;
    using LayerChangeCallback = std::function<void(NovaEngine::i32)>;
    using EntityPlacementCallback = std::function<void(const NovaEngine::Vec2f&)>;
    using EntitySelectionCallback = std::function<void(const NovaEngine::Vec2f&)>;
    using EntityDragStartCallback = std::function<void(const NovaEngine::Vec2f&)>;
    using EntityDragUpdateCallback = std::function<void(const NovaEngine::Vec2f&)>;
    using EntityDragStopCallback = std::function<void()>;

    struct InputCallbacks {
        GridToggleCallback onGridToggle;
        LayersPanelToggleCallback onLayersPanelToggle;
        PropertiesPanelToggleCallback onPropertiesPanelToggle;
        LayerChangeCallback onLayerChange;
        EntityPlacementCallback onEntityPlacement;
        EntitySelectionCallback onEntitySelection;
        EntityDragStartCallback onEntityDragStart;
        EntityDragUpdateCallback onEntityDragUpdate;
        EntityDragStopCallback onEntityDragStop;
    };

    EditorInputHandler(
        EditorState* editorState,
        EditorCamera* camera,
        const InputCallbacks& callbacks
    )
        : m_editorState(editorState)
        , m_camera(camera)
        , m_callbacks(callbacks)
        , m_isMousePressed(false)
    {}

    /**
     * @brief Handle keyboard input
     */
    void handleKeyPress(const NovaEngine::InputEvent& input) {
        using namespace NovaEngine;

        // G: Toggle grid
        if (input.key.code == KeyCode::G) {
            if (m_callbacks.onGridToggle) {
                m_callbacks.onGridToggle();
            }
        }

        // L: Toggle layers panel
        if (input.key.code == KeyCode::L) {
            if (m_callbacks.onLayersPanelToggle) {
                m_callbacks.onLayersPanelToggle();
            }
        }

        // P: Show properties panel
        if (input.key.code == KeyCode::P) {
            if (m_callbacks.onPropertiesPanelToggle) {
                m_callbacks.onPropertiesPanelToggle();
            }
        }

        // Number keys: Change layer
        if (input.key.code >= KeyCode::Num0 && input.key.code <= KeyCode::Num9) {
            i32 layer = static_cast<i32>(input.key.code) - static_cast<i32>(KeyCode::Num0);

            // If Shift is held, use negative layer
            if (input.key.shift) {
                layer = -layer;
            }

            if (m_callbacks.onLayerChange) {
                m_callbacks.onLayerChange(layer);
            }
        }
    }

    /**
     * @brief Handle mouse click
     */
    void handleMouseClick(const NovaEngine::InputEvent& input, bool isPlacementMode) {
        using namespace NovaEngine;

        if (input.mouse.button == MouseButton::Left) {
            m_isMousePressed = true;
            m_mouseDownWorldPos = m_camera->screenToWorld(Vec2f{
                static_cast<f32>(input.mouse.x),
                static_cast<f32>(input.mouse.y)
            });

            if (isPlacementMode) {
                // Place entity
                if (m_callbacks.onEntityPlacement) {
                    m_callbacks.onEntityPlacement(m_mouseDownWorldPos);
                }
            } else {
                // Start entity selection or dragging
                if (m_editorState->getSelectedEntity()) {
                    // Start dragging selected entity
                    if (m_callbacks.onEntityDragStart) {
                        m_callbacks.onEntityDragStart(m_mouseDownWorldPos);
                    }
                } else {
                    // Select entity at click position
                    if (m_callbacks.onEntitySelection) {
                        m_callbacks.onEntitySelection(m_mouseDownWorldPos);
                    }
                }
            }
        }
    }

    /**
     * @brief Handle mouse movement
     */
    void handleMouseMove(const NovaEngine::InputEvent& input, bool isDragging) {
        using namespace NovaEngine;

        if (m_isMousePressed && isDragging) {
            Vec2f currentWorldPos = m_camera->screenToWorld(Vec2f{
                static_cast<f32>(input.mouse.x),
                static_cast<f32>(input.mouse.y)
            });

            if (m_callbacks.onEntityDragUpdate) {
                m_callbacks.onEntityDragUpdate(currentWorldPos);
            }
        }
    }

    /**
     * @brief Handle mouse release
     */
    void handleMouseRelease(const NovaEngine::InputEvent& input, bool isDragging) {
        using namespace NovaEngine;

        if (input.mouse.button == MouseButton::Left) {
            m_isMousePressed = false;

            if (isDragging) {
                if (m_callbacks.onEntityDragStop) {
                    m_callbacks.onEntityDragStop();
                }
            }
        }
    }

    /**
     * @brief Update camera based on input (called every frame)
     */
    void updateCamera(float deltaTime) {
        using namespace NovaEngine;

        if (!m_camera) return;

        // Camera movement with arrow keys
        const float moveSpeed = 300.0f * deltaTime;
        Vec2f movement{0.0f, 0.0f};

        if (INPUT().isKeyPressed(KeyCode::Left) || INPUT().isKeyPressed(KeyCode::A)) {
            movement.x -= moveSpeed;
        }
        if (INPUT().isKeyPressed(KeyCode::Right) || INPUT().isKeyPressed(KeyCode::D)) {
            movement.x += moveSpeed;
        }
        if (INPUT().isKeyPressed(KeyCode::Up) || INPUT().isKeyPressed(KeyCode::W)) {
            movement.y -= moveSpeed;
        }
        if (INPUT().isKeyPressed(KeyCode::Down) || INPUT().isKeyPressed(KeyCode::S)) {
            movement.y += moveSpeed;
        }

        if (movement.x != 0.0f || movement.y != 0.0f) {
            m_camera->move(movement);
        }

        // Camera zoom with +/- keys
        if (INPUT().isKeyPressed(KeyCode::Add) || INPUT().isKeyPressed(KeyCode::Equal)) {
            m_camera->zoom(1.0f + deltaTime);
        }
        if (INPUT().isKeyPressed(KeyCode::Subtract) || INPUT().isKeyPressed(KeyCode::Dash)) {
            m_camera->zoom(1.0f - deltaTime);
        }
    }

private:
    EditorState* m_editorState;
    EditorCamera* m_camera;
    InputCallbacks m_callbacks;

    bool m_isMousePressed;
    NovaEngine::Vec2f m_mouseDownWorldPos;
};

} // namespace NovaEditor
