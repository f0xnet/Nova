#include "NovaEngine/Audio/AudioManager.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

    AudioManager::AudioManager() {
        LOG_INFO("AudioManager initialized");
    }

    AudioManager::~AudioManager() {
        AUDIO().stopMusic();
        LOG_INFO("AudioManager destroyed");
    }

    bool AudioManager::registerSound(const ID& id, const std::string& path) {
        SoundHandle handle = RESOURCES().loadSound(path);
        if (handle == INVALID_HANDLE) {
            LOG_ERROR("AudioManager: failed to load sound '{}' from: {}", id, path);
            return false;
        }
        m_sounds[id] = handle;
        LOG_DEBUG("AudioManager: sound '{}' registered", id);
        return true;
    }

    bool AudioManager::registerMusic(const ID& id, const std::string& path) {
        MusicHandle handle = RESOURCES().loadMusic(path);
        if (handle == INVALID_HANDLE) {
            LOG_ERROR("AudioManager: failed to register music '{}' from: {}", id, path);
            return false;
        }
        m_musics[id] = handle;
        LOG_DEBUG("AudioManager: music '{}' registered", id);
        return true;
    }

    void AudioManager::playSound(const ID& id) {
        auto it = m_sounds.find(id);
        if (it == m_sounds.end()) {
            LOG_ERROR("AudioManager: sound '{}' not registered", id);
            return;
        }
        AUDIO().playSound(it->second);
        LOG_DEBUG("AudioManager: playing sound '{}'", id);
    }

    void AudioManager::playMusic(const ID& id, bool loop) {
        auto it = m_musics.find(id);
        if (it == m_musics.end()) {
            LOG_ERROR("AudioManager: music '{}' not registered", id);
            return;
        }
        AUDIO().playMusic(it->second, loop);
        LOG_INFO("AudioManager: playing music '{}' (loop: {})", id, loop);
    }

    void AudioManager::stopMusic() {
        AUDIO().stopMusic();
        LOG_DEBUG("AudioManager: music stopped");
    }

    void AudioManager::setSoundVolume(float volume) {
        AUDIO().setSoundVolume(volume);
        LOG_DEBUG("AudioManager: sound volume set to {}", volume);
    }

    void AudioManager::setMusicVolume(float volume) {
        AUDIO().setMusicVolume(volume);
        LOG_DEBUG("AudioManager: music volume set to {}", volume);
    }

} // namespace NovaEngine
