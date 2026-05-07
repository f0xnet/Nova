-- nova/sound.lua
-- API haut niveau pour le son — jouer par chemin de fichier sans gérer les handles.
-- Utilise Resources (chargement) + Audio (playback) du moteur C++.
-- Disponible sans require : Sound est un global auto-chargé.
--
-- API :
--   Sound.play(path, vol, pitch, loop)  -- joue un son (handle mis en cache)
--   Sound.stop(path)                    -- arrête toutes les instances de ce son
--   Sound.stopAll()                     -- arrête tous les sons SFX
--   Sound.playMusic(path, loop)         -- joue une musique (une seule à la fois)
--   Sound.stopMusic()                   -- arrête la musique
--   Sound.pauseMusic()                  -- met en pause la musique
--   Sound.resumeMusic()                 -- reprend la musique
--   Sound.setVolume(v)                  -- volume global SFX (0.0–1.0)
--   Sound.setMusicVolume(v)             -- volume musique (0.0–1.0)
--   Sound.preload(path)                 -- précharge un son sans le jouer
--   Sound.preloadMusic(path)            -- précharge une musique sans la jouer
--
-- Exemple :
--   Sound.play("data/sounds/sword.ogg")
--   Sound.play("data/sounds/hit.ogg", 0.8)
--   Sound.playMusic("data/music/battle.ogg", true)
--
--   EventBus.on("explosion", function(data)
--       Sound.play("data/sounds/boom.ogg", 1.0, 1.0 + math.random() * 0.2)
--   end)

local Sound = {}

local _soundCache = {}   -- path → SoundHandle
local _musicCache = {}   -- path → MusicHandle

local function loadSound(path)
    if not _soundCache[path] then
        local h = Resources.loadSound(path)
        if not h then
            Log.warn("[Sound] Impossible de charger : " .. path)
            return nil
        end
        _soundCache[path] = h
    end
    return _soundCache[path]
end

local function loadMusic(path)
    if not _musicCache[path] then
        local h = Resources.loadMusic(path)
        if not h then
            Log.warn("[Sound] Impossible de charger la musique : " .. path)
            return nil
        end
        _musicCache[path] = h
    end
    return _musicCache[path]
end

-- vol: 0.0–1.0 (défaut 1.0), pitch: 1.0 = normal, loop: false par défaut
function Sound.play(path, vol, pitch, loop)
    local h = loadSound(path)
    if not h then return end
    Audio.playSound(h, vol or 1.0, pitch or 1.0, loop == true)
end

function Sound.stop(path)
    local h = _soundCache[path]
    if h then Audio.stopSound(h) end
end

function Sound.stopAll()
    Audio.stopAll()
end

-- loop = true par défaut pour la musique de fond
function Sound.playMusic(path, loop)
    local h = loadMusic(path)
    if not h then return end
    Audio.playMusic(h, loop ~= false)
end

function Sound.stopMusic()    Audio.stopMusic()    end
function Sound.pauseMusic()   Audio.pauseMusic()   end
function Sound.resumeMusic()  Audio.resumeMusic()  end

function Sound.setVolume(v)        Audio.setVolume(v)       end
function Sound.setMusicVolume(v)   Audio.setMusicVolume(v)  end

-- Préchargement explicite pour éviter la latence au premier appel
function Sound.preload(path)       loadSound(path)  end
function Sound.preloadMusic(path)  loadMusic(path)  end

return Sound
