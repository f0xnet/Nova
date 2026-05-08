-- nova/particles.lua
-- Système de particules 2D — rendu via DebugDraw (carrés colorés).
-- Disponible sans require : Particles est un global auto-chargé.
--
-- API :
--   Particles.emit(x, y, opts)         -- burst one-shot
--   Particles.emitter(x, y, opts)      -- émetteur continu, retourne un handle
--   Particles.move(handle, x, y)       -- déplace un émetteur continu
--   Particles.stop(handle)             -- arrête un émetteur continu
--   Particles.clear()                  -- supprime toutes les particules et émetteurs
--   Particles.count()                  -- nombre de particules actives
--   Particles._update(dt)              -- appelé par ScriptSystem chaque frame
--
-- Cycle de vie :
--   Au changement de scène (scene_changed), Particles.clear() est appelé automatiquement
--   — les particules d'une scène ne se rendent pas par-dessus la suivante.
--
-- opts (tous optionnels) :
--   count    int     -- particules par burst (défaut 12)
--   rate     number  -- secondes entre spawns pour emitter (défaut 0.04)
--   lifetime number  -- durée de vie en secondes (défaut 1.0)
--   speed    number  -- vitesse max px/s (défaut 80)
--   spread   number  -- demi-angle de dispersion en degrés (défaut 180 = hémisphère)
--   angle    number  -- direction centrale en degrés (0=droite, 270=haut) (défaut 270)
--   gravity  number  -- accélération verticale px/s² (défaut 0)
--   size     number  -- taille initiale en px (défaut 5)
--   sizeEnd  number  -- taille finale en px (défaut 0)
--   color    Color   -- couleur de départ (défaut blanc)
--   colorEnd Color   -- couleur d'arrivée (défaut même couleur, alpha→0)
--
-- Exemples :
--   -- Explosion
--   Particles.emit(x, y, {
--       count = 30, speed = 200, spread = 180, lifetime = 0.8,
--       color = Color.new(255, 120, 0), colorEnd = Color.new(80, 0, 0, 0),
--       size = 6, sizeEnd = 1, gravity = 60
--   })
--
--   -- Traînée de magie (émetteur continu sur une entité)
--   local trail = Particles.emitter(x, y, {
--       rate = 0.03, count = 1, speed = 20, spread = 360,
--       lifetime = 0.5, size = 4, sizeEnd = 0,
--       color = Color.new(80, 120, 255)
--   })
--   -- Chaque frame : Particles.move(trail, entity:getPosition().x, entity:getPosition().y)
--   -- Pour arrêter : Particles.stop(trail)

local Particles = {}

local _pool     = {}
local _emitters = {}

local function rnd(a, b)
    return a + math.random() * (b - a)
end

local function spawn(x, y, opts)
    local half   = math.rad(opts.spread or 180)
    local base   = math.rad(opts.angle  or 270)
    local ang    = base + rnd(-half, half)
    local speed  = rnd((opts.speed or 80) * 0.5, opts.speed or 80)
    local life   = opts.lifetime or 1.0
    local sz     = opts.size    or 5
    local szEnd  = opts.sizeEnd or 0
    local c      = opts.color   or Color.White
    local cEnd   = opts.colorEnd
    _pool[#_pool + 1] = {
        x = x, y = y,
        vx = math.cos(ang) * speed,
        vy = math.sin(ang) * speed,
        life = life, maxLife = life,
        sz = sz, szEnd = szEnd,
        r  = c.r,   g  = c.g,   b  = c.b,   a  = c.a,
        er = cEnd and cEnd.r or c.r,
        eg = cEnd and cEnd.g or c.g,
        eb = cEnd and cEnd.b or c.b,
        ea = cEnd and cEnd.a or 0,
        gravity = opts.gravity or 0,
    }
end

function Particles.emit(x, y, opts)
    opts = opts or {}
    local n = opts.count or 12
    for _ = 1, n do spawn(x, y, opts) end
end

function Particles.emitter(x, y, opts)
    opts = opts or {}
    local h = { _active = true, _accum = 0, x = x, y = y, opts = opts }
    _emitters[#_emitters + 1] = h
    return h
end

function Particles.move(handle, x, y)
    if handle then handle.x = x; handle.y = y end
end

function Particles.stop(handle)
    if handle then handle._active = false end
end

function Particles.clear()
    _pool     = {}
    _emitters = {}
end

function Particles.count()
    return #_pool
end

function Particles._update(dt)
    -- Émetteurs continus
    local alive = {}
    for _, em in ipairs(_emitters) do
        if em._active then
            em._accum = em._accum + dt
            local rate = em.opts.rate or 0.04
            while em._accum >= rate do
                em._accum = em._accum - rate
                local o = {}
                for k, v in pairs(em.opts) do o[k] = v end
                o.count = 1
                spawn(em.x, em.y, o)
            end
            alive[#alive + 1] = em
        end
    end
    _emitters = alive

    -- Particules
    local surviving = {}
    for _, p in ipairs(_pool) do
        p.life = p.life - dt
        if p.life > 0 then
            p.x  = p.x + p.vx * dt
            p.y  = p.y + p.vy * dt
            p.vy = p.vy + p.gravity * dt

            local t  = 1.0 - p.life / p.maxLife
            local sz = p.sz + (p.szEnd - p.sz) * t
            if sz > 0 then
                local r = math.floor(p.r + (p.er - p.r) * t + 0.5)
                local g = math.floor(p.g + (p.eg - p.g) * t + 0.5)
                local b = math.floor(p.b + (p.eb - p.b) * t + 0.5)
                local a = math.floor(p.a + (p.ea - p.a) * t + 0.5)
                if a > 0 then
                    Debug.fillRect(p.x - sz * 0.5, p.y - sz * 0.5, sz, sz,
                        Color.new(r, g, b, a), 0)
                end
            end
            surviving[#surviving + 1] = p
        end
    end
    _pool = surviving
end

-- Nettoyage automatique au changement de scène : vide les particules et
-- émetteurs actifs pour éviter qu'ils se rendent par-dessus la nouvelle scène.
EventBus.on("scene_changed", function()
    Particles.clear()
end)

return Particles
