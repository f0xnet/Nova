#include "../include/PlayModeManager.hpp"
#include "../include/SceneSerializer.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <sstream>

namespace NovaEditor {

PlayModeManager::PlayModeManager()
    : m_isPlaying(false)
    , m_isPaused(false)
    , m_playTime(0.0f)
    , m_frameCount(0)
{
}

void PlayModeManager::enterPlayMode(NovaEngine::Scene* scene) {
    if (!scene || m_isPlaying) return;

    LOG_INFO("=== ENTERING PLAY MODE ===");

    // Sauvegarder l'état de la scène
    saveSceneState(scene);

    m_isPlaying = true;
    m_isPaused = false;
    m_playTime = 0.0f;
    m_frameCount = 0;

    LOG_INFO("Play mode active - Press P to pause, ESC to exit");
}

void PlayModeManager::exitPlayMode(NovaEngine::Scene* scene) {
    if (!m_isPlaying) return;

    LOG_INFO("=== EXITING PLAY MODE ===");

    // Restaurer l'état de la scène
    restoreSceneState(scene);

    m_isPlaying = false;
    m_isPaused = false;

    LOG_INFO("Returned to edit mode - Play time: {:.2f}s, Frames: {}", m_playTime, m_frameCount);
}

void PlayModeManager::pause() {
    if (!m_isPlaying || m_isPaused) return;

    m_isPaused = true;
    LOG_INFO("Play mode PAUSED");
}

void PlayModeManager::resume() {
    if (!m_isPlaying || !m_isPaused) return;

    m_isPaused = false;
    LOG_INFO("Play mode RESUMED");
}

void PlayModeManager::step() {
    if (!m_isPlaying || !m_isPaused) return;

    // Permet un seul frame
    LOG_DEBUG("Step: 1 frame");
}

void PlayModeManager::update(float deltaTime) {
    if (!m_isPlaying || m_isPaused) return;

    m_playTime += deltaTime;
    m_frameCount++;

    // Log stats périodiquement
    if (m_frameCount % 300 == 0) {  // Toutes les 5 secondes à 60 FPS
        LOG_DEBUG("Play mode: {:.1f}s, {} frames", m_playTime, m_frameCount);
    }
}

void PlayModeManager::saveSceneState(NovaEngine::Scene* scene) {
    LOG_DEBUG("Saving scene state...");

    // Sérialiser la scène en string JSON
    SceneSerializer serializer;

    // Créer un fichier temporaire
    std::string tempPath = "editor_temp_playmode.json";
    if (serializer.saveScene(scene, tempPath)) {
        // Lire le contenu pour le garder en mémoire
        std::ifstream file(tempPath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            m_sceneSnapshot = buffer.str();
            file.close();
        }
        LOG_INFO("Scene state saved ({} bytes)", m_sceneSnapshot.size());
    } else {
        LOG_ERROR("Failed to save scene state");
    }
}

void PlayModeManager::restoreSceneState(NovaEngine::Scene* scene) {
    LOG_DEBUG("Restoring scene state...");

    if (m_sceneSnapshot.empty()) {
        LOG_WARN("No scene snapshot to restore");
        return;
    }

    // Écrire le snapshot dans un fichier temporaire
    std::string tempPath = "editor_temp_playmode.json";
    std::ofstream file(tempPath);
    if (file.is_open()) {
        file << m_sceneSnapshot;
        file.close();

        // Charger depuis le fichier
        SceneSerializer serializer;
        if (serializer.loadScene(scene, tempPath)) {
            LOG_INFO("Scene state restored");
        } else {
            LOG_ERROR("Failed to restore scene state");
        }

        // Nettoyer le fichier temporaire
        std::remove(tempPath.c_str());
    }

    m_sceneSnapshot.clear();
}

} // namespace NovaEditor
