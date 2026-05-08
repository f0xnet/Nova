-- nova/timer.lua
-- Timers one-shot et répétitifs. Mis à jour automatiquement chaque frame.
-- Disponible sans require : Timer est un global auto-chargé.
--
-- API :
--   Timer.after(delay, callback, scope)   → id   appel unique après delay secondes
--   Timer.every(interval, callback, scope)→ id   appel répété toutes les N secondes
--   Timer.cancel(id)                            annule un timer
--   Timer.cancelScope(scope)                    annule tous les timers d'un scope
--   Timer.cancelAll()                           annule tous les timers (reset global)
--   Timer.update(dt)                            appelé par ScriptSystem

local Timer  = {}
local timers = {}
local nextId = 1

function Timer.after(delay, callback, scope)
    local id = nextId; nextId = nextId + 1
    timers[id] = { remaining = delay, callback = callback, interval = nil, scope = scope }
    return id
end

function Timer.every(interval, callback, scope)
    local id = nextId; nextId = nextId + 1
    timers[id] = { remaining = interval, callback = callback, interval = interval, scope = scope }
    return id
end

function Timer.cancel(id)
    timers[id] = nil
end

function Timer.cancelScope(scope)
    for id, t in pairs(timers) do
        if t.scope == scope then timers[id] = nil end
    end
end

function Timer.cancelAll()
    timers = {}
end

function Timer.update(dt)
    for id, t in pairs(timers) do
        t.remaining = t.remaining - dt
        if t.remaining <= 0 then
            local ok, err = pcall(t.callback)
            if not ok then
                Log.error("Timer error: " .. tostring(err))
                timers[id] = nil
            elseif t.interval then
                t.remaining = t.interval + t.remaining
            else
                timers[id] = nil
            end
        end
    end
end

return Timer
