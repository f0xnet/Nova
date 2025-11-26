#pragma once

#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Core/Types.hpp>
#include <memory>
#include <vector>
#include <string>

namespace NovaEditor {

/**
 * @brief Command pattern pour Undo/Redo
 */
class EditorCommand {
public:
    virtual ~EditorCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
};

/**
 * @brief Commande de déplacement d'entité
 */
class MoveEntityCommand : public EditorCommand {
public:
    MoveEntityCommand(NovaEngine::Entity* entity, const NovaEngine::Vec2f& oldPos, const NovaEngine::Vec2f& newPos)
        : m_entity(entity), m_oldPosition(oldPos), m_newPosition(newPos) {}

    void execute() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->position = m_newPosition;
    }

    void undo() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->position = m_oldPosition;
    }

    std::string getName() const override { return "Move Entity"; }

private:
    NovaEngine::Entity* m_entity;
    NovaEngine::Vec2f m_oldPosition;
    NovaEngine::Vec2f m_newPosition;
};

/**
 * @brief Commande de rotation d'entité
 */
class RotateEntityCommand : public EditorCommand {
public:
    RotateEntityCommand(NovaEngine::Entity* entity, float oldRot, float newRot)
        : m_entity(entity), m_oldRotation(oldRot), m_newRotation(newRot) {}

    void execute() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->rotation = m_newRotation;
    }

    void undo() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->rotation = m_oldRotation;
    }

    std::string getName() const override { return "Rotate Entity"; }

private:
    NovaEngine::Entity* m_entity;
    float m_oldRotation;
    float m_newRotation;
};

/**
 * @brief Commande de mise à l'échelle d'entité
 */
class ScaleEntityCommand : public EditorCommand {
public:
    ScaleEntityCommand(NovaEngine::Entity* entity, const NovaEngine::Vec2f& oldScale, const NovaEngine::Vec2f& newScale)
        : m_entity(entity), m_oldScale(oldScale), m_newScale(newScale) {}

    void execute() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->scale = m_newScale;
    }

    void undo() override {
        auto* transform = m_entity->getComponent<NovaEngine::TransformComponent>();
        if (transform) transform->scale = m_oldScale;
    }

    std::string getName() const override { return "Scale Entity"; }

private:
    NovaEngine::Entity* m_entity;
    NovaEngine::Vec2f m_oldScale;
    NovaEngine::Vec2f m_newScale;
};

/**
 * @brief Gestionnaire d'historique Undo/Redo
 */
class EditorHistory {
public:
    EditorHistory();
    ~EditorHistory() = default;

    // Exécuter une commande et l'ajouter à l'historique
    void executeCommand(std::unique_ptr<EditorCommand> command);

    // Undo/Redo
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // Gestion
    void clear();
    void setMaxHistorySize(size_t size);

    // Info
    std::string getUndoCommandName() const;
    std::string getRedoCommandName() const;
    size_t getHistorySize() const;

private:
    std::vector<std::unique_ptr<EditorCommand>> m_undoStack;
    std::vector<std::unique_ptr<EditorCommand>> m_redoStack;
    size_t m_maxHistorySize;
};

} // namespace NovaEditor
