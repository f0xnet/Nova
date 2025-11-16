#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <fstream>

namespace NovaEngine {

/**
 * @brief Connection between two scenes
 */
struct SceneConnection {
    std::string fromScene;
    std::string toScene;
    Vec2f exitPortalPos;      // Position du portail de sortie dans fromScene
    Vec2f entryPortalPos;     // Position d'arrivée dans toScene
    f32 travelTime;           // Temps de traversée estimé (secondes)
    bool bidirectional;       // Connexion dans les 2 sens?
};

/**
 * @brief Graph of scene connections for multi-scene pathfinding
 *
 * The SceneGraph manages connections between scenes and provides
 * pathfinding to ensure NPCs physically traverse all intermediate
 * scenes instead of teleporting.
 */
class SceneGraph {
private:
    // Adjacency list: scene name -> list of connections
    std::unordered_map<std::string, std::vector<SceneConnection>> m_connections;

public:
    /**
     * @brief Add a connection between two scenes
     */
    void addConnection(const SceneConnection& connection) {
        m_connections[connection.fromScene].push_back(connection);

        // Si bidirectionnel, ajouter la connexion inverse
        if (connection.bidirectional) {
            SceneConnection reverse = connection;
            reverse.fromScene = connection.toScene;
            reverse.toScene = connection.fromScene;
            reverse.exitPortalPos = connection.entryPortalPos;
            reverse.entryPortalPos = connection.exitPortalPos;
            m_connections[reverse.fromScene].push_back(reverse);
        }
    }

    /**
     * @brief Load scene graph from JSON file
     */
    bool loadFromJSON(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open scene graph: {}", path);
            return false;
        }

        nlohmann::json data;
        try {
            file >> data;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse scene graph JSON: {}", e.what());
            return false;
        }

        if (data.contains("connections")) {
            for (const auto& conn : data["connections"]) {
                SceneConnection connection;
                connection.fromScene = conn["from"];
                connection.toScene = conn["to"];
                connection.exitPortalPos = Vec2f{
                    conn["exitPortal"][0].get<f32>(),
                    conn["exitPortal"][1].get<f32>()
                };
                connection.entryPortalPos = Vec2f{
                    conn["entryPortal"][0].get<f32>(),
                    conn["entryPortal"][1].get<f32>()
                };
                connection.travelTime = conn.value("travelTime", 5.0f);
                connection.bidirectional = conn.value("bidirectional", true);

                addConnection(connection);
            }
        }

        LOG_INFO("Loaded scene graph with {} scenes", m_connections.size());
        return true;
    }

    /**
     * @brief Find shortest path between two scenes using BFS
     * @param startScene Starting scene
     * @param endScene Destination scene
     * @return List of scenes to traverse (including start and end), empty if no path
     */
    std::vector<std::string> findPath(const std::string& startScene,
                                      const std::string& endScene) {
        if (startScene == endScene) {
            return {startScene};
        }

        // BFS pour trouver le chemin le plus court
        std::queue<std::string> queue;
        std::unordered_map<std::string, std::string> cameFrom;
        std::unordered_set<std::string> visited;

        queue.push(startScene);
        visited.insert(startScene);
        cameFrom[startScene] = "";

        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();

            if (current == endScene) {
                // Reconstruire le chemin
                std::vector<std::string> path;
                std::string node = endScene;
                while (!node.empty()) {
                    path.push_back(node);
                    node = cameFrom[node];
                }
                std::reverse(path.begin(), path.end());

                LOG_DEBUG("Found path from '{}' to '{}': {} scenes",
                         startScene, endScene, path.size());
                for (size_t i = 0; i < path.size(); ++i) {
                    LOG_DEBUG("  [{}] {}", i, path[i]);
                }

                return path;
            }

            // Explorer les voisins
            auto it = m_connections.find(current);
            if (it != m_connections.end()) {
                for (const auto& connection : it->second) {
                    if (visited.find(connection.toScene) == visited.end()) {
                        queue.push(connection.toScene);
                        visited.insert(connection.toScene);
                        cameFrom[connection.toScene] = current;
                    }
                }
            }
        }

        LOG_WARN("No path found from '{}' to '{}'", startScene, endScene);
        return {};  // Pas de chemin trouvé
    }

    /**
     * @brief Get connection details between two adjacent scenes
     * @return Pointer to connection, or nullptr if not connected
     */
    const SceneConnection* getConnection(const std::string& fromScene,
                                         const std::string& toScene) const {
        auto it = m_connections.find(fromScene);
        if (it != m_connections.end()) {
            for (const auto& conn : it->second) {
                if (conn.toScene == toScene) {
                    return &conn;
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief Get all scenes directly connected to a scene
     * @return List of neighboring scene names
     */
    std::vector<std::string> getNeighbors(const std::string& sceneName) const {
        std::vector<std::string> neighbors;
        auto it = m_connections.find(sceneName);
        if (it != m_connections.end()) {
            for (const auto& conn : it->second) {
                neighbors.push_back(conn.toScene);
            }
        }
        return neighbors;
    }

    /**
     * @brief Check if two scenes are directly connected
     */
    bool areConnected(const std::string& sceneA, const std::string& sceneB) const {
        return getConnection(sceneA, sceneB) != nullptr;
    }

    /**
     * @brief Get total number of scenes in the graph
     */
    size_t getSceneCount() const {
        return m_connections.size();
    }
};

} // namespace NovaEngine
