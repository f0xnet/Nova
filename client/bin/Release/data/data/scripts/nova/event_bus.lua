-- nova/event_bus.lua
-- Bus d'événements global partagé par tous les scripts.
-- Disponible sans require : EventBus est un global auto-chargé.
--
-- API :
--   EventBus.on(event, handler)       -- s'abonner (persistant)
--   EventBus.once(event, handler)     -- s'abonner une seule fois
--   EventBus.off(event, handler)      -- se désabonner
--   EventBus.emit(event, data)        -- émettre (data optionnel)
--   EventBus.clear(event)            -- supprimer tous les handlers d'un event
--   EventBus.clear()                  -- tout supprimer

local EventBus = {}
local listeners = {}

function EventBus.on(event, handler)
    if not listeners[event] then listeners[event] = {} end
    table.insert(listeners[event], { fn = handler, once = false })
end

function EventBus.once(event, handler)
    if not listeners[event] then listeners[event] = {} end
    table.insert(listeners[event], { fn = handler, once = true })
end

function EventBus.off(event, handler)
    if not listeners[event] then return end
    for i = #listeners[event], 1, -1 do
        if listeners[event][i].fn == handler then
            table.remove(listeners[event], i)
        end
    end
end

function EventBus.emit(event, data)
    if not listeners[event] then return end
    local toRemove = {}
    for i, l in ipairs(listeners[event]) do
        local ok, err = pcall(l.fn, data)
        if not ok then
            Log.error("EventBus '" .. tostring(event) .. "': " .. tostring(err))
        end
        if l.once then table.insert(toRemove, i) end
    end
    for i = #toRemove, 1, -1 do
        table.remove(listeners[event], toRemove[i])
    end
end

function EventBus.clear(event)
    if event then
        listeners[event] = nil
    else
        listeners = {}
    end
end

return EventBus
