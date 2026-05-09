-- nova/stats.lua
-- Valeurs d'acteur par entité — équivalent de GetActorValue / SetActorValue / ModActorValue
-- dans Papyrus. Chaque entité peut avoir des stats nommées (Health, Stamina, Mana, Level...)
-- Disponible sans require : Stats est un global auto-chargé.
--
-- API :
--   Stats.get(entityId, stat, default)       -- lit une valeur (default si absente)
--   Stats.set(entityId, stat, value)         -- définit une valeur
--   Stats.mod(entityId, stat, delta)         -- modifie par delta, retourne la nouvelle valeur
--   Stats.modClamped(entityId, stat, d, min, max) -- modifie avec borne
--   Stats.has(entityId, stat)                -- vrai si la stat existe
--   Stats.remove(entityId, stat)             -- supprime une stat
--   Stats.getAll(entityId)                   -- retourne toutes les stats de l'entité
--   Stats.clear(entityId)                    -- supprime toutes les stats de l'entité
--   Stats.init(entityId, defaults)           -- initialise sans écraser, sans émettre stat_changed
--   Stats.bind(entityId, stat, fn)           -- fn(value, previous, entityId, stat) à chaque set
--   Stats.unbind(entityId, stat, fn)         -- retire un binding spécifique
--   Stats.unbindAll(entityId)               -- retire tous les bindings de l'entité
--
-- Cycle de vie :
--   Les données persistent tant que l'entité est vivante.
--   À la destruction (entity_destroyed), Stats.clear() est appelé automatiquement
--   pour toute entité, qu'elle ait un ScriptComponent ou non.
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
local data      = {}   -- data[entityId][statName] = value
local _bindings = {}   -- _bindings[entityId][statName] = { fn, ... }
local _tb       = (type(debug) == "table" and debug.traceback) or tostring

-- Internal: set a value without emitting any events (used by Stats.init)
local function _rawSet(entityId, stat, value)
    if not data[entityId] then data[entityId] = {} end
    data[entityId][stat] = value
end

-- Accepte une entité (userdata) ou un entityId (number)
local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

function Stats.get(entityId, stat, default)
    entityId = toId(entityId)
    local entityStats = data[entityId]
    if not entityStats then return default end
    local v = entityStats[stat]
    return (v ~= nil) and v or default
end

function Stats.set(entityId, stat, value)
    entityId = toId(entityId)
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
    -- Fire reactive bindings
    local eb = _bindings[entityId]
    if eb then
        local fns = eb[stat]
        if fns then
            for _, fn in ipairs(fns) do
                local ok, err = xpcall(fn, _tb, value, previous, entityId, stat)
                if not ok then Log.error("[Stats] bind: " .. tostring(err)) end
            end
        end
    end
end

-- Modifie par delta et retourne la nouvelle valeur
function Stats.mod(entityId, stat, delta)
    entityId = toId(entityId)
    local current = Stats.get(entityId, stat, 0)
    local next    = current + delta
    Stats.set(entityId, stat, next)
    return next
end

-- Clamp : modifie avec une borne min/max
function Stats.modClamped(entityId, stat, delta, min, max)
    entityId = toId(entityId)
    local current = Stats.get(entityId, stat, 0)
    local next    = math.max(min, math.min(max, current + delta))
    Stats.set(entityId, stat, next)
    return next
end

function Stats.has(entityId, stat)
    entityId = toId(entityId)
    return data[entityId] ~= nil and data[entityId][stat] ~= nil
end

function Stats.remove(entityId, stat)
    entityId = toId(entityId)
    if data[entityId] then
        data[entityId][stat] = nil
        if not next(data[entityId]) then data[entityId] = nil end
    end
end

function Stats.getAll(entityId)
    entityId = toId(entityId)
    if not data[entityId] then return {} end
    local copy = {}
    for k, v in pairs(data[entityId]) do copy[k] = v end
    return copy
end

function Stats.clear(entityId)
    entityId = toId(entityId)
    data[entityId] = nil
end

-- Initialise un ensemble de stats depuis une table, sans émettre stat_changed.
-- Stats.init(entity.id, { Health=100, Stamina=50, Level=1 })
function Stats.init(entityId, defaults)
    entityId = toId(entityId)
    for stat, value in pairs(defaults) do
        if not Stats.has(entityId, stat) then
            _rawSet(entityId, stat, value)
        end
    end
end

-- ─── Reactive bindings ────────────────────────────────────────────────────────

-- Appelle fn(value, previous, entityId, stat) chaque fois que stat change pour entityId.
function Stats.bind(entityId, stat, fn)
    entityId = toId(entityId)
    if not _bindings[entityId] then _bindings[entityId] = {} end
    if not _bindings[entityId][stat] then _bindings[entityId][stat] = {} end
    table.insert(_bindings[entityId][stat], fn)
end

function Stats.unbind(entityId, stat, fn)
    entityId = toId(entityId)
    local eb = _bindings[entityId]
    if not eb or not eb[stat] then return end
    local list = eb[stat]
    for i = #list, 1, -1 do
        if list[i] == fn then table.remove(list, i) end
    end
    if #list == 0 then eb[stat] = nil end
    if not next(eb) then _bindings[entityId] = nil end
end

function Stats.unbindAll(entityId)
    entityId = toId(entityId)
    _bindings[entityId] = nil
end

-- ─── Nettoyage automatique ────────────────────────────────────────────────────

EventBus.on("entity_destroyed", function(data)
    Stats.clear(data.entityId)
    Stats.unbindAll(data.entityId)
end)

return Stats
