-- nova/cooldown.lua
-- Cooldowns par entité et par action — essentiel pour tout système de combat.
-- Disponible sans require : Cooldown est un global auto-chargé.
--
-- API :
--   Cooldown.use(entityId, action, duration)   -- démarre le cooldown, retourne true si prêt
--   Cooldown.start(entityId, action, duration) -- démarre sans vérifier
--   Cooldown.isReady(entityId, action)         -- vrai si pas en cooldown
--   Cooldown.getRemaining(entityId, action)    -- secondes restantes (0 si prêt)
--   Cooldown.getProgress(entityId, action, duration) -- progression 0..1
--   Cooldown.reset(entityId, action)           -- annule un cooldown spécifique
--   Cooldown.resetAll(entityId)                -- annule tous les cooldowns de l'entité
--   Cooldown.update(dt)                        -- appelé automatiquement par ScriptSystem
--
-- Cycle de vie :
--   À la destruction d'une entité (entity_destroyed), Cooldown.resetAll() est appelé
--   automatiquement — aucune entrée ne subsiste pour des entités détruites.
--
-- Exemple :
--   function update(entity, dt)
--       -- Attaque toutes les 1.5 secondes
--       if Input.isKeyPressed("Space") then
--           if Cooldown.use(entity.id, "attack", 1.5) then
--               performAttack()
--           end
--       end
--
--       -- Soin toutes les 10 secondes avec la touche H
--       if InputEx.isKeyJustPressed("H") then
--           if Cooldown.use(entity.id, "heal", 10.0) then
--               Stats.mod(entity.id, "Health", 30)
--               Log.info("Soin ! (CD: 10s)")
--           else
--               local rem = Cooldown.getRemaining(entity.id, "heal")
--               Log.info("Soin en CD : " .. string.format("%.1f", rem) .. "s")
--           end
--       end
--   end

local Cooldown = {}

-- timers[entityId][action] = secondes restantes
local timers = {}

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

-- Vérifie si prêt ET démarre le cooldown si c'est le cas.
-- Retourne true si l'action peut être exécutée, false sinon.
function Cooldown.use(entityId, action, duration)
    entityId = toId(entityId)
    if not Cooldown.isReady(entityId, action) then return false end
    Cooldown.start(entityId, action, duration)
    return true
end

-- Démarre le cooldown sans vérifier s'il était prêt.
function Cooldown.start(entityId, action, duration)
    entityId = toId(entityId)
    if not timers[entityId] then timers[entityId] = {} end
    timers[entityId][action] = duration
end

function Cooldown.isReady(entityId, action)
    entityId = toId(entityId)
    if not timers[entityId] then return true end
    local rem = timers[entityId][action]
    return not rem or rem <= 0
end

function Cooldown.getRemaining(entityId, action)
    entityId = toId(entityId)
    if not timers[entityId] then return 0 end
    return math.max(0, timers[entityId][action] or 0)
end

-- Fraction 0..1 de progression (0 = vient de commencer, 1 = prêt)
-- Utile pour afficher une barre de progression.
function Cooldown.getProgress(entityId, action, duration)
    entityId = toId(entityId)
    local rem = Cooldown.getRemaining(entityId, action)
    if duration <= 0 then return 1 end
    return 1 - (rem / duration)
end

function Cooldown.reset(entityId, action)
    entityId = toId(entityId)
    if timers[entityId] then timers[entityId][action] = nil end
end

function Cooldown.resetAll(entityId)
    entityId = toId(entityId)
    timers[entityId] = nil
end

function Cooldown.update(dt)
    for entityId, actions in pairs(timers) do
        for action, remaining in pairs(actions) do
            local next = remaining - dt
            if next <= 0 then
                actions[action] = nil
                EventBus.emit("cooldown_ready", { entityId = entityId, action = action })
            else
                actions[action] = next
            end
        end
    end
end

-- Nettoyage automatique à la destruction d'une entité
EventBus.on("entity_destroyed", function(data)
    Cooldown.resetAll(data.entityId)
end)

return Cooldown
