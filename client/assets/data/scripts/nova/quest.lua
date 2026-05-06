-- nova/quest.lua
-- Système de quêtes — étapes, objectifs, conditions, récompenses
-- Disponible sans require : Quest est un global auto-chargé.
--
-- API :
--   Quest.register(id, def)         -- enregistre un modèle de quête
--   Quest.start(id)                 -- démarre une quête
--   Quest.advance(id)               -- passe à l'étape suivante
--   Quest.complete(id)              -- complète la quête (force)
--   Quest.fail(id)                  -- échoue la quête
--   Quest.isActive(id)              -- vrai si en cours
--   Quest.isCompleted(id)           -- vrai si terminée avec succès
--   Quest.getStage(id)              -- retourne l'étape courante {id, description}
--   Quest.getStageIndex(id)         -- retourne l'index de l'étape courante
--   Quest.getAll()                  -- liste des quêtes actives
--
-- EventBus events émis :
--   "quest_started"   { questId, stage }
--   "quest_advanced"  { questId, stageIndex, stage }
--   "quest_completed" { questId }
--   "quest_failed"    { questId }
--
-- Exemple :
--   Quest.register("find_key", {
--       name = "Trouver la clé",
--       stages = {
--           { id = "search_room",  description = "Chercher dans la chambre" },
--           { id = "take_key",     description = "Prendre la clé" },
--           { id = "open_door",    description = "Ouvrir la porte" },
--       },
--       onStart    = function(id) Log.info("Quête démarrée : " .. id) end,
--       onComplete = function(id) EventBus.emit("door_unlocked") end,
--   })
--
--   Quest.start("find_key")
--
--   EventBus.on("key_picked_up", function()
--       if Quest.isActive("find_key") then Quest.advance("find_key") end
--   end)

local Quest = {}
local definitions = {}
local active      = {}
local completed   = {}

function Quest.register(id, def)
    definitions[id] = def
end

function Quest.start(id)
    if active[id] or completed[id] then return false end
    local def = definitions[id]
    if not def then
        Log.warn("Quest.start: quête inconnue '" .. id .. "'")
        return false
    end
    active[id] = { stageIndex = 1 }
    if def.onStart then
        local ok, err = pcall(def.onStart, id)
        if not ok then Log.error("Quest.start callback: " .. tostring(err)) end
    end
    EventBus.emit("quest_started", {
        questId = id,
        stage   = def.stages and def.stages[1],
    })
    return true
end

function Quest.advance(id)
    local inst = active[id]
    if not inst then return false end
    local def = definitions[id]
    inst.stageIndex = inst.stageIndex + 1
    local stage = def.stages and def.stages[inst.stageIndex]
    if not stage then
        return Quest.complete(id)
    end
    EventBus.emit("quest_advanced", {
        questId    = id,
        stageIndex = inst.stageIndex,
        stage      = stage,
    })
    return true
end

function Quest.complete(id)
    if not active[id] then return false end
    local def = definitions[id]
    active[id]    = nil
    completed[id] = true
    if def and def.onComplete then
        local ok, err = pcall(def.onComplete, id)
        if not ok then Log.error("Quest.complete callback: " .. tostring(err)) end
    end
    EventBus.emit("quest_completed", { questId = id })
    return true
end

function Quest.fail(id)
    if not active[id] then return false end
    active[id] = nil
    EventBus.emit("quest_failed", { questId = id })
    return true
end

function Quest.isActive(id)    return active[id] ~= nil    end
function Quest.isCompleted(id) return completed[id] == true end

function Quest.getStage(id)
    local inst = active[id]
    if not inst then return nil end
    local def = definitions[id]
    return def and def.stages and def.stages[inst.stageIndex]
end

function Quest.getStageIndex(id)
    return active[id] and active[id].stageIndex
end

function Quest.getAll()
    local list = {}
    for id in pairs(active) do table.insert(list, id) end
    return list
end

-- Retourne le nom affiché pour l'UI
function Quest.getName(id)
    local def = definitions[id]
    return def and def.name or id
end

return Quest
