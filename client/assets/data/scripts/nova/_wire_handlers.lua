-- nova/_wire_handlers.lua
-- Auto-wiring des named handlers + nettoyage de contexte Lua par entité.
-- Chargé par ScriptSystem au démarrage — ne pas modifier les noms de fonctions.
--
-- Fonctions globales exposées :
--   __wireNamedHandlers(env, entityId, isSceneScript)
--   __cleanEntityHandlers(entityId)   -- utilisé par ScriptRegistry.reload()

local _entityHandlers = {}

-- Retire immédiatement tous les handlers trackés d'une entité.
-- Appelé par ScriptRegistry.reload() avant de réinitialiser le ScriptComponent.
function __cleanEntityHandlers(entityId)
    local handlers = _entityHandlers[entityId]
    if not handlers then return end
    for _, h in ipairs(handlers) do EventBus.off(h[1], h[2]) end
    _entityHandlers[entityId] = nil
end

-- isSceneScript : true pour les scripts de scène (entityId=0, nettoyés à scene_changing).
-- Scripts globaux purs (entityId=0, isSceneScript=false) : jamais nettoyés automatiquement.
function __wireNamedHandlers(env, entityId, isSceneScript)
    local function safe(fn, ...)
        local ok, err = pcall(fn, ...)
        if not ok then Log.error("[AutoWire] " .. tostring(err)) end
    end

    local handlers = {}
    local function reg(event, fn)
        EventBus.on(event, fn)
        handlers[#handlers + 1] = { event, fn }
    end

    if type(env.OnActivate) == "function" then
        reg("activator_on", function(data)
            if data.entityId == entityId then safe(env.OnActivate, data.entityId, data.actionID) end
        end)
    end
    if type(env.OnDeactivate) == "function" then
        reg("activator_off", function(data)
            if data.entityId == entityId then safe(env.OnDeactivate, data.entityId, data.actionID) end
        end)
    end

    if type(env.OnKeyDown) == "function" then
        reg("key_down", function(data) safe(env.OnKeyDown, data.key) end)
    end
    if type(env.OnKeyUp) == "function" then
        reg("key_up",   function(data) safe(env.OnKeyUp,   data.key) end)
    end

    if type(env.OnMouseDown) == "function" then
        reg("mouse_down", function(data) safe(env.OnMouseDown, data.button, data.x, data.y) end)
    end
    if type(env.OnMouseUp) == "function" then
        reg("mouse_up", function(data) safe(env.OnMouseUp, data.button, data.x, data.y) end)
    end

    if type(env.OnAnimationEvent) == "function" then
        reg("animation_frame", function(data)
            if data.entityId == entityId then safe(env.OnAnimationEvent, data.animationID, data.frame) end
        end)
    end
    if type(env.OnAnimationChanged) == "function" then
        reg("animation_changed", function(data)
            if data.entityId == entityId then safe(env.OnAnimationChanged, data.animationID, data.previousID) end
        end)
    end

    if type(env.OnMessage) == "function" then
        reg("script_message_" .. entityId, function(data)
            safe(env.OnMessage, data.msg, data.payload)
        end)
    end

    if type(env.OnQuestComplete) == "function" then
        reg("quest_completed", function(data) safe(env.OnQuestComplete, data.questId) end)
    end
    if type(env.OnQuestAdvanced) == "function" then
        reg("quest_advanced", function(data) safe(env.OnQuestAdvanced, data.questId, data.stageIndex) end)
    end

    if type(env.OnStatZeroed) == "function" then
        reg("stat_zeroed", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnStatZeroed, data.entityId, data.stat)
            end
        end)
    end

    if type(env.OnItemAdded) == "function" then
        reg("item_added", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnItemAdded, data.item, data.count) end
        end)
    end
    if type(env.OnItemRemoved) == "function" then
        reg("item_removed", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnItemRemoved, data.item, data.count) end
        end)
    end

    if type(env.OnEffectApplied) == "function" then
        reg("effect_applied", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnEffectApplied, data.effectId, data.entityId) end
        end)
    end
    if type(env.OnEffectExpired) == "function" then
        reg("effect_expired", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnEffectExpired, data.effectId, data.entityId) end
        end)
    end

    if type(env.OnFlagSet) == "function" then
        reg("flag_set",   function(data) safe(env.OnFlagSet,   data.name) end)
    end
    if type(env.OnFlagUnset) == "function" then
        reg("flag_unset", function(data) safe(env.OnFlagUnset, data.name) end)
    end

    if type(env.OnTriggerEnter) == "function" then
        reg("trigger_enter", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnTriggerEnter, data.id, data.entityId) end
        end)
    end
    if type(env.OnTriggerExit) == "function" then
        reg("trigger_exit", function(data)
            if entityId == 0 or data.entityId == entityId then safe(env.OnTriggerExit, data.id, data.entityId) end
        end)
    end

    if type(env.OnSceneChanged) == "function" then
        reg("scene_changed", function(data) safe(env.OnSceneChanged, data.name) end)
    end

    if type(env.OnConversationNode) == "function" then
        reg("conversation_node", function(data)
            safe(env.OnConversationNode, data.nodeId, data.speaker, data.text)
        end)
    end
    if type(env.OnConversationEnd) == "function" then
        reg("conversation_end", function(data) safe(env.OnConversationEnd, data.id) end)
    end

    if type(env.OnUIAction) == "function" then
        reg("ui_action", function(data) safe(env.OnUIAction, data.action, data.value, data.id) end)
    end

    if type(env.OnCollisionEnter) == "function" then
        reg("collision_enter_" .. tostring(entityId), function(data)
            safe(env.OnCollisionEnter, data.entityId)
        end)
    end
    if type(env.OnCollisionExit) == "function" then
        reg("collision_exit_" .. tostring(entityId), function(data)
            safe(env.OnCollisionExit, data.entityId)
        end)
    end

    -- ── Scoped EventBus ────────────────────────────────────────────────────────
    -- Injecte EventBus.on / .once surchargeant dans l'env du script pour que
    -- tout abonnement dans init() soit tracké dans la même table `handlers`
    -- et nettoyé automatiquement (entity_destroyed ou scene_changing).
    if entityId ~= 0 or isSceneScript then
        env.EventBus = setmetatable({
            on = function(event, handler)
                EventBus.on(event, handler)
                handlers[#handlers + 1] = { event, handler }
            end,
            once = function(event, handler)
                EventBus.once(event, handler)
                handlers[#handlers + 1] = { event, handler }
            end,
        }, { __index = EventBus })
    end

    -- ── Nettoyage automatique ──────────────────────────────────────────────────
    if entityId ~= 0 then
        _entityHandlers[entityId] = handlers
        local cleanupFn
        cleanupFn = function(data)
            if data.entityId ~= entityId then return end
            for _, h in ipairs(handlers) do EventBus.off(h[1], h[2]) end
            EventBus.off("entity_destroyed", cleanupFn)
            _entityHandlers[entityId] = nil
            local sr = ScriptRegistry
            if sr and sr._envs then sr._envs[entityId] = nil end
            if Stats    then Stats.clear(entityId)         end
            if Effect   then Effect.clear(entityId)        end
            if Cooldown then Cooldown.resetAll(entityId)   end
            if Anim     then Anim.clearCallbacks(entityId) end
            if __cleanBridgeMaps then __cleanBridgeMaps(entityId) end
        end
        EventBus.on("entity_destroyed", cleanupFn)
    elseif isSceneScript then
        local cleanupFn
        cleanupFn = function()
            for _, h in ipairs(handlers) do EventBus.off(h[1], h[2]) end
            EventBus.off("scene_changing", cleanupFn)
        end
        EventBus.on("scene_changing", cleanupFn)
    end
end
