#pragma once

#include "../Core/Types.hpp"
#include <nlohmann/json.hpp>
#include <vector>

namespace NovaEngine {

    /**
     * @brief Structure représentant une ressource avec ID et chemin.
     */
    struct ResourceEntry {
        ID id;
        std::string path;
    };

    /**
     * @brief Structure représentant la définition d'un pack de ressources.
     */
    struct ResourcePack {
        std::vector<ResourceEntry> textures;
        std::vector<ResourceEntry> fonts;
        std::vector<ResourceEntry> sounds;
        std::vector<ResourceEntry> musics;
    };

    /**
     * @brief Convertit un JSON en ResourcePack.
     */
    void from_json(const nlohmann::json& j, ResourcePack& pack);

} // namespace NovaEngine