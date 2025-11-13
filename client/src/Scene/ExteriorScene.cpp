#include "NovaEngine/Scene/ExteriorScene.hpp"

namespace NovaEngine {
namespace Scene {

    ExteriorScene::ExteriorScene(const String& name)
        : Scene(name, SceneType::Exterior),
          m_worldBounds(0, 0, 10000, 10000),
          m_streamingEnabled(true),
          m_defaultLoadRadius(2000.0f),
          m_timeOfDay(0.5f),
          m_weather("clear") {
        LOG_TRACE("ExteriorScene created: {}", name);
    }

    void ExteriorScene::addBuildingEntrance(const BuildingEntrance& entrance) {
        // Vérifier si une entrée avec cet ID existe déjà
        for (auto& existing : m_buildingEntrances) {
            if (existing.id == entrance.id) {
                LOG_WARN("Building entrance with ID {} already exists, replacing", entrance.id);
                existing = entrance;
                return;
            }
        }

        m_buildingEntrances.push_back(entrance);
        LOG_DEBUG("Building entrance added: {} -> {}", entrance.id, entrance.interiorScene);
    }

    void ExteriorScene::removeBuildingEntrance(const String& entranceId) {
        auto it = std::remove_if(m_buildingEntrances.begin(), m_buildingEntrances.end(),
            [&entranceId](const BuildingEntrance& e) {
                return e.id == entranceId;
            });

        if (it != m_buildingEntrances.end()) {
            m_buildingEntrances.erase(it, m_buildingEntrances.end());
            LOG_DEBUG("Building entrance removed: {}", entranceId);
        }
    }

    BuildingEntrance* ExteriorScene::getBuildingEntrance(const String& entranceId) {
        for (auto& entrance : m_buildingEntrances) {
            if (entrance.id == entranceId) {
                return &entrance;
            }
        }
        return nullptr;
    }

    const std::vector<BuildingEntrance>& ExteriorScene::getBuildingEntrances() const {
        return m_buildingEntrances;
    }

    const BuildingEntrance* ExteriorScene::checkEntranceTrigger(const Vec2f& position) const {
        for (const auto& entrance : m_buildingEntrances) {
            if (!entrance.locked && entrance.triggerZone.contains(position)) {
                return &entrance;
            }
        }
        return nullptr;
    }

    void ExteriorScene::addLoadingZone(const LoadingZone& zone) {
        // Vérifier si une zone avec cet ID existe déjà
        for (auto& existing : m_loadingZones) {
            if (existing.id == zone.id) {
                LOG_WARN("Loading zone with ID {} already exists, replacing", zone.id);
                existing = zone;
                return;
            }
        }

        m_loadingZones.push_back(zone);
        LOG_DEBUG("Loading zone added: {}", zone.id);
    }

    void ExteriorScene::removeLoadingZone(const String& zoneId) {
        auto it = std::remove_if(m_loadingZones.begin(), m_loadingZones.end(),
            [&zoneId](const LoadingZone& z) {
                return z.id == zoneId;
            });

        if (it != m_loadingZones.end()) {
            // Décharger la zone si elle était chargée
            if (m_loadedZones.find(zoneId) != m_loadedZones.end()) {
                m_loadedZones.erase(zoneId);
                onZoneUnloaded(*it);
            }

            m_loadingZones.erase(it, m_loadingZones.end());
            LOG_DEBUG("Loading zone removed: {}", zoneId);
        }
    }

    const std::vector<LoadingZone>& ExteriorScene::getLoadingZones() const {
        return m_loadingZones;
    }

    void ExteriorScene::updateLoadingZones(const Vec2f& position, f32 loadRadius) {
        if (!m_streamingEnabled) return;

        Rect loadArea(
            position.x - loadRadius,
            position.y - loadRadius,
            loadRadius * 2,
            loadRadius * 2
        );

        for (auto& zone : m_loadingZones) {
            // Vérifier si la zone intersecte avec la zone de chargement
            bool shouldBeLoaded = !(loadArea.left > zone.bounds.left + zone.bounds.width ||
                                   loadArea.left + loadArea.width < zone.bounds.left ||
                                   loadArea.top > zone.bounds.top + zone.bounds.height ||
                                   loadArea.top + loadArea.height < zone.bounds.top);
            bool isLoaded = m_loadedZones.find(zone.id) != m_loadedZones.end();

            if (shouldBeLoaded && !isLoaded) {
                // Charger la zone
                zone.loaded = true;
                m_loadedZones.insert(zone.id);
                onZoneLoaded(zone);
                LOG_DEBUG("Zone loaded: {}", zone.id);
            } else if (!shouldBeLoaded && isLoaded) {
                // Décharger la zone
                zone.loaded = false;
                m_loadedZones.erase(zone.id);
                onZoneUnloaded(zone);
                LOG_DEBUG("Zone unloaded: {}", zone.id);
            }
        }
    }

    void ExteriorScene::setWorldBounds(const Rect& bounds) {
        m_worldBounds = bounds;
    }

    const Rect& ExteriorScene::getWorldBounds() const {
        return m_worldBounds;
    }

    void ExteriorScene::setStreamingEnabled(bool enabled) {
        m_streamingEnabled = enabled;

        if (!enabled) {
            // Charger toutes les zones
            for (auto& zone : m_loadingZones) {
                if (!zone.loaded) {
                    zone.loaded = true;
                    m_loadedZones.insert(zone.id);
                    onZoneLoaded(zone);
                }
            }
        }
    }

    bool ExteriorScene::isStreamingEnabled() const {
        return m_streamingEnabled;
    }

    void ExteriorScene::setDefaultLoadRadius(f32 radius) {
        m_defaultLoadRadius = radius;
    }

    f32 ExteriorScene::getDefaultLoadRadius() const {
        return m_defaultLoadRadius;
    }

    void ExteriorScene::setTimeOfDay(f32 time) {
        // Normaliser entre 0.0 et 1.0
        while (time < 0.0f) time += 1.0f;
        while (time > 1.0f) time -= 1.0f;
        m_timeOfDay = time;
    }

    f32 ExteriorScene::getTimeOfDay() const {
        return m_timeOfDay;
    }

    void ExteriorScene::setWeather(const String& weather) {
        m_weather = weather;
        LOG_DEBUG("Weather changed to: {}", weather);
    }

    const String& ExteriorScene::getWeather() const {
        return m_weather;
    }

    void ExteriorScene::onEnter() {
        LOG_INFO("Entering exterior scene: {}", getName());
    }

    void ExteriorScene::onExit() {
        LOG_INFO("Exiting exterior scene: {}", getName());
    }

    void ExteriorScene::update(f32 deltaTime) {
        Scene::update(deltaTime);

        // Logique spécifique aux extérieurs peut être ajoutée ici
        // Par exemple, mise à jour de l'heure, de la météo, etc.
    }

    void ExteriorScene::handleEvent(const InputEvent& event) {
        // Gestion des événements spécifiques aux extérieurs
    }

} // namespace Scene
} // namespace NovaEngine
