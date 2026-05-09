-- nova/trigger.lua
-- Zones de déclenchement (trigger boxes) en espace monde.
-- Disponible sans require : Trigger est un global auto-chargé.
--
-- API :
--   Trigger.create(id, rect, opts)   -- crée une zone
--   Trigger.remove(id)               -- supprime une zone
--   Trigger.enable(id)               -- réactive une zone désactivée
--   Trigger.disable(id)              -- désactive sans supprimer
--   Trigger.clear()                  -- supprime toutes les zones
--   Trigger._update(dt)              -- appelé par ScriptSystem chaque frame
--
-- rect : { x, y, w, h }  (coin haut-gauche + dimensions en pixels monde)
--
-- opts :
--   onEnter   = function(entityId)       -- appelé quand une entité entre
--   onExit    = function(entityId)       -- appelé quand une entité sort
--   onStay    = function(entityId, dt)   -- appelé chaque frame où l'entité est dans la zone
--   filter    = function(entityId) → bool  -- filtre les entités (défaut : toutes)
--   once      = bool                     -- se désactive après le premier déclenchement
--   tags      = { "player", ... }        -- filtre par tag (AND : tous les tags requis)
--
-- EventBus events émis :
--   "trigger_enter" { id, entityId }
--   "trigger_exit"  { id, entityId }
--
-- Exemple :
--   Trigger.create("lava_zone", { x = 200, y = 400, w = 100, h = 50 }, {
--       tags    = { "player" },
--       onEnter = function(eid) Stats.modStat(eid, "health", -10) end,
--       onStay  = function(eid, dt) Stats.modStat(eid, "health", -5 * dt) end,
--   })
--
--   Trigger.create("boss_trigger", { x = 512, y = 256, w = 64, h = 64 }, {
--       once    = true,
--       filter  = function(eid) return World.findByTag("player").id == eid end,
--       onEnter = function() EventBus.emit("boss_fight_start", {}) end,
--   })

local Trigger = {}

local _zones = {}  -- id → zone

local function _tagFilter(tags)
    -- Retourne une fonction-filtre acceptant les entités dont le tag est dans la liste
    local set = {}
    for _, t in ipairs(tags) do set[t] = true end
    return function(eid)
        local ent = Registry:getEntity(eid)
        if not ent then return false end
        local tc = ent:getTag()
        return tc ~= nil and set[tc.tag] == true
    end
end

local function _rectContains(rect, x, y)
    return x >= rect.x and x <= rect.x + rect.w
       and y >= rect.y and y <= rect.y + rect.h
end

function Trigger.create(id, rect, opts)
    opts = opts or {}
    local filter = opts.filter
    if opts.tags and not filter then
        filter = _tagFilter(opts.tags)
    end
    _zones[id] = {
        rect    = rect,
        onEnter = opts.onEnter,
        onExit  = opts.onExit,
        onStay  = opts.onStay,
        filter  = filter,
        once    = opts.once or false,
        active  = true,
        inside  = {},  -- entityId (number) → true
    }
end

function Trigger.remove(id)
    _zones[id] = nil
end

function Trigger.enable(id)
    if _zones[id] then _zones[id].active = true end
end

function Trigger.disable(id)
    if _zones[id] then _zones[id].active = false end
end

function Trigger.clear()
    _zones = {}
end

function Trigger._update(dt)
    local entities = Registry:getAllEntities()
    for zoneId, zone in pairs(_zones) do
        if zone.active then
            local current  = {}

            for _, ent in ipairs(entities) do
                local eid = ent.id

                -- Filtre
                if not zone.filter or zone.filter(eid) then
                    local pos = ent:getPosition()
                    if pos and _rectContains(zone.rect, pos.x, pos.y) then
                        current[eid] = true

                        if not zone.inside[eid] then
                            -- Entrée dans la zone
                            zone.inside[eid] = true
                            EventBus.emit("trigger_enter", { id = zoneId, entityId = eid })
                            if zone.onEnter then zone.onEnter(eid) end

                            if zone.once then
                                zone.active = false
                                break
                            end
                        elseif zone.onStay then
                            zone.onStay(eid, dt)
                        end
                    end
                end
            end

            -- Détecte les sorties
            for eid in pairs(zone.inside) do
                if not current[eid] then
                    zone.inside[eid] = nil
                    EventBus.emit("trigger_exit", { id = zoneId, entityId = eid })
                    if zone.onExit then zone.onExit(eid) end
                end
            end
        end
    end
end

return Trigger
