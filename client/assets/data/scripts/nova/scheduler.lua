-- nova/scheduler.lua
-- Scheduler de coroutines : équivalent Lua du Utility.Wait() de Papyrus.
-- Disponible sans require : Scheduler est un global auto-chargé.
--
-- API :
--   Scheduler.start(fn, ...)    démarre une coroutine asynchrone
--   Scheduler.wait(seconds)     suspend la coroutine courante N secondes
--   Scheduler.update(dt)        appelé par ScriptSystem, ne pas appeler manuellement
--
-- Exemple :
--   Scheduler.start(function()
--       Log.info("début")
--       Scheduler.wait(2.0)
--       Log.info("2 secondes plus tard")
--       Scheduler.wait(1.0)
--       EventBus.emit("sequence_done")
--   end)

local Scheduler = {}
local routines  = {}

function Scheduler.start(fn, ...)
    local args = { ... }
    local co = coroutine.create(function() fn(table.unpack(args)) end)
    -- Premier resume immédiat
    local ok, yieldVal = coroutine.resume(co)
    if not ok then
        Log.error("Scheduler.start error: " .. tostring(yieldVal))
        return nil
    end
    if coroutine.status(co) ~= "dead" then
        table.insert(routines, { co = co, wait = tonumber(yieldVal) or 0 })
    end
    return co
end

function Scheduler.wait(seconds)
    coroutine.yield(seconds or 0)
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
                    table.insert(alive, r)
                end
            end
        else
            table.insert(alive, r)
        end
    end
    routines = alive
end

function Scheduler.clear()
    routines = {}
end

return Scheduler
