-- nova/pool.lua
-- Pooling d'entités — réutilisation d'objets pré-alloués pour limiter les allocations.
-- Disponible sans require : Pool est un global auto-chargé.
-- Dépend de : Registry (binding C++), World (global auto-chargé).
--
-- entity:enable()  → active  SpriteComponent.visible + ColliderComponent.enabled
-- entity:disable() → désactive SpriteComponent.visible + ColliderComponent.enabled
--
-- API :
--   Pool.create(id, templateFn, size)  -- crée un pool ; templateFn() doit créer et retourner
--                                      --   une entité ; appelée `size` fois ; chaque entité
--                                      --   est désactivée immédiatement via entity:disable()
--   Pool.acquire(id)                   -- retourne une entité disponible (entity:enable() appelé) ;
--                                      --   retourne nil si le pool est épuisé
--   Pool.release(id, entity)           -- rend une entité au pool (entity:disable() appelé)
--   Pool.available(id)                 -- nombre d'entités disponibles dans le pool
--   Pool.size(id)                      -- taille totale du pool (actives + disponibles)
--   Pool.clear(id)                     -- détruit toutes les entités du pool via World.destroy()
--   Pool.clearAll()                    -- détruit tous les pools
--
-- Exemple — pool de balles :
--   Pool.create("bullet", function()
--       local e = World.spawn("BulletType")
--       return e
--   end, 64)
--
--   -- Tir : récupère une balle du pool
--   local bullet = Pool.acquire("bullet")
--   if bullet then
--       World.setPosition(bullet, shooter.x, shooter.y)
--       bullet:getComponent("Velocity").vx = 500
--   end
--
--   -- Impact ou sortie d'écran : remet la balle dans le pool
--   Pool.release("bullet", bullet)
--
--   -- Nettoyage de fin de niveau
--   Pool.clear("bullet")

local Pool = {}

-- { [id] = { available = { entity, ... }, all = { entity, ... } } }
local _pools = {}

-- Crée un pool identifié par `id`.
-- templateFn()  doit créer une entité et la retourner.
-- Les entités sont pré-allouées et immédiatement désactivées.
function Pool.create(id, templateFn, size)
    if type(id) ~= "string" then
        Log.warn("[Pool] create: id doit être une chaîne")
        return
    end
    if type(templateFn) ~= "function" then
        Log.warn("[Pool] create: templateFn doit être une fonction")
        return
    end
    size = size or 0
    if _pools[id] then
        Log.warn("[Pool] create: le pool '" .. id .. "' existe déjà — ignoré")
        return
    end

    local pool = { available = {}, all = {} }

    for i = 1, size do
        local ok, entity = pcall(templateFn)
        if not ok or not entity then
            Log.error("[Pool] create: templateFn a échoué à l'itération " .. i
                      .. " pour le pool '" .. id .. "' : " .. tostring(entity))
        else
            entity:disable()
            table.insert(pool.available, entity)
            table.insert(pool.all, entity)
        end
    end

    _pools[id] = pool
end

-- Retourne une entité disponible du pool et l'active.
-- Retourne nil si le pool est épuisé ou inexistant.
function Pool.acquire(id)
    local pool = _pools[id]
    if not pool then
        Log.warn("[Pool] acquire: pool inconnu '" .. tostring(id) .. "'")
        return nil
    end
    if #pool.available == 0 then
        return nil
    end
    local entity = table.remove(pool.available)
    entity:enable()
    return entity
end

-- Rend une entité au pool et la désactive.
function Pool.release(id, entity)
    local pool = _pools[id]
    if not pool then
        Log.warn("[Pool] release: pool inconnu '" .. tostring(id) .. "'")
        return
    end
    if not entity then
        Log.warn("[Pool] release: entité nil pour le pool '" .. id .. "'")
        return
    end
    entity:disable()
    table.insert(pool.available, entity)
end

-- Retourne le nombre d'entités actuellement disponibles dans le pool.
function Pool.available(id)
    local pool = _pools[id]
    if not pool then return 0 end
    return #pool.available
end

-- Retourne la taille totale du pool (entités actives + disponibles).
function Pool.size(id)
    local pool = _pools[id]
    if not pool then return 0 end
    return #pool.all
end

-- Détruit toutes les entités d'un pool via World.destroy() et supprime le pool.
function Pool.clear(id)
    local pool = _pools[id]
    if not pool then return end
    for _, entity in ipairs(pool.all) do
        local ok, err = pcall(World.destroy, entity.id)
        if not ok then
            Log.error("[Pool] clear: World.destroy a échoué pour le pool '" .. id
                      .. "' : " .. tostring(err))
        end
    end
    _pools[id] = nil
end

-- Détruit tous les pools existants.
function Pool.clearAll()
    for id in pairs(_pools) do
        Pool.clear(id)
    end
end

return Pool
