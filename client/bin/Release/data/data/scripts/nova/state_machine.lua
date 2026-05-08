-- nova/state_machine.lua
-- Machine à états finis (FSM) légère.
-- Disponible sans require : StateMachine est un global auto-chargé.
--
-- API :
--   local sm = StateMachine.new("idle")
--   sm:addState("idle",  onEnter, onUpdate, onExit)
--   sm:addState("chase", onEnter, onUpdate, onExit)
--   sm:transition("chase")       -- transition au prochain update
--   sm:update(entity, dt)        -- à appeler dans la fonction update du script
--   sm:getState()                -- retourne l'état courant (string)

local StateMachine   = {}
StateMachine.__index = StateMachine

function StateMachine.new(initialState)
    return setmetatable({
        current            = initialState,
        states             = {},
        _pendingTransition = nil
    }, StateMachine)
end

function StateMachine:addState(name, onEnter, onUpdate, onExit)
    self.states[name] = {
        onEnter  = onEnter  or function() end,
        onUpdate = onUpdate or function() end,
        onExit   = onExit   or function() end,
    }
    return self  -- chaînable
end

function StateMachine:transition(newState)
    if newState ~= self.current then
        self._pendingTransition = newState
    end
end

function StateMachine:update(entity, dt)
    -- Applique la transition en début de frame pour que onExit/onEnter soient propres
    if self._pendingTransition then
        local old = self.states[self.current]
        if old then
            local ok, err = pcall(old.onExit, entity)
            if not ok then Log.error("FSM onExit '" .. self.current .. "': " .. err) end
        end
        self.current           = self._pendingTransition
        self._pendingTransition = nil
        local new = self.states[self.current]
        if new then
            local ok, err = pcall(new.onEnter, entity)
            if not ok then Log.error("FSM onEnter '" .. self.current .. "': " .. err) end
        end
    end

    local state = self.states[self.current]
    if state then
        local ok, err = pcall(state.onUpdate, entity, dt)
        if not ok then Log.error("FSM onUpdate '" .. self.current .. "': " .. err) end
    end
end

function StateMachine:getState()
    return self.current
end

return StateMachine
