-- nova/stats.lua
-- Valeurs d'acteur par entité — équivalent de GetActorValue / SetActorValue / ModActorValue
-- dans Papyrus. Chaque entité peut avoir des stats nommées (Health, Stamina, Mana, Level...)
-- Disponible sans require : Stats est un global auto-chargé.
--
-- API :
--   Stats.get(entityId, stat, default)  -- lit une valeur (default si absente)
--   Stats.set(entityId, stat, value)    -- définit une valeur
--   Stats.mod(entityId, stat, delta)    -- modifie par delta, retourne la nouvelle valeur
--   Stats.has(entityId, stat)           -- vrai si la stat existe
--   Stats.remove(entityId, stat)        -- supprime une stat
--   Stats.getAll(entityId)              -- retourne toutes les stats de l'entité
--   Stats.clear(entityId)               -- supprime toutes les stats de l'entité
--
-- EventBus events émis :
--   "stat_changed"  { entityId, stat, value, previous }
--   "stat_zeroed"   { entityId, stat }  — quand value atteint 0 ou moins
--
-- Exemple :
--   -- Initialiser un ennemi
--   Stats.set(entity.id, "Health",  50)
--   Stats.set(entity.id, "Stamina", 30)
--   Stats.set(entity.id, "Level",   3)
--
--   -- Infliger des dégâts
--   local newHp = Stats.mod(entity.id, "Health", -15)
--   if newHp <= 0 then entity:sendMessage("die") end
--
--   -- Dans un script ennemi avec named handlers :
--   function OnMessage(msg, data)
--       if msg == "takeDamage" then
--           local hp = Stats.mod(entity.id, "Health", -data.amount)
--           if hp <= 0 then OnDeath() end
--       end
--   end

local Stats = {}
local data = {}   -- data[entityId][statName] = value

function Stats.get(entityId, stat, default)
    local entityStats = data[entityId]
    if not entityStats then return default end
    local v = entityStats[stat]
    return (v ~= nil) and v or default
end

function Stats.set(entityId, stat, value)
    if not data[entityId] then data[entityId] = {} end
    local previous = data[entityId][stat]
    data[entityId][stat] = value
    EventBus.emit("stat_changed", {
        entityId = entityId,
        stat     = stat,
        value    = value,
        previous = previous,
    })
    if type(value) == "number" and value <= 0 then
        EventBus.emit("stat_zeroed", { entityId = entityId, stat = stat })
    end
end

-- Modifie par delta et retourne la nouvelle valeur
function Stats.mod(entityId, stat, delta)
    local current = Stats.get(entityId, stat, 0)
    local next    = current + delta
    Stats.set(entityId, stat, next)
    return next
end

-- Clamp : modifie avec une borne min/max
function Stats.modClamped(entityId, stat, delta, min, max)
    local current = Stats.get(entityId, stat, 0)
    local next    = math.max(min, math.min(max, current + delta))
    Stats.set(entityId, stat, next)
    return next
end

function Stats.has(entityId, stat)
    return data[entityId] ~= nil and data[entityId][stat] ~= nil
end

function Stats.remove(entityId, stat)
    if data[entityId] then
        data[entityId][stat] = nil
    end
end

function Stats.getAll(entityId)
    if not data[entityId] then return {} end
    local copy = {}
    for k, v in pairs(data[entityId]) do copy[k] = v end
    return copy
end

function Stats.clear(entityId)
    data[entityId] = nil
end

-- Initialise un ensemble de stats depuis une table
-- Stats.init(entity.id, { Health=100, Stamina=50, Level=1 })
function Stats.init(entityId, defaults)
    for stat, value in pairs(defaults) do
        if not Stats.has(entityId, stat) then
            Stats.set(entityId, stat, value)
        end
    end
end

return Stats
