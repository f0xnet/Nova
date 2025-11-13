#pragma once

#include "Scene.hpp"
#include "NovaEngine/Backend/Core/BackendTypes.hpp"
#include <vector>
#include <unordered_set>

namespace NovaEngine {
namespace Scene {

    /**
     * @brief Structure représentant une zone de chargement dans la scène extérieure
     */
    struct LoadingZone {
        String id;                      // Identifiant unique
        Rect bounds;                    // Zone de chargement
        std::vector<String> entities;   // Entités à charger dans cette zone
        bool loaded;                    // État de chargement

        LoadingZone() : id(""), bounds(0, 0, 0, 0), loaded(false) {}
    };

    /**
     * @brief Structure représentant un point d'entrée vers un intérieur
     */
    struct BuildingEntrance {
        String id;                      // Identifiant unique
        Vec2f position;                 // Position de l'entrée
        String interiorScene;           // Nom de la scène intérieure
        String transitionId;            // ID de la transition dans l'intérieur
        Rect triggerZone;               // Zone d'interaction
        bool locked;                    // Entrée verrouillée
        String displayName;             // Nom affiché (ex: "Maison de Marie")

        BuildingEntrance()
            : id(""), position(0, 0), interiorScene(""), transitionId(""),
              triggerZone(0, 0, 0, 0), locked(false), displayName("") {}
    };

    /**
     * @brief Classe pour la scène extérieure (ville, monde ouvert)
     *
     * La scène extérieure représente l'environnement principal non segmenté.
     * Elle gère le streaming de contenu et les entrées vers les intérieurs.
     */
    class ExteriorScene : public Scene {
    public:
        /**
         * @brief Constructeur
         * @param name Nom de la scène extérieure
         */
        explicit ExteriorScene(const String& name);

        ~ExteriorScene() override = default;

        /**
         * @brief Ajoute une entrée de bâtiment
         */
        void addBuildingEntrance(const BuildingEntrance& entrance);

        /**
         * @brief Retire une entrée de bâtiment
         */
        void removeBuildingEntrance(const String& entranceId);

        /**
         * @brief Récupère une entrée de bâtiment
         */
        BuildingEntrance* getBuildingEntrance(const String& entranceId);

        /**
         * @brief Récupère toutes les entrées de bâtiment
         */
        const std::vector<BuildingEntrance>& getBuildingEntrances() const;

        /**
         * @brief Vérifie si une position déclenche une entrée de bâtiment
         * @return Pointeur vers l'entrée déclenchée ou nullptr
         */
        const BuildingEntrance* checkEntranceTrigger(const Vec2f& position) const;

        /**
         * @brief Ajoute une zone de chargement
         */
        void addLoadingZone(const LoadingZone& zone);

        /**
         * @brief Retire une zone de chargement
         */
        void removeLoadingZone(const String& zoneId);

        /**
         * @brief Récupère toutes les zones de chargement
         */
        const std::vector<LoadingZone>& getLoadingZones() const;

        /**
         * @brief Met à jour les zones de chargement basées sur une position
         * @param position Position du joueur/caméra
         * @param loadRadius Rayon de chargement autour de la position
         */
        void updateLoadingZones(const Vec2f& position, f32 loadRadius);

        /**
         * @brief Définit les limites du monde extérieur
         */
        void setWorldBounds(const Rect& bounds);

        /**
         * @brief Récupère les limites du monde
         */
        const Rect& getWorldBounds() const;

        /**
         * @brief Active/désactive le streaming de contenu
         */
        void setStreamingEnabled(bool enabled);

        /**
         * @brief Vérifie si le streaming est activé
         */
        bool isStreamingEnabled() const;

        /**
         * @brief Définit le rayon de chargement par défaut
         */
        void setDefaultLoadRadius(f32 radius);

        /**
         * @brief Récupère le rayon de chargement par défaut
         */
        f32 getDefaultLoadRadius() const;

        /**
         * @brief Définit l'heure de la journée (0.0 = minuit, 0.5 = midi, 1.0 = minuit)
         */
        void setTimeOfDay(f32 time);

        /**
         * @brief Récupère l'heure de la journée
         */
        f32 getTimeOfDay() const;

        /**
         * @brief Définit la météo actuelle
         */
        void setWeather(const String& weather);

        /**
         * @brief Récupère la météo actuelle
         */
        const String& getWeather() const;

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
         * @brief Méthode virtuelle appelée quand une zone est chargée
         */
        virtual void onZoneLoaded(const LoadingZone& zone) {}

        /**
         * @brief Méthode virtuelle appelée quand une zone est déchargée
         */
        virtual void onZoneUnloaded(const LoadingZone& zone) {}

        /**
         * @brief Méthode virtuelle appelée quand une entrée est déclenchée
         */
        virtual void onEntranceTriggered(const BuildingEntrance& entrance) {}

    private:
        Rect m_worldBounds;
        std::vector<BuildingEntrance> m_buildingEntrances;
        std::vector<LoadingZone> m_loadingZones;
        std::unordered_set<String> m_loadedZones;

        bool m_streamingEnabled;
        f32 m_defaultLoadRadius;
        f32 m_timeOfDay;
        String m_weather;
    };

} // namespace Scene
} // namespace NovaEngine
