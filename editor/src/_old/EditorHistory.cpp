#include "../include/EditorHistory.hpp"
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEditor {

EditorHistory::EditorHistory()
    : m_maxHistorySize(100)
{
}

void EditorHistory::executeCommand(std::unique_ptr<EditorCommand> command) {
    if (!command) return;

    // Exécuter la commande
    command->execute();

    // Ajouter à l'historique
    m_undoStack.push_back(std::move(command));

    // Limiter la taille de l'historique
    if (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.erase(m_undoStack.begin());
    }

    // Vider la redo stack (nouvelle branche d'historique)
    m_redoStack.clear();

    LOG_DEBUG("Command executed: {}", m_undoStack.back()->getName());
}

bool EditorHistory::canUndo() const {
    return !m_undoStack.empty();
}

bool EditorHistory::canRedo() const {
    return !m_redoStack.empty();
}

void EditorHistory::undo() {
    if (!canUndo()) return;

    // Prendre la dernière commande
    auto command = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    // Annuler la commande
    command->undo();
    LOG_INFO("Undo: {}", command->getName());

    // Ajouter à la redo stack
    m_redoStack.push_back(std::move(command));
}

void EditorHistory::redo() {
    if (!canRedo()) return;

    // Prendre la dernière commande annulée
    auto command = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    // Ré-exécuter la commande
    command->execute();
    LOG_INFO("Redo: {}", command->getName());

    // Remettre dans l'undo stack
    m_undoStack.push_back(std::move(command));
}

void EditorHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    LOG_DEBUG("History cleared");
}

void EditorHistory::setMaxHistorySize(size_t size) {
    m_maxHistorySize = size;

    // Réduire si nécessaire
    while (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

std::string EditorHistory::getUndoCommandName() const {
    if (!canUndo()) return "";
    return m_undoStack.back()->getName();
}

std::string EditorHistory::getRedoCommandName() const {
    if (!canRedo()) return "";
    return m_redoStack.back()->getName();
}

size_t EditorHistory::getHistorySize() const {
    return m_undoStack.size() + m_redoStack.size();
}

} // namespace NovaEditor
