-- nova/game.lua
-- Fonctions globales du jeu — inspiré de Game.* dans Papyrus
-- Disponible sans require : Game est un global auto-chargé.
--
-- API :
--   Game.getPlayer()              -- retourne l'entité joueur (ou nil)
--   Game.getTime()                -- temps écoulé depuis le début (secondes)
--   Game.getDeltaTime()           -- dernier dt (approximatif, mis à jour à chaque frame)
--   Game.notification(text)       -- émet "ui_notification" {text} pour l'UI
--   Game.setTimescale(scale)      -- multiplie tous les dt (1.0 = normal, 0 = pause)
--   Game.getTimescale()           -- valeur courante du timescale
--   Game.isPaused()               -- vrai si timescale == 0
--   Game.pause() / Game.resume()  -- raccourcis pause/reprend
--
-- Exemple :
--   local player = Game.getPlayer()
--   if player then
--       Log.info("Joueur à " .. player:getPosition().x .. ", " .. player:getPosition().y)
--   end
--
--   Game.notification("Quête complétée !")
--   Game.pause()
--   Timer.after(2.0, function() Game.resume() end)

local Game = {}

local _time      = 0.0
local _dt        = 0.0
local _timescale = 1.0

-- Appelé automatiquement par ScriptSystem chaque frame
function Game._update(dt)
    _dt    = dt
    _time  = _time + dt * _timescale
end

-- Retourne l'entité avec le tag "player" (utilise World)
function Game.getPlayer()
    return World.findByTag("player")
end

-- Temps de jeu écoulé en secondes (affecté par timescale)
function Game.getTime()
    return _time
end

-- Dernier deltaTime reçu (avant application du timescale)
function Game.getDeltaTime()
    return _dt
end

-- Affiche un message dans l'UI via l'EventBus
-- Le jeu doit s'abonner à "ui_notification" pour l'afficher
function Game.notification(text)
    EventBus.emit("ui_notification", { text = tostring(text) })
end

-- Modifie la vitesse du jeu
-- 0.0 = pause totale, 0.5 = ralenti, 1.0 = normal, 2.0 = accéléré
function Game.setTimescale(scale)
    _timescale = math.max(scale, 0.0)
    EventBus.emit("timescale_changed", { timescale = _timescale })
end

function Game.getTimescale()
    return _timescale
end

function Game.isPaused()
    return _timescale == 0.0
end

function Game.pause()
    Game.setTimescale(0.0)
end

function Game.resume()
    Game.setTimescale(1.0)
end

return Game
