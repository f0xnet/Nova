#pragma once

#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <functional>
#include <string>

namespace NovaEditor {

class EditorState;
class EntityPalette;

/**
 * @brief Interface utilisateur de l'éditeur
 *
 * Gère tous les panneaux UI :
 * - Barre d'outils (toolbar)
 * - Palette d'entités
 * - Inspecteur de propriétés
 * - Hiérarchie de scène
 * - Panneau de paramètres
 */
class EditorUI {
public:
    EditorUI(NovaEngine::UIManager& uiManager, EditorState* editorState);
    ~EditorUI() = default;

    // Initialisation
    void initialize();

    // Update
    void update(float deltaTime);

    // Render
    void render();

    // Actions
    void setActionCallback(std::function<void(const std::string&, const std::string&)> callback);

    // Rafraîchir UI selon état
    void refresh();
    void refreshEntityInspector(NovaEngine::Entity* entity);
    void refreshEntityList(const std::vector<NovaEngine::Entity*>& entities);

    // Panneaux
    void showToolbar(bool show);
    void showEntityPalette(bool show);
    void showInspector(bool show);
    void showSceneHierarchy(bool show);
    void showSettings(bool show);

    // Messages
    void showMessage(const std::string& message, float duration = 3.0f);
    void showError(const std::string& error);
    void showSuccess(const std::string& message);

private:
    void createToolbar();
    void createEntityPalettePanel();
    void createInspectorPanel();
    void createSceneHierarchyPanel();
    void createSettingsPanel();
    void createMenuBar();

    void updateInspectorForEntity(NovaEngine::Entity* entity);
    void createTransformInspector(NovaEngine::Entity* entity);
    void createSpriteInspector(NovaEngine::Entity* entity);
    void createLightInspector(NovaEngine::Entity* entity);
    void createColliderInspector(NovaEngine::Entity* entity);
    void createActivatorInspector(NovaEngine::Entity* entity);

private:
    NovaEngine::UIManager& m_uiManager;
    EditorState* m_editorState;

    std::function<void(const std::string&, const std::string&)> m_actionCallback;

    // Font
    NovaEngine::FontHandle m_defaultFont;

    // Panneaux
    bool m_showToolbar;
    bool m_showEntityPalette;
    bool m_showInspector;
    bool m_showSceneHierarchy;
    bool m_showSettings;

    // Message temporaire
    std::string m_currentMessage;
    float m_messageTimer;
};

} // namespace NovaEditor
