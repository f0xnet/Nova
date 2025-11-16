#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>

namespace NovaEngine {

/**
 * @brief Représente un point de passage dans une scène
 *
 * Les waypoints permettent de définir des chemins fixes que les NPCs peuvent suivre.
 * Chaque waypoint peut avoir des tags pour permettre des chemins personnalisés.
 */
struct Waypoint {
    std::string id;              // Identifiant unique (ex: "fountain", "north_plaza")
    Vec2f position;              // Position dans la scène
    std::vector<std::string> tags; // Tags pour personnalité (ex: "main_road", "shortcut", "scenic")

    Waypoint() = default;
    Waypoint(const std::string& id_, const Vec2f& pos)
        : id(id_), position(pos) {}
    Waypoint(const std::string& id_, const Vec2f& pos, const std::vector<std::string>& tags_)
        : id(id_), position(pos), tags(tags_) {}
};

/**
 * @brief Représente une connexion entre deux waypoints
 */
struct WaypointConnection {
    std::string from;
    std::string to;
    f32 cost;                    // Coût de traversée (distance ou temps)
    std::vector<std::string> tags; // Tags pour filtrage (ex: "guard_patrol", "merchant_route")
    bool bidirectional;          // Connexion dans les deux sens?

    WaypointConnection() : cost(0.0f), bidirectional(true) {}
    WaypointConnection(const std::string& from_, const std::string& to_, f32 cost_ = 1.0f, bool bidir = true)
        : from(from_), to(to_), cost(cost_), bidirectional(bidir) {}
};

/**
 * @brief Graphe de waypoints pour le pathfinding dans une scène
 *
 * Permet de définir des chemins fixes et personnalisés pour les NPCs.
 * Utilise BFS pour trouver le chemin le plus court entre deux points.
 */
class WaypointGraph {
public:
    WaypointGraph() = default;
    ~WaypointGraph() = default;

    /**
     * @brief Charge le graphe de waypoints depuis un JSON
     *
     * Format attendu:
     * {
     *   "waypoints": [
     *     {"id": "fountain", "position": [640, 360], "tags": ["landmark", "center"]},
     *     ...
     *   ],
     *   "connections": [
     *     {"from": "fountain", "to": "north_plaza", "tags": ["main_road"], "bidirectional": true},
     *     ...
     *   ]
     * }
     */
    bool loadFromJSON(const nlohmann::json& json);

    /**
     * @brief Trouve le waypoint le plus proche d'une position donnée
     * @param position Position de recherche
     * @param maxDistance Distance maximale de recherche (optionnel)
     * @return Pointeur vers le waypoint le plus proche, ou nullptr si aucun trouvé
     */
    const Waypoint* findNearestWaypoint(const Vec2f& position, f32 maxDistance = -1.0f) const;

    /**
     * @brief Trouve un waypoint par son ID
     * @param id Identifiant du waypoint
     * @return Pointeur vers le waypoint, ou nullptr si non trouvé
     */
    const Waypoint* findWaypointByID(const std::string& id) const;

    /**
     * @brief Trouve le chemin entre deux positions
     * @param startPos Position de départ
     * @param endPos Position d'arrivée
     * @param preferredTags Tags préférés pour le filtrage des connexions (optionnel)
     * @return Liste des positions des waypoints formant le chemin (vide si aucun chemin)
     */
    std::vector<Vec2f> findPath(const Vec2f& startPos, const Vec2f& endPos,
                                const std::vector<std::string>& preferredTags = {}) const;

    /**
     * @brief Trouve le chemin entre deux waypoints (par ID)
     * @param startID ID du waypoint de départ
     * @param endID ID du waypoint d'arrivée
     * @param preferredTags Tags préférés pour le filtrage des connexions (optionnel)
     * @return Liste des IDs des waypoints formant le chemin (vide si aucun chemin)
     */
    std::vector<std::string> findPathByID(const std::string& startID, const std::string& endID,
                                          const std::vector<std::string>& preferredTags = {}) const;

    /**
     * @brief Obtient tous les waypoints
     */
    const std::vector<Waypoint>& getWaypoints() const { return m_waypoints; }

