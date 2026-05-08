-- nova/projectile.lua
-- Système de projectiles via Pool — tirs, flèches, sorts, balles.
-- Disponible sans require : Projectile est un global auto-chargé.
-- Dépend de : Pool, World, EventBus (tous globals auto-chargés).
--
-- API :
--   Projectile.init(opts)              -- crée un pool de projectiles
--   Projectile.fire(x, y, opts)        -- tire un projectile, retourne l'entité (ou nil)
--   Projectile.release(entity)         -- libère manuellement (avant expiration)
--   Projectile.clearPool(poolID)       -- détruit un pool et ses projectiles actifs
--   Projectile.clearAll()              -- détruit tous les pools et handlers de collision
--   Projectile.activeCount()           -- nombre de projectiles en vol
--   Projectile._update(dt)             -- appelé par ScriptSystem chaque frame
--
-- Cycle de vie :
--   Au changement de scène (scene_changed), Projectile.clearAll() est appelé
--   automatiquement — les handlers de collision et les références d'entités sont libérés.
--
-- opts pour init() :
--   poolID      string   -- identifiant du pool (défaut "projectile")
--   count       int      -- taille du pool (défaut 32)
--   templateFn  function -- crée et retourne une entité (requis)
--
-- opts pour fire() :
--   poolID      string   -- pool à utiliser (défaut "projectile")
--   vx, vy      number   -- vitesse en px/s (prioritaire sur angle+speed)
--   angle       number   -- direction en degrés (0=droite, 270=haut)
--   speed       number   -- magnitude en px/s (utilisé avec angle)
--   lifetime    number   -- durée de vie en secondes (défaut 3.0)
--   onHit       function -- onHit(projEntity, hitEntityId) appelé lors d'une collision
--   onExpire    function -- onExpire(projEntity) appelé à l'expiration
--
-- Exemple — flèches :
--   Projectile.init({
--       poolID = "arrow", count = 16,
--       templateFn = function()
--           local e = World.spawn("ArrowEntity")
--           return e
--       end
--   })
--
--   -- Tir vers la droite :
--   local arrow = Projectile.fire(player.x, player.y, {
--       poolID  = "arrow",
--       angle   = 0, speed = 400,
--       lifetime = 2.0,
--       onHit = function(proj, hitId)
--           Sound.play("data/sounds/arrow_hit.ogg")
--           Projectile.release(proj)
--       end,
--       onExpire = function(proj)
--           Particles.emit(World.getPosition(proj).x, World.getPosition(proj).y, { count = 4 })
--       end
--   })

local Projectile = {}

-- { entity → proj_record } pour la résolution rapide dans release()
local _byEntity = {}
-- liste des projectiles actifs
local _active   = {}
-- pools déclarés { poolID → true }
local _pools    = {}

local function releaseProj(proj)
    if not proj or not proj.alive then return end
    proj.alive = false

    if proj._colEvent and proj._colHandler then
        EventBus.off(proj._colEvent, proj._colHandler)
        proj._colEvent   = nil
        proj._colHandler = nil
    end

    _byEntity[proj.entity] = nil
    Pool.release(proj.poolID, proj.entity)
end

-- Crée un pool de projectiles pré-alloués.
function Projectile.init(opts)
    opts = opts or {}
    local poolID = opts.poolID or "projectile"
    local count  = opts.count  or 32
    local fn     = opts.templateFn
    if not fn then
        Log.warn("[Projectile.init] templateFn requis")
        return
    end
    if _pools[poolID] then
        Log.warn("[Projectile.init] pool '" .. poolID .. "' déjà initialisé")
        return
    end
    Pool.create(poolID, fn, count)
    _pools[poolID] = true
end

-- Tire un projectile depuis (x, y).
-- Retourne l'entité acquise ou nil si le pool est épuisé.
function Projectile.fire(x, y, opts)
    opts = opts or {}
    local poolID = opts.poolID or "projectile"

    local entity = Pool.acquire(poolID)
    if not entity then
        Log.warn("[Projectile.fire] pool '" .. poolID .. "' épuisé")
        return nil
    end

    World.setPosition(entity, x, y)

    local vx, vy = opts.vx or 0, opts.vy or 0
    if opts.angle ~= nil and opts.speed then
        local rad = math.rad(opts.angle)
        vx = math.cos(rad) * opts.speed
        vy = math.sin(rad) * opts.speed
    end

    local proj = {
        entity   = entity,
        vx       = vx,
        vy       = vy,
        life     = opts.lifetime or 3.0,
        poolID   = poolID,
        onHit    = opts.onHit,
        onExpire = opts.onExpire,
        alive    = true,
        -- rempli par le handler de collision :
        _hitEntityId = nil,
        _colEvent    = nil,
        _colHandler  = nil,
    }

    -- Souscrit aux événements de collision si un callback est fourni
    if opts.onHit then
        local eid    = tostring(entity.id)
        local ev     = "collision_enter_" .. eid
        local handler = function(data)
            -- Marque pour traitement dans _update (évite mutation pendant emit)
            if proj.alive and proj._hitEntityId == nil then
                proj._hitEntityId = data.entityId
                proj.alive = false
            end
        end
        EventBus.on(ev, handler)
        proj._colEvent   = ev
        proj._colHandler = handler
    end

    _byEntity[entity] = proj
    _active[#_active + 1] = proj
    return entity
end

-- Libère manuellement un projectile (depuis onHit ou une condition externe).
function Projectile.release(entity)
    local proj = entity and _byEntity[entity]
    if proj then
        releaseProj(proj)
    end
end

-- Détruit un pool et libère tous ses projectiles actifs.
function Projectile.clearPool(poolID)
    for _, proj in ipairs(_active) do
        if proj.poolID == poolID and proj.alive then
            releaseProj(proj)
        end
    end
    Pool.clear(poolID)
    _pools[poolID] = nil
end

-- Détruit tous les pools.
function Projectile.clearAll()
    for _, proj in ipairs(_active) do
        releaseProj(proj)
    end
    _active   = {}
    _byEntity = {}
    Pool.clearAll()
    _pools = {}
end

-- Nombre de projectiles actuellement en vol.
function Projectile.activeCount()
    local n = 0
    for _, proj in ipairs(_active) do
        if proj.alive then n = n + 1 end
    end
    return n
end

-- Appelé par ScriptSystem chaque frame.
function Projectile._update(dt)
    local surviving = {}

    for _, proj in ipairs(_active) do
        if proj.alive then
            -- Mise à jour position
            local tr = proj.entity:getTransform()
            if tr then
                tr.position.x = tr.position.x + proj.vx * dt
                tr.position.y = tr.position.y + proj.vy * dt
            end

            proj.life = proj.life - dt

            if proj.life <= 0 then
                -- Expiration naturelle
                proj.alive = false
                if proj.onExpire then
                    pcall(proj.onExpire, proj.entity)
                end
                releaseProj(proj)
            else
                surviving[#surviving + 1] = proj
            end
        elseif proj._hitEntityId ~= nil then
            -- Collision détectée par handler, traitée ici hors du cycle EventBus.emit
            local hitId = proj._hitEntityId
            if proj.onHit then
                pcall(proj.onHit, proj.entity, hitId)
            end
            releaseProj(proj)
        end
        -- proj.alive == false sans _hitEntityId → déjà libéré via Projectile.release()
    end

    _active = surviving
end

-- Nettoyage automatique au changement de scène : libère tous les projectiles
-- en vol et leurs handlers de collision pour éviter les références d'entités périmées.
EventBus.on("scene_changed", function()
    Projectile.clearAll()
end)

return Projectile
