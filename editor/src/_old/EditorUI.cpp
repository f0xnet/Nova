#include "../include/EditorUI.hpp"
#include "../include/EditorState.hpp"
#include "../include/EntityPalette.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/UI/Components/Panel.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <NovaEngine/UI/Components/Text.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>

namespace NovaEditor {

EditorUI::EditorUI(NovaEngine::UIManager& uiManager, EditorState* editorState)
    : m_uiManager(uiManager)
    , m_editorState(editorState)
    , m_showToolbar(true)
    , m_showEntityPalette(true)
    , m_showInspector(true)
    , m_showSceneHierarchy(true)
    , m_showSettings(false)
    , m_messageTimer(0.0f)
{
}

void EditorUI::initialize() {
    LOG_INFO("Initializing EditorUI...");

    // Load default font (system font)
    m_defaultFont = FONTS().loadFont("C:/Windows/Fonts/arial.ttf");
    if (m_defaultFont == NovaEngine::INVALID_HANDLE) {
        LOG_WARN("Failed to load default font, trying alternative...");
        m_defaultFont = FONTS().loadFont("C:/Windows/Fonts/segoeui.ttf");
    }
    if (m_defaultFont == NovaEngine::INVALID_HANDLE) {
        LOG_ERROR("CRITICAL: No font could be loaded! UI text will be invisible!");
    } else {
        LOG_INFO("Default font loaded successfully");
    }

    createMenuBar();
    createToolbar();
    createEntityPalettePanel();
    createInspectorPanel();
    createSceneHierarchyPanel();
    createSettingsPanel();

    LOG_INFO("EditorUI initialized");
}

void EditorUI::update(float deltaTime) {
    // Update UIManager (updates all UI components)
    m_uiManager.update(deltaTime);

    // Update message timer
    if (m_messageTimer > 0.0f) {
        m_messageTimer -= deltaTime;
        if (m_messageTimer <= 0.0f) {
            m_currentMessage.clear();
        }
    }
}

void EditorUI::render() {
    // Render all UI components via UIManager
    m_uiManager.render();

    // TODO: Render temporary messages if needed
    // For now, messages are logged to console
}

void EditorUI::setActionCallback(std::function<void(const std::string&, const std::string&)> callback) {
    m_actionCallback = callback;
}

void EditorUI::refresh() {
    // Rafraîchir tous les panels
    if (m_editorState && m_editorState->hasSelection()) {
        refreshEntityInspector(m_editorState->getSelectedEntity());
    }
}

void EditorUI::refreshEntityInspector(NovaEngine::Entity* entity) {
    if (!entity) return;
    updateInspectorForEntity(entity);
}

void EditorUI::refreshEntityList(const std::vector<NovaEngine::Entity*>& entities) {
    // TODO: Mettre à jour la hiérarchie de scène
    LOG_DEBUG("Refreshing entity list: {} entities", entities.size());
}

void EditorUI::showToolbar(bool show) {
    m_showToolbar = show;
}

void EditorUI::showEntityPalette(bool show) {
    m_showEntityPalette = show;
}

void EditorUI::showInspector(bool show) {
    m_showInspector = show;
}

void EditorUI::showSceneHierarchy(bool show) {
    m_showSceneHierarchy = show;
}

void EditorUI::showSettings(bool show) {
    m_showSettings = show;
}

void EditorUI::showMessage(const std::string& message, float duration) {
    m_currentMessage = message;
    m_messageTimer = duration;
    LOG_INFO("[UI] {}", message);
}

void EditorUI::showError(const std::string& error) {
    showMessage("[ERROR] " + error, 5.0f);
    LOG_ERROR("{}", error);
}

void EditorUI::showSuccess(const std::string& message) {
    showMessage("[SUCCESS] " + message, 3.0f);
}

// === Private Methods ===

void EditorUI::createMenuBar() {
    // TODO: Créer menu bar avec UIManager
    // Pour l'instant, on utilise juste des raccourcis clavier
    LOG_DEBUG("Menu bar created (keyboard shortcuts only)");
}

void EditorUI::createToolbar() {
    if (!m_showToolbar) return;

    // Toolbar background panel
    auto toolbarPanel = std::make_shared<NovaEngine::Panel>();
    toolbarPanel->setID("editor_toolbar_panel");
    toolbarPanel->setUIID("editor");
    toolbarPanel->setGroupID("toolbar");
    toolbarPanel->setLayer(10);
    toolbarPanel->setPosition({0, 0});
    toolbarPanel->setSize({1920, 50});
    toolbarPanel->setColor({40, 40, 45, 255});
    m_uiManager.addComponent(toolbarPanel);

    // Create toolbar buttons
    const float btnWidth = 80.0f;
    const float btnHeight = 35.0f;
    const float btnSpacing = 10.0f;
    float xPos = 10.0f;

    // Save button
    auto saveBtn = std::make_shared<NovaEngine::Button>();
    saveBtn->setID("btn_save");
    saveBtn->setUIID("editor");
    saveBtn->setGroupID("toolbar");
    saveBtn->setLayer(11);
    saveBtn->setPosition({xPos, 7.5f});
    saveBtn->setSize({btnWidth, btnHeight});
    saveBtn->setText("Save");
    saveBtn->setFont(m_defaultFont);
    saveBtn->setFontSize(16);
    saveBtn->setAction("save_scene");
    saveBtn->setHaveText(true);
    saveBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(saveBtn);
    xPos += btnWidth + btnSpacing;

    // Load button
    auto loadBtn = std::make_shared<NovaEngine::Button>();
    loadBtn->setID("btn_load");
    loadBtn->setUIID("editor");
    loadBtn->setGroupID("toolbar");
    loadBtn->setLayer(11);
    loadBtn->setPosition({xPos, 7.5f});
    loadBtn->setSize({btnWidth, btnHeight});
    loadBtn->setText("Load");
    loadBtn->setFont(m_defaultFont);
    loadBtn->setFontSize(16);
    loadBtn->setAction("load_scene");
    loadBtn->setHaveText(true);
    loadBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(loadBtn);
    xPos += btnWidth + btnSpacing;

    // Play button
    auto playBtn = std::make_shared<NovaEngine::Button>();
    playBtn->setID("btn_play");
    playBtn->setUIID("editor");
    playBtn->setGroupID("toolbar");
    playBtn->setLayer(11);
    playBtn->setPosition({xPos, 7.5f});
    playBtn->setSize({btnWidth, btnHeight});
    playBtn->setText("Play");
    playBtn->setFont(m_defaultFont);
    playBtn->setFontSize(16);
    playBtn->setAction("play_mode");
    playBtn->setHaveText(true);
    playBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(playBtn);
    xPos += btnWidth + btnSpacing;

    // Grid toggle button
    auto gridBtn = std::make_shared<NovaEngine::Button>();
    gridBtn->setID("btn_grid");
    gridBtn->setUIID("editor");
    gridBtn->setGroupID("toolbar");
    gridBtn->setLayer(11);
    gridBtn->setPosition({xPos, 7.5f});
    gridBtn->setSize({btnWidth, btnHeight});
    gridBtn->setText("Grid");
    gridBtn->setFont(m_defaultFont);
    gridBtn->setFontSize(16);
    gridBtn->setAction("toggle_grid");
    gridBtn->setHaveText(true);
    gridBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(gridBtn);

    LOG_INFO("Toolbar created with {} buttons", 4);
}

void EditorUI::createEntityPalettePanel() {
    if (!m_showEntityPalette) return;

    // Entity palette panel (left side)
    auto palettePanel = std::make_shared<NovaEngine::Panel>();
    palettePanel->setID("entity_palette_panel");
    palettePanel->setUIID("editor");
    palettePanel->setGroupID("palette");
    palettePanel->setLayer(10);
    palettePanel->setPosition({0, 50});
    palettePanel->setSize({250, 500});
    palettePanel->setColor({30, 30, 35, 255});
    m_uiManager.addComponent(palettePanel);

    // Title
    auto titleText = std::make_shared<NovaEngine::Text>();
    titleText->setID("palette_title");
    titleText->setUIID("editor");
    titleText->setGroupID("palette");
    titleText->setLayer(11);
    titleText->setPosition({10, 60});
    titleText->setString("Entity Palette");
    titleText->setFont(m_defaultFont);
    titleText->setCharacterSize(18);
    titleText->setTextColor({255, 255, 255, 255});
    m_uiManager.addComponent(titleText);

    // Entity buttons
    const float btnWidth = 230.0f;
    const float btnHeight = 30.0f;
    const float btnSpacing = 5.0f;
    float yPos = 90.0f;

    // Sprite button
    auto spriteBtn = std::make_shared<NovaEngine::Button>();
    spriteBtn->setID("btn_add_sprite");
    spriteBtn->setUIID("editor");
    spriteBtn->setGroupID("palette");
    spriteBtn->setLayer(11);
    spriteBtn->setPosition({10, yPos});
    spriteBtn->setSize({btnWidth, btnHeight});
    spriteBtn->setText("+ Sprite");
    spriteBtn->setFont(m_defaultFont);
    spriteBtn->setFontSize(14);
    spriteBtn->setAction("add_entity");
    spriteBtn->setValue("sprite");
    spriteBtn->setHaveText(true);
    spriteBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(spriteBtn);
    yPos += btnHeight + btnSpacing;

    // Light button
    auto lightBtn = std::make_shared<NovaEngine::Button>();
    lightBtn->setID("btn_add_light");
    lightBtn->setUIID("editor");
    lightBtn->setGroupID("palette");
    lightBtn->setLayer(11);
    lightBtn->setPosition({10, yPos});
    lightBtn->setSize({btnWidth, btnHeight});
    lightBtn->setText("+ Light");
    lightBtn->setFont(m_defaultFont);
    lightBtn->setFontSize(14);
    lightBtn->setAction("add_entity");
    lightBtn->setValue("light");
    lightBtn->setHaveText(true);
    lightBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(lightBtn);
    yPos += btnHeight + btnSpacing;

    // Collider button
    auto colliderBtn = std::make_shared<NovaEngine::Button>();
    colliderBtn->setID("btn_add_collider");
    colliderBtn->setUIID("editor");
    colliderBtn->setGroupID("palette");
    colliderBtn->setLayer(11);
    colliderBtn->setPosition({10, yPos});
    colliderBtn->setSize({btnWidth, btnHeight});
    colliderBtn->setText("+ Collider");
    colliderBtn->setFont(m_defaultFont);
    colliderBtn->setFontSize(14);
    colliderBtn->setAction("add_entity");
    colliderBtn->setValue("collider");
    colliderBtn->setHaveText(true);
    colliderBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(colliderBtn);
    yPos += btnHeight + btnSpacing;

    // Activator button
    auto activatorBtn = std::make_shared<NovaEngine::Button>();
    activatorBtn->setID("btn_add_activator");
    activatorBtn->setUIID("editor");
    activatorBtn->setGroupID("palette");
    activatorBtn->setLayer(11);
    activatorBtn->setPosition({10, yPos});
    activatorBtn->setSize({btnWidth, btnHeight});
    activatorBtn->setText("+ Activator");
    activatorBtn->setFont(m_defaultFont);
    activatorBtn->setFontSize(14);
    activatorBtn->setAction("add_entity");
    activatorBtn->setValue("activator");
    activatorBtn->setHaveText(true);
    activatorBtn->setOnClickWithAction([this](const std::string& action, const std::string& value, const NovaEngine::ID& id) {
        m_uiManager.handleAction(action, value, id);
    });
    m_uiManager.addComponent(activatorBtn);

    LOG_INFO("Entity palette panel created with {} entity types", 4);
}

void EditorUI::createInspectorPanel() {
    if (!m_showInspector) return;

    // Inspector panel (right side)
    auto inspectorPanel = std::make_shared<NovaEngine::Panel>();
    inspectorPanel->setID("inspector_panel");
    inspectorPanel->setUIID("editor");
    inspectorPanel->setGroupID("inspector");
    inspectorPanel->setLayer(10);
    inspectorPanel->setPosition({1920 - 300, 50});
    inspectorPanel->setSize({300, 600});
    inspectorPanel->setColor({30, 30, 35, 255});
    m_uiManager.addComponent(inspectorPanel);

    // Title
    auto titleText = std::make_shared<NovaEngine::Text>();
    titleText->setID("inspector_title");
    titleText->setUIID("editor");
    titleText->setGroupID("inspector");
    titleText->setLayer(11);
    titleText->setPosition({1920 - 290, 60});
    titleText->setString("Inspector");
    titleText->setFont(m_defaultFont);
    titleText->setCharacterSize(18);
    titleText->setTextColor({255, 255, 255, 255});
    m_uiManager.addComponent(titleText);

    // Info text (when no selection)
    auto infoText = std::make_shared<NovaEngine::Text>();
    infoText->setID("inspector_info");
    infoText->setUIID("editor");
    infoText->setGroupID("inspector");
    infoText->setLayer(11);
    infoText->setPosition({1920 - 290, 100});
    infoText->setString("Select an entity\nto view properties");
    infoText->setFont(m_defaultFont);
    infoText->setCharacterSize(14);
    infoText->setTextColor({180, 180, 180, 255});
    m_uiManager.addComponent(infoText);

    LOG_INFO("Inspector panel created");
}

void EditorUI::createSceneHierarchyPanel() {
    if (!m_showSceneHierarchy) return;

    // Scene hierarchy panel (left side, below palette)
    auto hierarchyPanel = std::make_shared<NovaEngine::Panel>();
    hierarchyPanel->setID("hierarchy_panel");
    hierarchyPanel->setUIID("editor");
    hierarchyPanel->setGroupID("hierarchy");
    hierarchyPanel->setLayer(10);
    hierarchyPanel->setPosition({0, 550});
    hierarchyPanel->setSize({250, 530});
    hierarchyPanel->setColor({30, 30, 35, 255});
    m_uiManager.addComponent(hierarchyPanel);

    // Title
    auto titleText = std::make_shared<NovaEngine::Text>();
    titleText->setID("hierarchy_title");
    titleText->setUIID("editor");
    titleText->setGroupID("hierarchy");
    titleText->setLayer(11);
    titleText->setPosition({10, 560});
    titleText->setString("Scene Hierarchy");
    titleText->setFont(m_defaultFont);
    titleText->setCharacterSize(18);
    titleText->setTextColor({255, 255, 255, 255});
    m_uiManager.addComponent(titleText);

    // Info text
    auto infoText = std::make_shared<NovaEngine::Text>();
    infoText->setID("hierarchy_info");
    infoText->setUIID("editor");
    infoText->setGroupID("hierarchy");
    infoText->setLayer(11);
    infoText->setPosition({10, 595});
    infoText->setString("Entities will appear here");
    infoText->setFont(m_defaultFont);
    infoText->setCharacterSize(12);
    infoText->setTextColor({150, 150, 150, 255});
    m_uiManager.addComponent(infoText);

    LOG_INFO("Scene hierarchy panel created");
}

void EditorUI::createSettingsPanel() {
    // Settings panel is created on-demand when user opens it
    // For now, settings are accessed via keyboard shortcuts
    LOG_DEBUG("Settings panel system ready (opened via shortcuts)");
}

void EditorUI::updateInspectorForEntity(NovaEngine::Entity* entity) {
    if (!entity) {
        // Clear inspector - show default info text
        auto infoComp = m_uiManager.getComponent("inspector_info");
        if (infoComp) {
            auto info = std::dynamic_pointer_cast<NovaEngine::Text>(infoComp);
            if (info) {
                info->setString("Select an entity\nto view properties");
                info->setVisible(true);
            }
        }
        LOG_DEBUG("Inspector cleared");
        return;
    }

    // Hide default info text
    auto infoComp = m_uiManager.getComponent("inspector_info");
    if (infoComp) {
        infoComp->setVisible(false);
    }

    LOG_INFO("Inspecting Entity ID: {}", entity->getID());

    // Log component data to console
    if (entity->hasComponent<NovaEngine::TransformComponent>()) {
        createTransformInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::SpriteComponent>()) {
        createSpriteInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::LightComponent>()) {
        createLightInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::ColliderComponent>()) {
        createColliderInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::ActivatorComponent>()) {
        createActivatorInspector(entity);
    }

    // TODO: Create dynamic text fields for component editing
    // For now, properties are logged to console
}

void EditorUI::createTransformInspector(NovaEngine::Entity* entity) {
    auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
    if (!transform) return;

    LOG_DEBUG("Transform: pos=({}, {}), rot={}, scale=({}, {})",
        transform->position.x, transform->position.y,
        transform->rotation,
        transform->scale.x, transform->scale.y);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createSpriteInspector(NovaEngine::Entity* entity) {
    auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
    if (!sprite) return;

    LOG_DEBUG("Sprite: texture={}, zOrder={}",
        sprite->textureHandle, sprite->zOrder);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createLightInspector(NovaEngine::Entity* entity) {
    auto* light = entity->getComponent<NovaEngine::LightComponent>();
    if (!light) return;

    LOG_DEBUG("Light: color=({},{},{},{}), radius={}, intensity={}",
        light->color.r, light->color.g, light->color.b, light->color.a,
        light->radius, light->intensity);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createColliderInspector(NovaEngine::Entity* entity) {
    auto* collider = entity->getComponent<NovaEngine::ColliderComponent>();
    if (!collider) return;

    LOG_DEBUG("Collider: size=({}, {}), isTrigger={}",
        collider->size.x, collider->size.y,
        collider->isTrigger);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createActivatorInspector(NovaEngine::Entity* entity) {
    auto* activator = entity->getComponent<NovaEngine::ActivatorComponent>();
    if (!activator) return;

    LOG_DEBUG("Activator: size=({}, {}), radius={}, active={}",
        activator->size.x, activator->size.y,
        activator->radius, activator->isActive);

    // TODO: Créer champs éditables avec UIManager
}

} // namespace NovaEditor
