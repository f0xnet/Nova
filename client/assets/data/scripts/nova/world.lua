-- nova/world.lua
-- Utilitaires de haut niveau pour interagir avec les entités de la scène.
-- Disponible sans require : World est un global auto-chargé.
--
-- API :
--   World.findByTag(tag)              -- première entité avec ce tag (ou nil)
--   World.findAllByTag(tag)           -- toutes les entités avec ce tag (table)
--   World.forEach(tag, callback)      -- itère les entités, stop si callback retourne false
--   World.count(tag)                  -- nombre d'entités avec ce tag
--   World.nearest(origin, tag)        -- entité la plus proche + distance
--   World.distance(a, b)             -- distance entre deux entités
--   World.getPosition(entity)         -- Vec2f de l'entité
--   World.setPosition(entity, x, y)   -- téléporte une entité
--   World.spawn(typeID)               -- crée une entité avec un composant
--   World.destroy(entityId)           -- détruit une entité
--   World.raycast(x1,y1,x2,y2,tag)   -- rayon ; retourne { entity,x,y,distance,t } ou nil
--
-- PERFORMANCE :
--   Le cache par tag est reconstruit une fois par frame maximum.
--   Appeler World.findByTag("enemy") 10 fois dans la même frame = 1 seul parcours.

local World = {}

-- Cache par tag — invalidé au début de chaque frame par ScriptSystem
local tagCache   = {}
local cacheValid = false

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

-- Appelé par ScriptSystem._update au début de chaque frame
function World._update(dt)
    cacheValid = false
end

-- Reconstruction du cache (parcours unique de toutes les entités)
local function ensureCache()
    if cacheValid then return end
    tagCache = {}
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t then
            local tag = t.tag
            if not tagCache[tag] then tagCache[tag] = {} end
            table.insert(tagCache[tag], e)
        end
    end
    cacheValid = true
end

function World.findByTag(tag)
    ensureCache()
    local list = tagCache[tag]
    return list and list[1] or nil
end

function World.findAllByTag(tag)
    ensureCache()
    -- Retourne une copie pour éviter la modification du cache
    local result = {}
    local list = tagCache[tag]
    if list then
        for _, e in ipairs(list) do table.insert(result, e) end
    end
    return result
end

-- Itère toutes les entités avec ce tag.
-- callback(entity) — retourner false pour arrêter l'itération.
function World.forEach(tag, callback)
    ensureCache()
    local list = tagCache[tag]
    if not list then return end
    for _, e in ipairs(list) do
        local ok, result = pcall(callback, e)
        if not ok then
            Log.error("World.forEach('" .. tag .. "'): " .. tostring(result))
        elseif result == false then
            break
        end
    end
end

function World.count(tag)
    ensureCache()
    local list = tagCache[tag]
    return list and #list or 0
end

-- Distance entre deux entités (Vec2, via Vec2.distance)
function World.distance(a, b)
    local ta = a:getTransform()
    local tb = b:getTransform()
    if not ta or not tb then return math.huge end
    local dx = ta.position.x - tb.position.x
    local dy = ta.position.y - tb.position.y
    return math.sqrt(dx * dx + dy * dy)
end

function World.getPosition(e)
    local t = e:getTransform()
    if not t then return nil end
    return t.position
end

function World.setPosition(e, x, y)
    local t = e:getTransform()
    if t then t.position.x = x; t.position.y = y end
end

function World.spawn(typeID)
    local e = Registry:createEntity()
    if typeID then e:addComponent(typeID) end
    return e
end

function World.destroy(entityId)
    Registry:destroyEntity(toId(entityId))
end

-- Lance un rayon de (x1,y1) à (x2,y2) et retourne le premier hit :
--   { entity, x, y, distance, t }   où t ∈ [0,1] le long du rayon
-- filterTag : si fourni, ne teste que les entités avec ce tag.
-- Retourne nil si aucun hit.
--
-- Exemple (ligne de vue mur → joueur) :
--   local hit = World.raycast(npc.x, npc.y, player.x, player.y, "wall")
--   if not hit then -- aucun mur entre les deux → ligne de vue libre end
function World.raycast(x1, y1, x2, y2, filterTag)
    local dx  = x2 - x1
    local dy  = y2 - y1
    local len = math.sqrt(dx * dx + dy * dy)
    if len == 0 then return nil end

    local bestT, bestEnt = math.huge, nil
    local bestX, bestY   = 0, 0

    local candidates = filterTag
        and World.findAllByTag(filterTag)
        or  Registry:getAllEntities()

    for _, e in ipairs(candidates) do
        local tr  = e:getTransform()
        if not tr then goto continue end
        local col = e:getCollider()
        if not col or not col.enabled then goto continue end

        local cx = tr.position.x + col.offset.x
        local cy = tr.position.y + col.offset.y
        local t

        if col.radius > 0 then
            -- Rayon vs Cercle (quadratique)
            local ox  = x1 - cx
            local oy  = y1 - cy
            local a   = dx * dx + dy * dy
            local b   = 2 * (ox * dx + oy * dy)
            local c   = ox * ox + oy * oy - col.radius * col.radius
            local d   = b * b - 4 * a * c
            if d < 0 then goto continue end
            local sq  = math.sqrt(d)
            local t0  = (-b - sq) / (2 * a)
            local t1  = (-b + sq) / (2 * a)
            if t1 < 0 or t0 > 1 then goto continue end
            t = (t0 >= 0) and t0 or 0
        else
            -- Rayon vs AABB (méthode des slabs)
            -- (cx, cy) = coin supérieur gauche, cohérent avec updateCollisionBridge C++
            local tEnter = -math.huge
            local tExit  =  math.huge

            if dx == 0 then
                if x1 < cx or x1 > cx + col.size.x then goto continue end
            else
                local t1x = (cx - x1) / dx
                local t2x = (cx + col.size.x - x1) / dx
                if t1x > t2x then t1x, t2x = t2x, t1x end
                tEnter = math.max(tEnter, t1x)
                tExit  = math.min(tExit,  t2x)
            end

            if dy == 0 then
                if y1 < cy or y1 > cy + col.size.y then goto continue end
            else
                local t1y = (cy - y1) / dy
                local t2y = (cy + col.size.y - y1) / dy
                if t1y > t2y then t1y, t2y = t2y, t1y end
                tEnter = math.max(tEnter, t1y)
                tExit  = math.min(tExit,  t2y)
            end

            if tEnter > tExit or tExit < 0 or tEnter > 1 then goto continue end
            t = math.max(0, tEnter)
        end

        if t < bestT then
            bestT    = t
            bestEnt  = e
            bestX    = x1 + dx * t
            bestY    = y1 + dy * t
        end

        ::continue::
    end

    if not bestEnt then return nil end
    return { entity = bestEnt, x = bestX, y = bestY, distance = bestT * len, t = bestT }
end

-- Retourne l'entité la plus proche ayant ce tag, et la distance
function World.nearest(origin, tag)
    ensureCache()
    local list = tagCache[tag]
    if not list then return nil, math.huge end
    local best, bestDist = nil, math.huge
    for _, e in ipairs(list) do
        local d = World.distance(origin, e)
        if d < bestDist then
            best, bestDist = e, d
        end
    end
    return best, bestDist
end

return World
