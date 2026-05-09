-- nova/effect.lua
-- Effets de statut temporaires — poison, ralentissement, bouclier, etc.
-- Inspiré du système d'ActiveEffects de Papyrus / Skyrim.
-- Disponible sans require : Effect est un global auto-chargé.
--
-- API :
--   Effect.apply(entityId, effectId, config)  -- applique un effet
--   Effect.remove(entityId, effectId)         -- retire un effet spécifique
--   Effect.has(entityId, effectId)            -- vrai si l'effet est actif
--   Effect.clear(entityId)                    -- retire tous les effets (appelle onExpire)
--   Effect.getRemaining(entityId, effectId)   -- secondes restantes (ou nil)
--   Effect.update(dt)                         -- appelé automatiquement par ScriptSystem
--
-- Config :
--   duration  (number)    -- durée en secondes (-1 = permanent)
--   stackable (bool)      -- plusieurs instances simultanées ? (défaut: false)
--   onApply   (function)  -- appelé une fois à l'application
--   onTick    (function(entityId, dt)) -- appelé chaque frame
--   onExpire  (function)  -- appelé à l'expiration ou quand Effect.clear() est forcé
--
-- Cycle de vie :
--   À la destruction d'une entité (entity_destroyed), Effect.clear() est appelé
--   automatiquement : tous les effets actifs reçoivent leur onExpire avant suppression.
--
-- EventBus events émis :
--   "effect_applied"  { entityId, effectId }
--   "effect_expired"  { entityId, effectId }
--   "effect_removed"  { entityId, effectId }
--
-- Exemple :
--   -- Poison : 3 dégâts/seconde pendant 5 secondes
--   Effect.apply(entity.id, "poisoned", {
--       duration = 5.0,
--       onApply  = function(eid)
--           Log.info("Empoisonné !")
--       end,
--       onTick   = function(eid, dt)
--           Stats.mod(eid, "Health", -3 * dt)
--       end,
--       onExpire = function(eid)
--           Log.info("Poison dissipé.")
--       end,
--   })
--
--   -- Bouclier permanent
--   Effect.apply(entity.id, "shielded", {
--       duration = -1,   -- permanent
--       onApply  = function(eid) Stats.set(eid, "Armor", 50) end,
--       onExpire = function(eid) Stats.set(eid, "Armor", 0)  end,
--   })

local Effect = {}

-- data[entityId][effectId] = { config, elapsed, instances[] }
local data = {}

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

local function safeCall(fn, ...)
    if not fn then return end
    local ok, err = pcall(fn, ...)
    if not ok then Log.error("[Effect] " .. tostring(err)) end
end

function Effect.apply(entityId, effectId, config)
    entityId = toId(entityId)
    if not data[entityId] then data[entityId] = {} end

    local existing = data[entityId][effectId]
    if existing and not (config.stackable) then
        -- Renouvelle la durée sans ré-appliquer onApply
        existing.elapsed = 0
        return
    end

    local inst = {
        config  = config,
        elapsed = 0,
    }
    if config.stackable then
        -- Stocke plusieurs instances sous forme de liste
        if not data[entityId][effectId] then
            data[entityId][effectId] = { instances = {} }
        end
        table.insert(data[entityId][effectId].instances, inst)
    else
        data[entityId][effectId] = inst
    end

    safeCall(config.onApply, entityId)
    EventBus.emit("effect_applied", { entityId = entityId, effectId = effectId })
end

function Effect.remove(entityId, effectId)
    entityId = toId(entityId)
    if not data[entityId] or not data[entityId][effectId] then return end
    local inst = data[entityId][effectId]
    if inst.instances then
        for _, si in ipairs(inst.instances) do
            safeCall(si.config and si.config.onExpire, entityId)
        end
    else
        safeCall(inst.config and inst.config.onExpire, entityId)
    end
    data[entityId][effectId] = nil
    EventBus.emit("effect_removed", { entityId = entityId, effectId = effectId })
end

function Effect.has(entityId, effectId)
    entityId = toId(entityId)
    return data[entityId] ~= nil and data[entityId][effectId] ~= nil
end

function Effect.clear(entityId)
    entityId = toId(entityId)
    if not data[entityId] then return end
    for effectId, inst in pairs(data[entityId]) do
        if inst.instances then
            for _, si in ipairs(inst.instances) do
                safeCall(si.config and si.config.onExpire, entityId)
            end
        else
            safeCall(inst.config and inst.config.onExpire, entityId)
        end
        EventBus.emit("effect_removed", { entityId = entityId, effectId = effectId })
    end
    data[entityId] = nil
end

function Effect.getRemaining(entityId, effectId)
    entityId = toId(entityId)
    local inst = data[entityId] and data[entityId][effectId]
    if not inst or not inst.config then return nil end
    if inst.config.duration < 0 then return math.huge end
    return math.max(0, inst.config.duration - inst.elapsed)
end

function Effect.update(dt)
    for entityId, effects in pairs(data) do
        for effectId, inst in pairs(effects) do
            -- Effets stackables : inst = { instances = {...} }
            if inst.instances then
                local alive = {}
                for _, si in ipairs(inst.instances) do
                    local cfg = si.config
                    if cfg then
                        safeCall(cfg.onTick, entityId, dt)
                        if cfg.duration >= 0 then
                            si.elapsed = si.elapsed + dt
                            if si.elapsed >= cfg.duration then
                                safeCall(cfg.onExpire, entityId)
                                EventBus.emit("effect_expired", { entityId = entityId, effectId = effectId })
                            else
                                table.insert(alive, si)
                            end
                        else
                            table.insert(alive, si)
                        end
                    end
                end
                if #alive == 0 then
                    effects[effectId] = nil
                else
                    inst.instances = alive
                end
            else
                -- Effet non-stackable : inst = { config, elapsed }
                local cfg = inst.config
                if cfg then
                    safeCall(cfg.onTick, entityId, dt)
                    if cfg.duration >= 0 then
                        inst.elapsed = inst.elapsed + dt
                        if inst.elapsed >= cfg.duration then
                            safeCall(cfg.onExpire, entityId)
                            effects[effectId] = nil
                            EventBus.emit("effect_expired", { entityId = entityId, effectId = effectId })
                        end
                    end
                end
            end
        end
    end
end

-- Nettoyage automatique à la destruction d'une entité (appelle onExpire sur chaque effet)
EventBus.on("entity_destroyed", function(data)
    Effect.clear(data.entityId)
end)

return Effect