    /**
     * @brief Obtient toutes les connexions
     */
    const std::vector<WaypointConnection>& getConnections() const { return m_connections; }

    /**
     * @brief Vérifie si le graphe est vide
     */
    bool isEmpty() const { return m_waypoints.empty(); }

private:
    std::vector<Waypoint> m_waypoints;
    std::vector<WaypointConnection> m_connections;
    std::unordered_map<std::string, size_t> m_waypointIndices; // ID -> index dans m_waypoints

    /**
     * @brief Calcule la distance entre deux positions
     */
    f32 distance(const Vec2f& a, const Vec2f& b) const {
        f32 dx = b.x - a.x;
        f32 dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Reconstruit le chemin à partir de la map cameFrom
     */
    std::vector<std::string> reconstructPath(const std::unordered_map<std::string, std::string>& cameFrom,
                                             const std::string& start, const std::string& end) const;

    /**
     * @brief Vérifie si une connexion a au moins un des tags préférés
     */
    bool hasPreferredTag(const WaypointConnection& connection,
                        const std::vector<std::string>& preferredTags) const;
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline bool WaypointGraph::loadFromJSON(const nlohmann::json& json) {
    m_waypoints.clear();
    m_connections.clear();
    m_waypointIndices.clear();

    // Charger les waypoints
    if (json.contains("waypoints") && json["waypoints"].is_array()) {
        for (const auto& wpData : json["waypoints"]) {
            if (!wpData.contains("id") || !wpData.contains("position")) {
                LOG_ERROR("Waypoint missing 'id' or 'position'");
                continue;
            }

            Waypoint wp;
            wp.id = wpData["id"].get<std::string>();

            auto posArray = wpData["position"];
            if (posArray.is_array() && posArray.size() >= 2) {
                wp.position.x = posArray[0].get<f32>();
                wp.position.y = posArray[1].get<f32>();
            }

            // Tags optionnels
            if (wpData.contains("tags") && wpData["tags"].is_array()) {
                for (const auto& tag : wpData["tags"]) {
                    wp.tags.push_back(tag.get<std::string>());
                }
            }

            m_waypointIndices[wp.id] = m_waypoints.size();
            m_waypoints.push_back(wp);
        }
    }

    // Charger les connexions
    if (json.contains("connections") && json["connections"].is_array()) {
        for (const auto& connData : json["connections"]) {
            if (!connData.contains("from") || !connData.contains("to")) {
                LOG_ERROR("Connection missing 'from' or 'to'");
                continue;
            }

            WaypointConnection conn;
            conn.from = connData["from"].get<std::string>();
            conn.to = connData["to"].get<std::string>();

            // Coût optionnel (par défaut: distance euclidienne)
            if (connData.contains("cost")) {
                conn.cost = connData["cost"].get<f32>();
            } else {
                // Calculer le coût automatiquement depuis la distance
                const Waypoint* fromWp = findWaypointByID(conn.from);
                const Waypoint* toWp = findWaypointByID(conn.to);
                if (fromWp && toWp) {
                    conn.cost = distance(fromWp->position, toWp->position);
                } else {
                    conn.cost = 1.0f;
                }
            }

            // Bidirectionnel par défaut
            conn.bidirectional = connData.value("bidirectional", true);

            // Tags optionnels
            if (connData.contains("tags") && connData["tags"].is_array()) {
                for (const auto& tag : connData["tags"]) {
                    conn.tags.push_back(tag.get<std::string>());
                }
            }

            m_connections.push_back(conn);
        }
    }

    LOG_INFO("WaypointGraph loaded: {} waypoints, {} connections",
             m_waypoints.size(), m_connections.size());
    return !m_waypoints.empty();
}

inline const Waypoint* WaypointGraph::findNearestWaypoint(const Vec2f& position, f32 maxDistance) const {
    if (m_waypoints.empty()) return nullptr;

    const Waypoint* nearest = nullptr;
    f32 minDist = (maxDistance > 0) ? maxDistance : std::numeric_limits<f32>::max();

    for (const auto& wp : m_waypoints) {
        f32 dist = distance(position, wp.position);
        if (dist < minDist) {
            minDist = dist;
            nearest = &wp;
        }
    }

    return nearest;
}

inline const Waypoint* WaypointGraph::findWaypointByID(const std::string& id) const {
    auto it = m_waypointIndices.find(id);
    if (it != m_waypointIndices.end()) {
        return &m_waypoints[it->second];
    }
    return nullptr;
}

inline bool WaypointGraph::hasPreferredTag(const WaypointConnection& connection,
                                           const std::vector<std::string>& preferredTags) const {
    if (preferredTags.empty()) return true; // Pas de préférence = accepte tout

    for (const auto& tag : preferredTags) {
        for (const auto& connTag : connection.tags) {
            if (tag == connTag) return true;
        }
    }
    return false;
}

inline std::vector<std::string> WaypointGraph::reconstructPath(
    const std::unordered_map<std::string, std::string>& cameFrom,
    const std::string& start, const std::string& end) const {

    std::vector<std::string> path;
    std::string current = end;

    while (current != start) {
        path.push_back(current);
        auto it = cameFrom.find(current);
        if (it == cameFrom.end()) {
            // Pas de chemin trouvé
            return {};
        }
        current = it->second;
    }
    path.push_back(start);

    std::reverse(path.begin(), path.end());
    return path;
}

inline std::vector<std::string> WaypointGraph::findPathByID(
    const std::string& startID, const std::string& endID,
    const std::vector<std::string>& preferredTags) const {

    if (startID == endID) {
        return {startID};
    }

    // BFS pour trouver le chemin le plus court
    std::queue<std::string> queue;
    std::unordered_map<std::string, std::string> cameFrom;
    std::unordered_set<std::string> visited;

    queue.push(startID);
    visited.insert(startID);

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();

        if (current == endID) {
            return reconstructPath(cameFrom, startID, endID);
        }

        // Explorer les voisins
        for (const auto& conn : m_connections) {
            std::string neighbor;

            // Vérifier direction de la connexion
            if (conn.from == current) {
                neighbor = conn.to;
            } else if (conn.bidirectional && conn.to == current) {
                neighbor = conn.from;
            } else {
                continue; // Cette connexion ne concerne pas le noeud actuel
            }

            // Filtrer par tags préférés si spécifié
            if (!preferredTags.empty() && !hasPreferredTag(conn, preferredTags)) {
                continue; // Connexion ne correspond pas aux tags préférés
            }

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    // Aucun chemin trouvé
    LOG_WARN("No waypoint path found from '{}' to '{}'", startID, endID);
    return {};
}

inline std::vector<Vec2f> WaypointGraph::findPath(
    const Vec2f& startPos, const Vec2f& endPos,
    const std::vector<std::string>& preferredTags) const {

    // Trouver les waypoints les plus proches
    const Waypoint* startWp = findNearestWaypoint(startPos);
    const Waypoint* endWp = findNearestWaypoint(endPos);

    if (!startWp || !endWp) {
        LOG_WARN("Cannot find waypoints near start/end positions");
        return {};
    }

    // Trouver le chemin entre les waypoints
    auto pathIDs = findPathByID(startWp->id, endWp->id, preferredTags);

    if (pathIDs.empty()) {
        return {};
    }

    // Convertir les IDs en positions
    std::vector<Vec2f> path;
    path.reserve(pathIDs.size() + 2); // +2 pour start et end exacts

    // Ajouter position de départ exacte si différente du premier waypoint
    if (distance(startPos, startWp->position) > 5.0f) {
        path.push_back(startPos);
    }

    // Ajouter les waypoints
    for (const auto& id : pathIDs) {
        const Waypoint* wp = findWaypointByID(id);
        if (wp) {
            path.push_back(wp->position);
        }
    }

    // Ajouter position d'arrivée exacte si différente du dernier waypoint
    if (distance(endPos, endWp->position) > 5.0f) {
        path.push_back(endPos);
    }

    return path;
}

} // namespace NovaEngine
