#include "NovaEngine/Resources/ResourceTypes.hpp"

namespace NovaEngine {

    void from_json(const nlohmann::json& j, ResourcePack& pack) {
        if (j.contains("textures")) {
            for (const auto& entry : j.at("textures")) {
                pack.textures.push_back(ResourceEntry{
                    entry.at("id").get<std::string>(),
                    entry.at("path").get<std::string>()
                });
            }
        }

        if (j.contains("fonts")) {
            for (const auto& entry : j.at("fonts")) {
                pack.fonts.push_back(ResourceEntry{
                    entry.at("id").get<std::string>(),
                    entry.at("path").get<std::string>()
                });
            }
        }

        if (j.contains("sounds")) {
            for (const auto& entry : j.at("sounds")) {
                pack.sounds.push_back(ResourceEntry{
                    entry.at("id").get<std::string>(),
                    entry.at("path").get<std::string>()
                });
            }
        }

        if (j.contains("musics")) {
            for (const auto& entry : j.at("musics")) {
                pack.musics.push_back(ResourceEntry{
                    entry.at("id").get<std::string>(),
                    entry.at("path").get<std::string>()
                });
            }
        }
    }

} // namespace NovaEngine