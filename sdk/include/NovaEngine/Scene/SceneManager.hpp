#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/Backend/Core/BackendTypes.hpp"
#include "Scene.hpp"
#include <unordered_map>
#include <stack>
#include <functional>

namespace NovaEngine {
namespace Scene {

    /**
     * @brief Données de transition entre scènes
     */
    struct SceneTransitionData {
        String fromScene;       // Scène d'origine
        String toScene;         // Scène de destination
        String transitionId;    // ID de la transition
        f32 duration;           // Durée de la transition
        bool fadeOut;           // Fade out activé
        bool fadeIn;            // Fade in activé

        SceneTransitionData()
            : fromScene(""), toScene(""), transitionId(""),
              duration(0.5f), fadeOut(true), fadeIn(true) {}
    };

    /**
     * @brief Callback appelé lors d'une transition de scène
     */
    using SceneTransitionCallback = std::function<void(const SceneTransitionData&)>;

    /**
     * @brief Gestionnaire de scènes
     *
     * Le SceneManager gère le cycle de vie des scènes, les transitions
     * et la pile de scènes actives.
     */
    class SceneManager {
    public:
        SceneManager();
        ~SceneManager();

        /**
         * @brief Ajoute une scène au gestionnaire
         * @param scene Scène à ajouter
         * @param load Charger immédiatement la scène
         */
        void addScene(ScenePtr scene, bool load = false);

        /**
         * @brief Retire une scène du gestionnaire
         * @param sceneName Nom de la scène à retirer
         */
        void removeScene(const String& sceneName);

        /**
         * @brief Charge une scène
         * @param sceneName Nom de la scène à charger
         */
        void loadScene(const String& sceneName);

        /**
         * @brief Décharge une scène
         * @param sceneName Nom de la scène à décharger
         */
        void unloadScene(const String& sceneName);

        /**
         * @brief Change la scène active
         * @param sceneName Nom de la scène à activer
         * @param transitionData Données de transition optionnelles
         */
        void changeScene(const String& sceneName, const SceneTransitionData& transitionData = SceneTransitionData());

        /**
         * @brief Empile une nouvelle scène (garde l'ancienne en arrière-plan)
         * @param sceneName Nom de la scène à empiler
         */
        void pushScene(const String& sceneName);

        /**
         * @brief Dépile la scène actuelle (retourne à la précédente)
         */
        void popScene();

        /**
         * @brief Récupère une scène par son nom
         */
        ScenePtr getScene(const String& sceneName);

        /**
         * @brief Récupère la scène actuellement active
         */
        ScenePtr getActiveScene();

        /**
         * @brief Récupère la scène actuellement active (version const)
         */
        const ScenePtr getActiveScene() const;

        /**
         * @brief Vérifie si une scène existe
         */
        bool hasScene(const String& sceneName) const;

        /**
         * @brief Met à jour la scène active
         * @param deltaTime Temps écoulé depuis la dernière frame
         */
        void update(f32 deltaTime);

        /**
         * @brief Effectue le rendu de la scène active
         */
        void render();

        /**
         * @brief Gère les événements pour la scène active
         * @param event Événement du backend
         */
        void handleEvent(const InputEvent& event);

        /**
         * @brief Enregistre un callback de transition
         */
        void setTransitionCallback(SceneTransitionCallback callback);

        /**
         * @brief Vérifie si une transition est en cours
         */
        bool isTransitioning() const;

        /**
         * @brief Récupère le progrès de la transition actuelle (0.0 à 1.0)
         */
        f32 getTransitionProgress() const;

        /**
         * @brief Vide toutes les scènes
         */
        void clear();

        /**
         * @brief Récupère le nombre de scènes gérées
         */
        size_t getSceneCount() const;

        /**
         * @brief Récupère tous les noms de scènes
         */
        std::vector<String> getSceneNames() const;

    private:
        /**
         * @brief Met à jour la transition en cours
         */
        void updateTransition(f32 deltaTime);

        /**
         * @brief Commence une transition
         */
        void beginTransition(const SceneTransitionData& data);

        /**
         * @brief Termine la transition
         */
        void endTransition();

        // Scènes gérées
        std::unordered_map<String, ScenePtr> m_scenes;

        // Pile de scènes actives
        std::stack<ScenePtr> m_sceneStack;

        // Scène actuellement active
        ScenePtr m_activeScene;

        // Transition
        bool m_transitioning;
        f32 m_transitionProgress;
        SceneTransitionData m_currentTransition;
        ScenePtr m_transitionTargetScene;
        SceneTransitionCallback m_transitionCallback;
    };

} // namespace Scene
} // namespace NovaEngine
