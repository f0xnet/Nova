-- nova/world.lua
-- Utilitaires de haut niveau pour interagir avec les entités de la scène.
-- Disponible sans require : World est un global auto-chargé.
--
-- Dépend du global Registry (mis à jour chaque frame par ScriptSystem).

local World = {}

-- Trouve la première entité avec un TagComponent dont tag == tag
function World.findByTag(tag)
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t and t.tag == tag then return e end
    end
    return nil
end

-- Retourne toutes les entités avec ce tag
function World.findAllByTag(tag)
    local result = {}
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t and t.tag == tag then table.insert(result, e) end
    end
    return result
end

-- Distance entre deux entités (via leur TransformComponent)
function World.distance(a, b)
    local ta = a:getTransform()
    local tb = b:getTransform()
    if not ta or not tb then return math.huge end
    return Vec2.distance(ta.position, tb.position)
end

-- Position d'une entité (Vec2f)
function World.getPosition(e)
    local t = e:getTransform()
    return t and t.position or Vec2.new(0, 0)
end

-- Téléporte une entité
function World.setPosition(e, x, y)
    local t = e:getTransform()
    if t then t.position.x = x; t.position.y = y end
end

-- Crée une entité et lui ajoute un composant par typeID
function World.spawn(typeID)
    local e = Registry:createEntity()
    if typeID then e:addComponent(typeID) end
    return e
end

-- Détruit une entité
function World.destroy(entityId)
    Registry:destroyEntity(entityId)
end

-- Itère toutes les entités avec ce tag et appelle callback(entity)
-- Arrête si callback retourne false explicitement
function World.forEach(tag, callback)
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t and t.tag == tag then
            local ok, result = pcall(callback, e)
            if not ok then
                Log.error("World.forEach('" .. tag .. "'): " .. tostring(result))
            elseif result == false then
                break  -- le callback peut retourner false pour arrêter
            end
        end
    end
end

-- Compte les entités avec ce tag
function World.count(tag)
    local n = 0
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t and t.tag == tag then n = n + 1 end
    end
    return n
end

-- Entité la plus proche ayant un tag donné
function World.nearest(origin, tag)
    local best, bestDist = nil, math.huge
    for _, e in ipairs(Registry:getAllEntities()) do
        local t = e:getTag()
        if t and t.tag == tag then
            local d = World.distance(origin, e)
            if d < bestDist then
                best, bestDist = e, d
            end
        end
    end
    return best, bestDist
end

return World
