#pragma once

#include "../Core/Types.hpp"
#include "../Core/Logger.hpp"
#include "../Backend/BackendManager.hpp"

#include <unordered_map>
#include <string>

namespace NovaEngine {

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        bool registerSound(const ID& id, const std::string& path);
        bool registerMusic(const ID& id, const std::string& path);

        void playSound(const ID& id);
        void playMusic(const ID& id, bool loop = true);
        void stopMusic();
        void setSoundVolume(float volume);
        void setMusicVolume(float volume);

    private:
        std::unordered_map<ID, SoundHandle> m_sounds;
        std::unordered_map<ID, MusicHandle> m_musics;
    };

} // namespace NovaEngine
