#include "../include/EditorConsole.hpp"

namespace NovaEditor {

EditorConsole::EditorConsole()
    : m_maxMessages(1000)
    , m_minLevel(NovaEngine::LogLevel::Debug)
    , m_visible(true)
    , m_time(0.0f)
{}

void EditorConsole::addMessage(NovaEngine::LogLevel level, const std::string& message, const std::string& module) {
    if (level < m_minLevel) return;
    
    ConsoleMessage msg;
    msg.level = level;
    msg.message = message;
    msg.module = module;
    msg.timestamp = m_time++;
    
    m_messages.push_back(msg);
    
    if (m_messages.size() > m_maxMessages) {
        m_messages.pop_front();
    }
}

void EditorConsole::clear() {
    m_messages.clear();
}

} // namespace NovaEditor
