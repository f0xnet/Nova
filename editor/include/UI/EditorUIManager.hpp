#pragma once

#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/UI/UILoader.hpp>
#include <NovaEngine/Core/Types.hpp>
#include "Core/EditorState.hpp"
#include "Core/EditorConfig.hpp"
#include <functional>
#include <string>

namespace NovaEditor {

/**
 * @brief Manages all editor UI panels and handles UI actions
 *
 * Loads JSON-based UI layouts and routes button actions to the application
 */
class EditorUIManager {
public:
    using ActionCallback = std::function<void(const std::string& action, const std::string& value)>;

    EditorUIManager(NovaEngine::UIManager& uiManager, EditorState* state, EditorConfig* config);
    ~EditorUIManager() = default;

    /**
     * @brief Initialize UI by loading all JSON layouts
     */
    bool initialize();

    /**
     * @brief Update UI components (refresh inspector, update button states, etc.)
     */
    void update(float deltaTime);

    /**
     * @brief Set callback for handling UI actions
     */
    void setActionCallback(ActionCallback callback);

    /**
     * @brief Refresh the inspector panel with current selection data
     */
    void refreshInspector();

    /**
     * @brief Refresh the scene hierarchy panel with current scene entities
     */
    void refreshSceneHierarchy();

    /**
     * @brief Update entity palette with available entities from definitions
     */
    void updateEntityPalette(const std::string& category);

    /**
     * @brief Update toolbar button states (undo/redo enabled, grid toggle text)
     */
    void updateToolbarState();

private:
    NovaEngine::UIManager& m_uiManager;
    NovaEngine::UILoader m_uiLoader;
    EditorState* m_editorState;
    EditorConfig* m_editorConfig;

    ActionCallback m_actionCallback;

    /**
     * @brief Internal handler that receives all UI actions
     */
    void handleAction(const std::string& action, const std::string& value, const NovaEngine::ID& componentID);

    /**
     * @brief Load all JSON UI layout files
     */
    bool loadAllLayouts();

    /**
     * @brief Load a single UI layout from file
     */
    bool loadLayout(const std::string& filename);
};

} // namespace NovaEditor
