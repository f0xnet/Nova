-- nova/nav.lua
-- Navigation par waypoints en espace monde (pathfinding via WaypointGraph C++).
-- Disponible sans require : Nav est un global auto-chargé.
--
-- API :
--   Nav.findPath(entity, target)             -- calcule un chemin, retourne [{x,y}...]
--   Nav.moveTo(entity, target, speed, onDone) -- démarre la navigation vers la cible
--   Nav.moveAlongPath(entity, path, speed, onDone) -- suit un chemin pré-calculé
--   Nav.stop(entity)                         -- arrête la navigation
--   Nav.isMoving(entity)                     -- vrai si en déplacement
--   Nav._update(dt)                          -- appelé par ScriptSystem chaque frame
--
-- target peut être :
--   - une entité (userdata ou entityId)
--   - une position { x, y }
--
-- Dépend de Scene.findPath(x1, y1, x2, y2) (ajouté à nova/scene.lua).
--
-- Exemple :
--   -- NPC patrouillie vers un waypoint
--   Nav.moveTo(npc, { x = 400, y = 300 }, 80, function()
--       Timer.after(2.0, function()
--           Nav.moveTo(npc, startPos, 80)
--       end)
--   end)
--
--   -- Pathfinding explicite
--   local path = Nav.findPath(npc, player)
--   Nav.moveAlongPath(npc, path, 100, function()
--       Log.info("Arrivé !")
--   end)

local Nav = {}

-- Seuil en pixels pour considérer un waypoint comme atteint
local WAYPOINT_REACH = 8.0

-- _navState[entityId] = { path, index, speed, onDone }
local _navState = {}

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

local function getEntity(v)
    if type(v) == "userdata" then return v end
    return Registry:getEntity(v)
end

-- Retourne {x, y} depuis une entité, un entityId, ou un {x,y} direct
local function resolvePos(target)
    if type(target) == "table" then
        return target.x, target.y
    end
    local ent = getEntity(target)
    if not ent then return nil, nil end
    local pos = ent:getPosition()
    return pos and pos.x, pos and pos.y
end

function Nav.findPath(entity, target)
    local ent = getEntity(entity)
    if not ent then return {} end
    local pos = ent:getPosition()
    if not pos then return {} end

    local tx, ty = resolvePos(target)
    if not tx then return {} end

    return Scene.findPath(pos.x, pos.y, tx, ty)
end

function Nav.moveTo(entity, target, speed, onDone)
    local path = Nav.findPath(entity, target)
    -- Si pas de waypoints, déplacement direct vers la cible
    if #path == 0 then
        local tx, ty = resolvePos(target)
        if tx then
            path = { { x = tx, y = ty } }
        else
            if onDone then onDone() end
            return
        end
    end
    Nav.moveAlongPath(entity, path, speed, onDone)
end

function Nav.moveAlongPath(entity, path, speed, onDone)
    if not path or #path == 0 then
        if onDone then onDone() end
        return
    end
    local eid = toId(entity)
    _navState[eid] = {
        path   = path,
        index  = 1,
        speed  = speed or 100.0,
        onDone = onDone,
    }
end

function Nav.stop(entity)
    local eid = toId(entity)
    _navState[eid] = nil
end

function Nav.isMoving(entity)
    return _navState[toId(entity)] ~= nil
end

function Nav._update(dt)
    for eid, state in pairs(_navState) do
        local ent = Registry:getEntity(eid)
        if not ent then
            _navState[eid] = nil
        else
            local pos = ent:getPosition()
            if not pos then
                _navState[eid] = nil
            else
                local wp = state.path[state.index]
                if not wp then
                    _navState[eid] = nil
                    if state.onDone then state.onDone() end
                else
                    local dx   = wp.x - pos.x
                    local dy   = wp.y - pos.y
                    local dist = math.sqrt(dx * dx + dy * dy)

                    if dist <= WAYPOINT_REACH then
                        -- Atteint ce waypoint, passe au suivant
                        state.index = state.index + 1
                        if state.index > #state.path then
                            _navState[eid] = nil
                            if state.onDone then state.onDone() end
                        end
                    else
                        -- Avance vers le waypoint
                        local step = math.min(state.speed * dt, dist)
                        local nx = pos.x + (dx / dist) * step
                        local ny = pos.y + (dy / dist) * step
                        ent:setPosition(nx, ny)
                    end
                end
            end
        end
    end
end

return Nav
