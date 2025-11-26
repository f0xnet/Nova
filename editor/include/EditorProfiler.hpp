#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <string>
#include <chrono>

namespace NovaEditor {

struct ProfilerStats {
    float fps;
    float frameTime;  // ms
    NovaEngine::u32 entityCount;
    NovaEngine::u32 componentCount;
    NovaEngine::u32 systemCount;
    NovaEngine::u64 memoryUsage;  // bytes
    
    NovaEngine::u32 transformCount;
    NovaEngine::u32 spriteCount;
    NovaEngine::u32 lightCount;
    NovaEngine::u32 colliderCount;
};

class EditorProfiler {
public:
    EditorProfiler();
    
    void beginFrame();
    void endFrame();
    void update();
    
    const ProfilerStats& getStats() const { return m_stats; }
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
private:
    ProfilerStats m_stats;
    std::chrono::high_resolution_clock::time_point m_frameStart;
    bool m_visible;
    float m_updateTimer;
};

} // namespace NovaEditor
