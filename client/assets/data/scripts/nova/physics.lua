-- nova/physics.lua
-- Requêtes spatiales pures Lua — AABB, cercle, raycast.
-- Utilise ColliderComponent + World.forEach (cache par tag).
-- Disponible sans require : Physics est un global auto-chargé.
--
-- API :
--   Physics.findInRadius(origin, radius, tag)         -- entités dans un rayon
--   Physics.nearest(origin, radius, tag)              -- entité la plus proche dans un rayon
--   Physics.overlap(entityA, entityB)                 -- chevauchement AABB entre deux entités
--   Physics.overlapPoint(x, y, tag)                   -- entités dont le collider contient le point
--   Physics.raycast(ox, oy, dx, dy, maxDist, tag)     -- rayon → première entité touchée
--
-- origin peut être :
--   une entité (userdata)    — position extraite via getPosition()
--   une table {x=…, y=…}    — position directe
--
-- Note : approximation AABB/cercle sans moteur physique dédié.
--
-- Exemple :
--   -- Trouver tous les ennemis à portée d'attaque
--   local enemies = Physics.findInRadius(entity, 120, "enemy")
--   for _, e in ipairs(enemies) do
--       Stats.mod(e.id, "Health", -damage)
--   end
--
--   -- Ligne de vue vers la cible (bloquée par les murs ?)
--   local pos = entity:getPosition()
--   local tpos = target:getPosition()
--   local dx, dy = tpos.x - pos.x, tpos.y - pos.y
--   local wall = Physics.raycast(pos.x, pos.y, dx, dy, 500, "wall")
--   local hasLOS = (wall == nil)

local Physics = {}

-- Retourne (x, y) depuis une entité userdata ou une table {x,y}
local function getXY(origin)
    if type(origin) == "userdata" then
        local pos = origin:getPosition()
        return (pos and pos.x) or 0, (pos and pos.y) or 0
    elseif type(origin) == "table" then
        return origin.x or 0, origin.y or 0
    end
    return 0, 0
end

-- Retourne le bounding box { x, y, hw, hh, r, isCircle } d'une entité
-- Utilise ColliderComponent si présent, sinon SpriteComponent comme fallback
local function getAABB(e)
    local t = e:getTransform()
    if not t then return nil end
    local c = e:getCollider()
    if c and c.enabled then
        if c.radius > 0 and c.size.x == 0 then
            return { x = t.position.x + c.offset.x, y = t.position.y + c.offset.y,
                     hw = c.radius, hh = c.radius, r = c.radius, isCircle = true }
        end
        return { x = t.position.x + c.offset.x, y = t.position.y + c.offset.y,
                 hw = c.size.x * 0.5, hh = c.size.y * 0.5, r = 0, isCircle = false }
    end
    local s = e:getSprite()
    local hw = (s and s.size.x > 0) and s.size.x * 0.5 or 16
    local hh = (s and s.size.y > 0) and s.size.y * 0.5 or 16
    return { x = t.position.x, y = t.position.y, hw = hw, hh = hh, r = 0, isCircle = false }
end

local function dist2(x1, y1, x2, y2)
    local dx, dy = x2 - x1, y2 - y1
    return dx * dx + dy * dy
end

-- Toutes les entités avec ce tag dont le centre est dans le rayon
function Physics.findInRadius(origin, radius, tag)
    local ox, oy = getXY(origin)
    local r2     = radius * radius
    local result = {}
    World.forEach(tag, function(e)
        local aabb = getAABB(e)
        if aabb and dist2(ox, oy, aabb.x, aabb.y) <= r2 then
            table.insert(result, e)
        end
    end)
    return result
end

-- Entité la plus proche dans le rayon (nil si aucune), + la distance
function Physics.nearest(origin, radius, tag)
    local ox, oy   = getXY(origin)
    local r2       = radius * radius
    local best, bestD2 = nil, r2 + 1
    World.forEach(tag, function(e)
        local aabb = getAABB(e)
        if not aabb then return end
        local d2 = dist2(ox, oy, aabb.x, aabb.y)
        if d2 < bestD2 then best, bestD2 = e, d2 end
    end)
    return best, best and math.sqrt(bestD2) or nil
end

-- Vrai si les bounding boxes de deux entités se chevauchent
function Physics.overlap(entityA, entityB)
    local a = getAABB(entityA)
    local b = getAABB(entityB)
    if not a or not b then return false end
    if a.isCircle or b.isCircle then
        local sumR = (a.isCircle and a.r or math.max(a.hw, a.hh))
                   + (b.isCircle and b.r or math.max(b.hw, b.hh))
        return dist2(a.x, a.y, b.x, b.y) <= sumR * sumR
    end
    return math.abs(a.x - b.x) < (a.hw + b.hw)
       and math.abs(a.y - b.y) < (a.hh + b.hh)
end

-- Entités avec ce tag dont le collider contient le point (x, y)
function Physics.overlapPoint(x, y, tag)
    local result = {}
    World.forEach(tag, function(e)
        local aabb = getAABB(e)
        if not aabb then return end
        if aabb.isCircle then
            if dist2(x, y, aabb.x, aabb.y) <= aabb.r * aabb.r then
                table.insert(result, e)
            end
        else
            if math.abs(x - aabb.x) <= aabb.hw
            and math.abs(y - aabb.y) <= aabb.hh then
                table.insert(result, e)
            end
        end
    end)
    return result
end

-- Slab method : intersection rayon/AABB
-- Retourne (tmin, tmax) ou (1, -1) si pas d'intersection
local function slabAABB(ox, oy, nx, ny, aabb)
    local function slab1D(origin, dir, center, half)
        if math.abs(dir) < 1e-8 then
            return (math.abs(origin - center) <= half) and -math.huge or 1,
                   (math.abs(origin - center) <= half) and  math.huge or -1
        end
        local t1 = (center - half - origin) / dir
        local t2 = (center + half - origin) / dir
        return math.min(t1, t2), math.max(t1, t2)
    end
    local tx1, tx2 = slab1D(ox, nx, aabb.x, aabb.hw)
    local ty1, ty2 = slab1D(oy, ny, aabb.y, aabb.hh)
    return math.max(tx1, ty1), math.min(tx2, ty2)
end

-- Rayon depuis (ox, oy) dans la direction (dx, dy), longueur max maxDist.
-- Retourne l'entité la plus proche touchée + la distance (t), ou nil.
function Physics.raycast(ox, oy, dx, dy, maxDist, tag)
    local len = math.sqrt(dx * dx + dy * dy)
    if len < 1e-8 then return nil, nil end
    local nx, ny = dx / len, dy / len
    maxDist = maxDist or math.huge

    local best, bestT = nil, maxDist
    World.forEach(tag, function(e)
        local aabb = getAABB(e)
        if not aabb then return end
        local tmin, tmax = slabAABB(ox, oy, nx, ny, aabb)
        if tmax >= tmin and tmin >= 0 and tmin < bestT then
            best, bestT = e, tmin
        end
    end)
    return best, best and bestT or nil
end

return Physics
