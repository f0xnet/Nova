#pragma once

#include "../Core/Types.hpp"
#include "../Core/Logger.hpp"
#include "ResourceTypes.hpp"

#include <memory>
#include <string>

namespace NovaEngine {

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        bool loadFromJSON(const std::string& path);

        bool loadTexture(const ID& id, const std::string& path);
        bool loadFont(const ID& id, const std::string& path);
        bool loadSoundBuffer(const ID& id, const std::string& path);
        bool loadMusic(const ID& id, const std::string& path);

        std::string getMusicPath(const ID& id) const;

        void clear();

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace NovaEngine
