#pragma once

#include "Scene.hpp"
#include "NovaEngine/Backend/Core/BackendTypes.hpp"
#include <vector>

namespace NovaEngine {
namespace Scene {

    /**
     * @brief Structure représentant un point d'entrée/sortie dans une scène intérieure
     */
    struct SceneTransition {
        String id;                      // Identifiant unique du point de transition
        Vec2f position;                 // Position du point de transition
        String targetScene;             // Nom de la scène de destination
        String targetTransition;        // ID du point de transition de destination
        Rect triggerZone;               // Zone qui déclenche la transition
        bool enabled;                   // Activation de la transition

        SceneTransition()
            : id(""), position(0, 0), targetScene(""), targetTransition(""),
              triggerZone(0, 0, 0, 0), enabled(true) {}
    };

    /**
     * @brief Classe pour les scènes intérieures (maisons, bâtiments, donjons, etc.)
     *
     * Les scènes intérieures sont des espaces fermés avec des points d'entrée/sortie définis.
     * Elles peuvent avoir plusieurs transitions vers d'autres scènes.
     */
    class InteriorScene : public Scene {
    public:
        /**
         * @brief Constructeur
         * @param name Nom de la scène
         * @param interiorType Type d'intérieur (ex: "house", "shop", "dungeon")
         */
        InteriorScene(const String& name, const String& interiorType = "generic");

        ~InteriorScene() override = default;

        /**
         * @brief Ajoute un point de transition
         */
        void addTransition(const SceneTransition& transition);

        /**
         * @brief Retire un point de transition par son ID
         */
        void removeTransition(const String& transitionId);

        /**
         * @brief Récupère un point de transition par son ID
         */
        SceneTransition* getTransition(const String& transitionId);

        /**
         * @brief Récupère tous les points de transition
         */
        const std::vector<SceneTransition>& getTransitions() const;

        /**
         * @brief Vérifie si une position déclenche une transition
         * @return Pointeur vers la transition déclenchée ou nullptr
         */
        const SceneTransition* checkTransitionTrigger(const Vec2f& position) const;

        /**
         * @brief Définit les limites de la scène intérieure
         */
        void setBounds(const Rect& bounds);

        /**
         * @brief Récupère les limites de la scène
         */
        const Rect& getBounds() const;

        /**
         * @brief Définit le type d'intérieur
         */
        void setInteriorType(const String& type);

        /**
         * @brief Récupère le type d'intérieur
         */
        const String& getInteriorType() const;

        /**
         * @brief Définit l'éclairage ambiant (0.0 = noir, 1.0 = lumineux)
         */
        void setAmbientLight(f32 light);

        /**
         * @brief Récupère l'éclairage ambiant
         */
        f32 getAmbientLight() const;

        /**
         * @brief Appelé lors de l'entrée dans la scène
         */
        void onEnter() override;

        /**
         * @brief Appelé lors de la sortie de la scène
         */
        void onExit() override;

        /**
         * @brief Met à jour la scène
         */
        void update(f32 deltaTime) override;

        /**
         * @brief Gère les événements
         */
        void handleEvent(const InputEvent& event) override;

    protected:
        /**
         * @brief Méthode virtuelle appelée quand une transition est déclenchée
         */
        virtual void onTransitionTriggered(const SceneTransition& transition) {}

    private:
        String m_interiorType;
        Rect m_bounds;
        f32 m_ambientLight;
        std::vector<SceneTransition> m_transitions;
    };

} // namespace Scene
} // namespace NovaEngine
