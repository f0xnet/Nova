#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <string>
#include <vector>
#include <deque>

namespace NovaEditor {

struct ConsoleMessage {
    NovaEngine::LogLevel level;
    std::string message;
    std::string module;
    float timestamp;
};

class EditorConsole {
public:
    EditorConsole();
    
    void addMessage(NovaEngine::LogLevel level, const std::string& message, const std::string& module = "Editor");
    void clear();
    
    const std::deque<ConsoleMessage>& getMessages() const { return m_messages; }
    void setMaxMessages(size_t max) { m_maxMessages = max; }
    
    void setLevelFilter(NovaEngine::LogLevel minLevel) { m_minLevel = minLevel; }
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
private:
    std::deque<ConsoleMessage> m_messages;
    size_t m_maxMessages;
    NovaEngine::LogLevel m_minLevel;
    bool m_visible;
    float m_time;
};

} // namespace NovaEditor
