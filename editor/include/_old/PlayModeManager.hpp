#pragma once

#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/Core/Types.hpp>
#include <memory>
#include <string>

namespace NovaEditor {

/**
 * @brief Gestionnaire du mode Play dans l'éditeur
 *
 * Permet de tester le jeu directement dans l'éditeur :
 * - Sauvegarde l'état de la scène
 * - Active tous les systems (Physics, AI, Audio)
 * - Permet de jouer
 * - Restaure l'état à la sortie
 */
class PlayModeManager {
public:
    PlayModeManager();
    ~PlayModeManager() = default;

    // Play mode
    void enterPlayMode(NovaEngine::Scene* scene);
    void exitPlayMode(NovaEngine::Scene* scene);
    bool isInPlayMode() const { return m_isPlaying; }

    // Pause
    void pause();
    void resume();
    bool isPaused() const { return m_isPaused; }

    // Step (frame by frame)
    void step();

    // Update (appelé pendant play mode)
    void update(float deltaTime);

    // Stats
    float getPlayTime() const { return m_playTime; }
    NovaEngine::u32 getFrameCount() const { return m_frameCount; }

private:
    void saveSceneState(NovaEngine::Scene* scene);
    void restoreSceneState(NovaEngine::Scene* scene);

private:
    bool m_isPlaying;
    bool m_isPaused;
    float m_playTime;
    NovaEngine::u32 m_frameCount;

    // Snapshot de la scène pour restauration
    std::string m_sceneSnapshot;
};

} // namespace NovaEditor
