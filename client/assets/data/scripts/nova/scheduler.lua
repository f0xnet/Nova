-- nova/scheduler.lua
-- Scheduler de coroutines : équivalent Lua du Utility.Wait() de Papyrus.
-- Disponible sans require : Scheduler est un global auto-chargé.
--
-- API :
--   Scheduler.start(fn, scope, ...)   démarre une coroutine (scope optionnel avant args)
--   Scheduler.startWith(fn, args, scope) démarre avec args passés en table (scope optionnel)
--   Scheduler.wait(seconds)           suspend la coroutine courante N secondes
--   Scheduler.cancelScope(scope)      stoppe toutes les coroutines d'un scope
--   Scheduler.clear()                 stoppe toutes les coroutines (reset global)
--   Scheduler.update(dt)              appelé par ScriptSystem
--
-- Note : scope est optionnel — si absent, Scene.current() est utilisé automatiquement.
-- Les coroutines sont donc toujours liées à une scène et nettoyées au changement de scène.
--
-- Différence start vs startWith :
--   Scheduler.start(fn, nil, arg1, arg2)        -- nil requis pour passer args sans scope
--   Scheduler.startWith(fn, {arg1, arg2})       -- plus lisible, scope optionnel en 3e param

local Scheduler = {}
local routines  = {}

function Scheduler.start(fn, scope, ...)
    local args = { ... }
    local resolvedScope = scope ~= nil and scope or Scene.current()
    local co = coroutine.create(function() fn(table.unpack(args)) end)
    local ok, yieldVal = coroutine.resume(co)
    if not ok then
        Log.error("Scheduler.start error: " .. tostring(yieldVal))
        return nil
    end
    if coroutine.status(co) ~= "dead" then
        table.insert(routines, { co = co, wait = tonumber(yieldVal) or 0, scope = resolvedScope })
    end
    return co
end

-- Démarre une coroutine avec les arguments passés en table.
-- Évite le nil obligatoire de start() quand on veut passer des args sans scope.
--   Scheduler.startWith(fn, {arg1, arg2})          -- auto-scope
--   Scheduler.startWith(fn, {arg1, arg2}, "scope") -- scope explicite
function Scheduler.startWith(fn, args, scope)
    args = args or {}
    return Scheduler.start(fn, scope, table.unpack(args))
end

function Scheduler.wait(seconds)
    coroutine.yield(seconds or 0)
end

function Scheduler.cancelScope(scope)
    local alive = {}
    for _, r in ipairs(routines) do
        if r.scope ~= scope then alive[#alive + 1] = r end
    end
    routines = alive
end

function Scheduler.clear()
    routines = {}
end

function Scheduler.update(dt)
    local alive = {}
    for _, r in ipairs(routines) do
        r.wait = r.wait - dt
        if r.wait <= 0 then
            if coroutine.status(r.co) ~= "dead" then
                local ok, yieldVal = coroutine.resume(r.co)
                if not ok then
                    Log.error("Scheduler resume error: " .. tostring(yieldVal))
                elseif coroutine.status(r.co) ~= "dead" then
                    r.wait = tonumber(yieldVal) or 0
                    alive[#alive + 1] = r
                end
            end
        else
            alive[#alive + 1] = r
        end
    end
    routines = alive
end

return Scheduler
