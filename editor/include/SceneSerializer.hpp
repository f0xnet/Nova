#pragma once

#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/Core/Types.hpp>
#include <string>
#include <vector>

namespace NovaEditor {

/**
 * @brief Sérialiseur/Désérialiseur de scènes JSON
 *
 * Permet de sauvegarder et charger des scènes depuis l'éditeur
 */
class SceneSerializer {
public:
    SceneSerializer() = default;
    ~SceneSerializer() = default;

    /**
     * @brief Sauvegarde une scène en JSON
     * @param scene Scène à sauvegarder
     * @param filepath Chemin du fichier de sortie
     * @return true si succès
     */
    bool saveScene(NovaEngine::Scene* scene, const std::string& filepath);

    /**
     * @brief Charge une scène depuis JSON
     * @param scene Scène cible (sera vidée puis remplie)
     * @param filepath Chemin du fichier à charger
     * @return true si succès
     */
    bool loadScene(NovaEngine::Scene* scene, const std::string& filepath);

    /**
     * @brief Valide une scène (checks d'intégrité)
     * @param scene Scène à valider
     * @return Liste des warnings/erreurs
     */
    std::vector<std::string> validateScene(NovaEngine::Scene* scene);

    /**
     * @brief Exporte une sélection d'entités en JSON
     * @param entities Entités à exporter
     * @param filepath Chemin du fichier de sortie
     * @return true si succès
     */
    bool exportEntities(const std::vector<NovaEngine::Entity*>& entities, const std::string& filepath);

    /**
     * @brief Importe des entités depuis JSON
     * @param scene Scène cible
     * @param filepath Chemin du fichier à importer
     * @return Entités créées
     */
    std::vector<NovaEngine::Entity*> importEntities(NovaEngine::Scene* scene, const std::string& filepath);

private:
    // Helpers pour sérialisation
    nlohmann::json serializeEntity(NovaEngine::Entity* entity);
    void deserializeEntity(NovaEngine::Entity* entity, const nlohmann::json& json);
};

} // namespace NovaEditor
