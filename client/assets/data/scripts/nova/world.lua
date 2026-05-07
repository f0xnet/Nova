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
