-- nova/entity_state.lua
-- Persistance de l'état des entités pré-placées dans les JSON de scène.
-- Disponible sans require : EntityState est un global auto-chargé.
--
-- Convention : les entités devant persister leur état ajoutent dans leur bloc
-- "properties" du JSON de scène (ou directement dans le script via init) :
--   self.stateID = "scene_nom_unique"   -- identifiant de persistance
--
-- Puis dans init() du script de l'entité :
--   EntityState.register(self.entity, self.stateID)
--
-- API :
--   EntityState.register(entity, stateID)   -- enregistre + applique l'état sauvegardé
--   EntityState.disable(entity)             -- désactive + persiste "dead"
--   EntityState.enable(entity)              -- réactive + efface la persistance
--   EntityState.isDead(stateID)             -- vrai si marqué comme mort en persistance
--   EntityState.reset(stateID)              -- efface la persistance (sans toucher l'entité)
--   EntityState.resetAll()                  -- efface tous les états (nouvelle partie)
--
-- Exemple :
--   -- Dans le script d'un ennemi :
--   function M.init(entity, props)
--       self.entity  = entity
--       self.stateID = props.stateID or ("enemy_" .. tostring(entity.id))
--       EntityState.register(self.entity, self.stateID)
--   end
--
--   -- Quand l'ennemi meurt :
--   EntityState.disable(self.entity)
--
--   -- Nouvelle partie :
--   EntityState.resetAll()

local EntityState = {}

-- stateID → entity (pour retrouver l'entité depuis l'ID)
local _byStateID = {}
-- entity.id → stateID (pour retrouver l'ID depuis l'entité)
local _byEntityID = {}

local DEAD_VALUE = "dead"
local KEY_PREFIX = "_es_"

-- Vide le registre en mémoire (appelé à chaque changement de scène).
-- Les nouvelles entités se ré-enregistreront via init().
local function clearRegistry()
    _byStateID  = {}
    _byEntityID = {}
end

EventBus.on("scene_changed", function()
    clearRegistry()
end)

-- Enregistre l'entité et applique immédiatement l'état persisté si présent.
function EntityState.register(entity, stateID)
    if not entity or not stateID then
        Log.warn("[EntityState.register] entity ou stateID manquant")
        return
    end
    _byStateID[stateID]        = entity
    _byEntityID[entity.id]     = stateID

    if Persist.get(KEY_PREFIX .. stateID) == DEAD_VALUE then
        entity:disable()
    end
end

-- Désactive l'entité et marque son état comme "dead" en persistance.
function EntityState.disable(entity)
    if not entity then return end
    local stateID = _byEntityID[entity.id]
    if not stateID then
        Log.warn("[EntityState.disable] entité non enregistrée (id=" .. tostring(entity.id) .. ")")
        entity:disable()
        return
    end
    entity:disable()
    Persist.set(KEY_PREFIX .. stateID, DEAD_VALUE)
end

-- Réactive l'entité et efface sa persistance.
function EntityState.enable(entity)
    if not entity then return end
    local stateID = _byEntityID[entity.id]
    if not stateID then
        Log.warn("[EntityState.enable] entité non enregistrée (id=" .. tostring(entity.id) .. ")")
        entity:enable()
        return
    end
    entity:enable()
    Persist.delete(KEY_PREFIX .. stateID)
end

-- Vrai si le stateID est marqué comme mort en persistance.
function EntityState.isDead(stateID)
    return Persist.get(KEY_PREFIX .. stateID) == DEAD_VALUE
end

-- Efface la persistance d'un stateID sans toucher l'entité en mémoire.
function EntityState.reset(stateID)
    Persist.delete(KEY_PREFIX .. stateID)
end

-- Efface TOUS les états persistés (nouvelle partie).
-- N'affecte pas les entités déjà en mémoire — recharger la scène pour voir l'effet.
function EntityState.resetAll()
    local all  = Persist.getAll()
    local keys = {}
    for k in pairs(all) do
        if k:sub(1, #KEY_PREFIX) == KEY_PREFIX then
            keys[#keys + 1] = k
        end
    end
    if #keys > 0 then Persist.deleteBatch(keys) end
    clearRegistry()
end

return EntityState
