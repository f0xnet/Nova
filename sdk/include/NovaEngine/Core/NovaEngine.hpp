#pragma once

#include "Types.hpp"
#include "Logger.hpp"
#include <string>
#include <memory>

namespace NovaEngine {

    class ResourceManager; // Forward declaration
    class EventDispatcher;
    struct DisplayConfig;   // Forward declaration

    /**
     * @brief Classe principale du moteur NovaEngine.
     * Gère la fenêtre, la boucle principale, le système d'événements et les ressources.
     */
    class NovaEngine {
    public:
        /**
         * @brief Retourne l'instance unique du moteur.
         */
        static NovaEngine& get();

        /**
         * @brief Initialise le moteur avec les paramètres de la fenêtre.
         * @return true si succès, false sinon.
         */
        bool initialize(const std::string& title, u32 width, u32 height, bool fullscreen = false);

        /**
         * @brief Initialise le moteur en utilisant un fichier de configuration.
         * @param title Titre de la fenêtre
         * @param configPath Chemin vers le fichier de configuration
         * @return true si succès, false sinon.
         */
        bool initializeWithConfig(const std::string& title, const std::string& configPath = "config/engine.ini");

        /**
         * @brief Arrête le moteur proprement.
         */
        void shutdown();

        /**
         * @brief Accès au gestionnaire de ressources.
         */
        ResourceManager& getResourceManager();

        /**
         * @brief Accès au système d'événements.
         */
        EventDispatcher& getEventDispatcher();

        /**
         * @brief Vérifie si le moteur est en cours d'exécution.
         */
        bool isRunning() const;

        // Nouvelles méthodes pour la configuration
        
        /**
         * @brief Applique les paramètres d'affichage.
         */
        void applyDisplaySettings(const DisplayConfig& config);

        /**
         * @brief Récupère la configuration d'affichage actuelle.
         */
        const DisplayConfig& getDisplayConfig() const;

        /**
         * @brief Définit une nouvelle configuration d'affichage.
         */
        void setDisplayConfig(const DisplayConfig& config);

        /**
         * @brief Récupère la largeur de la fenêtre.
         */
        u32 getWidth() const;

        /**
         * @brief Récupère la hauteur de la fenêtre.
         */
        u32 getHeight() const;

        /**
         * @brief Vérifie si la fenêtre est en plein écran.
         */
        bool isFullscreen() const;

        /**
         * @brief Bascule entre fenêtré et plein écran.
         */
        void toggleFullscreen();

        /**
         * @brief Change le titre de la fenêtre.
         */
        void setTitle(const std::string& title);

    private:
        NovaEngine();
        ~NovaEngine();

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace NovaEngine