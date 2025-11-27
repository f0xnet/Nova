#include "../include/EditorProfiler.hpp"

namespace NovaEditor {

EditorProfiler::EditorProfiler()
    : m_visible(true)
    , m_updateTimer(0.0f)
{
    m_stats = {};
}

void EditorProfiler::beginFrame() {
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void EditorProfiler::endFrame() {
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - m_frameStart);
    
    m_stats.frameTime = duration.count() / 1000.0f;
    m_stats.fps = (m_stats.frameTime > 0) ? 1000.0f / m_stats.frameTime : 0.0f;
}

void EditorProfiler::update() {
    // Stats will be updated by EditorApplication
}

} // namespace NovaEditor
