#include "../include/DebugRenderer.hpp"
#include "../include/Gizmos.hpp"
#include <NovaEngine/ECS/Components.hpp>

namespace NovaEditor {

DebugRenderer::DebugRenderer()
    : m_showColliders(true)
    , m_showLights(true)
    , m_showWaypoints(true)
    , m_showActivators(true)
{}

void DebugRenderer::renderColliders(NovaEngine::Scene* scene) {
    if (!m_showColliders || !scene) return;
    
    Gizmos gizmos;
    auto entities = scene->getEntityRegistry().getAllEntities();
    
    for (auto* entity : entities) {
        if (!entity->hasComponent<NovaEngine::ColliderComponent>()) continue;
        
        auto* collider = entity->getComponent<NovaEngine::ColliderComponent>();
        auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
        if (!transform) continue;
        
        NovaEngine::Color color = collider->isTrigger ? 
            NovaEngine::Color{255, 255, 0, 100} : 
            NovaEngine::Color{0, 255, 0, 100};
        
        gizmos.renderBounds(transform->position, collider->size, color);
    }
}

void DebugRenderer::renderLights(NovaEngine::Scene* scene) {
    if (!m_showLights || !scene) return;
    
    Gizmos gizmos;
    auto entities = scene->getEntityRegistry().getAllEntities();
    
    for (auto* entity : entities) {
        if (!entity->hasComponent<NovaEngine::LightComponent>()) continue;
        
        auto* light = entity->getComponent<NovaEngine::LightComponent>();
        auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
        if (!transform) continue;
        
        gizmos.renderLight(transform->position, light->radius, light->color);
    }
}

void DebugRenderer::renderWaypoints(NovaEngine::Scene* scene) {
    if (!m_showWaypoints || !scene) return;
    // TODO: Implement waypoint rendering
}

void DebugRenderer::renderActivators(NovaEngine::Scene* scene) {
    if (!m_showActivators || !scene) return;
    
    Gizmos gizmos;
    auto entities = scene->getEntityRegistry().getAllEntities();
    
    for (auto* entity : entities) {
        if (!entity->hasComponent<NovaEngine::ActivatorComponent>()) continue;
        
        auto* activator = entity->getComponent<NovaEngine::ActivatorComponent>();
        auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
        if (!transform) continue;
        
        NovaEngine::Color color = activator->isActive ?
            NovaEngine::Color{255, 0, 255, 150} :
            NovaEngine::Color{128, 0, 128, 100};
        
        gizmos.renderActivator(transform->position, activator->size, color);
    }
}

} // namespace NovaEditor
