#pragma once

#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/Core/Types.hpp>

namespace NovaEditor {

class DebugRenderer {
public:
    DebugRenderer();
    
    void renderColliders(NovaEngine::Scene* scene);
    void renderLights(NovaEngine::Scene* scene);
    void renderWaypoints(NovaEngine::Scene* scene);
    void renderActivators(NovaEngine::Scene* scene);
    
    void setShowColliders(bool show) { m_showColliders = show; }
    void setShowLights(bool show) { m_showLights = show; }
    void setShowWaypoints(bool show) { m_showWaypoints = show; }
    void setShowActivators(bool show) { m_showActivators = show; }
    
    bool showColliders() const { return m_showColliders; }
    bool showLights() const { return m_showLights; }
    bool showWaypoints() const { return m_showWaypoints; }
    bool showActivators() const { return m_showActivators; }
    
private:
    bool m_showColliders;
    bool m_showLights;
    bool m_showWaypoints;
    bool m_showActivators;
};

} // namespace NovaEditor
