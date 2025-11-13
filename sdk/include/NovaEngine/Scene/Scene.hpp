#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/Backend/Core/BackendTypes.hpp"
#include "NovaEngine/ECS/World.hpp"
#include <memory>
#include <string>

namespace NovaEngine {
namespace Scene {

    /**
     * @brief Type de scène
     */
    enum class SceneType {
        Interior,    // Intérieur (maison, bâtiment)
        Exterior,    // Extérieur (ville)
        Custom       // Type personnalisé
    };

    /**
     * @brief Classe de base abstraite pour toutes les scènes
     *
     * Une scène représente un espace de jeu distinct avec son propre World ECS.
     * Les scènes peuvent être chargées, déchargées et basculées entre elles.
     */
    class Scene {
    public:
        /**
         * @brief Constructeur
         * @param name Nom de la scène
         * @param type Type de la scène
         */
        Scene(const String& name, SceneType type = SceneType::Custom);

        virtual ~Scene();

        /**
         * @brief Appelé lors du chargement initial de la scène
         */
        virtual void onLoad() {}

        /**
         * @brief Appelé quand la scène devient active
         */
        virtual void onEnter() {}

        /**
         * @brief Appelé quand la scène devient inactive
         */
        virtual void onExit() {}

        /**
         * @brief Appelé lors du déchargement de la scène
         */
        virtual void onUnload() {}

        /**
         * @brief Met à jour la scène
         * @param deltaTime Temps écoulé depuis la dernière frame
         */
        virtual void update(f32 deltaTime);

        /**
         * @brief Effectue le rendu de la scène via les systèmes ECS
         */
        virtual void render();

        /**
         * @brief Gère les événements
         * @param event Événement du backend
         */
        virtual void handleEvent(const InputEvent& event) {}

        /**
         * @brief Retourne le World ECS de la scène
         */
        ECS::World& getWorld();

        /**
         * @brief Retourne le World ECS de la scène (version const)
         */
        const ECS::World& getWorld() const;

        /**
         * @brief Retourne le nom de la scène
         */
        const String& getName() const;

        /**
         * @brief Retourne le type de la scène
         */
        SceneType getType() const;

        /**
         * @brief Vérifie si la scène est chargée
         */
        bool isLoaded() const;

        /**
         * @brief Vérifie si la scène est active
         */
        bool isActive() const;

        /**
         * @brief Active ou désactive la mise à jour de la scène
         */
        void setPaused(bool paused);

        /**
         * @brief Vérifie si la scène est en pause
         */
        bool isPaused() const;

        /**
         * @brief Définit les métadonnées de la scène
         */
        void setMetadata(const String& key, const String& value);

        /**
         * @brief Récupère une métadonnée
         */
        String getMetadata(const String& key, const String& defaultValue = "") const;

    protected:
        /**
         * @brief Marque la scène comme chargée
         */
        void setLoaded(bool loaded);

        /**
         * @brief Marque la scène comme active
         */
        void setActive(bool active);

    private:
        String m_name;
        SceneType m_type;
        bool m_loaded;
        bool m_active;
        bool m_paused;

        Unique<ECS::World> m_world;
        std::unordered_map<String, String> m_metadata;

        friend class SceneManager;
    };

    /**
     * @brief Pointeur partagé vers une scène
     */
    using ScenePtr = Ref<Scene>;

} // namespace Scene
} // namespace NovaEngine
