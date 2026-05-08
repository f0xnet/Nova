-- nova/sequence.lua
-- Séquenceur d'événements pour cutscènes et scripts narratifs.
-- Disponible sans require : Sequence est un global auto-chargé.
--
-- API :
--   local seq = Sequence.new()
--   seq:wait(seconds)                  -- pause
--   seq:call(fn)                       -- appel synchrone
--   seq:tween(seconds, fn)             -- appel fn(t) chaque frame, t = 0..1
--   seq:emit(event, data)              -- EventBus.emit
--   seq:dialogue(speakerId, text)      -- démarre un dialogue et attend la fin
--   seq:repeat(n, fn)                  -- répète fn(i) n fois immédiatement
--   seq:play(onComplete)               -- lance la séquence
--   seq:stop()                         -- arrête la séquence
--   Sequence._update(dt)               -- appelé par ScriptSystem chaque frame
--
-- Exemple :
--   local seq = Sequence.new()
--   seq:call(function() Camera.shake(0.3, 5) end)
--   seq:wait(0.5)
--   seq:dialogue("elder", "The prophecy begins...")
--   seq:tween(1.0, function(t) PostFX.setBlur(t * 5) end)
--   seq:wait(0.2)
--   seq:emit("cutscene_end", {})
--   seq:play(function() Player.unlock() end)

local Sequence = {}
local _running = {}  -- liste des séquences actives
local _advanceSeq   -- forward declaration

function Sequence.new()
    local seq = {
        _steps    = {},
        _active   = false,
        _cursor   = 1,
        _timer    = 0,
        _tweenFn  = nil,
        _tweenDur = 0,
        _tweenElapsed = 0,
        _waiting  = false,  -- attend "dialogue_end"
        _onComplete = nil,
    }

    function seq:wait(seconds)
        self._steps[#self._steps + 1] = { type = "wait", duration = seconds }
        return self
    end

    function seq:call(fn)
        self._steps[#self._steps + 1] = { type = "call", fn = fn }
        return self
    end

    function seq:tween(seconds, fn)
        self._steps[#self._steps + 1] = { type = "tween", duration = seconds, fn = fn }
        return self
    end

    function seq:emit(event, data)
        self._steps[#self._steps + 1] = { type = "emit", event = event, data = data or {} }
        return self
    end

    function seq:dialogue(speakerId, text)
        self._steps[#self._steps + 1] = { type = "dialogue", speaker = speakerId, text = text }
        return self
    end

    -- Répétition synchrone (n appels immédiats de fn(i)) — utile pour setup
    function seq:repeat_(n, fn)
        self._steps[#self._steps + 1] = { type = "repeat", count = n, fn = fn }
        return self
    end

    function seq:play(onComplete)
        self._active     = true
        self._cursor     = 1
        self._onComplete = onComplete
        _running[#_running + 1] = self
        _advanceSeq(self, 0)
        return self
    end

    function seq:stop()
        self._active = false
    end

    return seq
end

-- Avance une séquence d'un dt ; retourne false quand terminée
_advanceSeq = function(seq, dt)
    if not seq._active then return false end

    -- Bloc tween en cours
    if seq._tweenFn then
        seq._tweenElapsed = seq._tweenElapsed + dt
        local t = math.min(seq._tweenElapsed / seq._tweenDur, 1.0)
        seq._tweenFn(t)
        if t >= 1.0 then
            seq._tweenFn  = nil
            seq._cursor   = seq._cursor + 1
        end
        return true
    end

    -- Attend la fin d'un dialogue
    if seq._waiting then return true end

    -- Bloc wait en cours
    if seq._timer > 0 then
        seq._timer = seq._timer - dt
        if seq._timer > 0 then
            return true
        end
        seq._timer  = 0
        seq._cursor = seq._cursor + 1
        -- fall through to process next steps
    end

    -- Exécute les étapes instantanées jusqu'à bloquer
    while seq._cursor <= #seq._steps do
        local step = seq._steps[seq._cursor]

        if step.type == "call" then
            step.fn()
            seq._cursor = seq._cursor + 1

        elseif step.type == "emit" then
            EventBus.emit(step.event, step.data)
            seq._cursor = seq._cursor + 1

        elseif step.type == "repeat" then
            for i = 1, step.count do step.fn(i) end
            seq._cursor = seq._cursor + 1

        elseif step.type == "wait" then
            seq._timer  = step.duration
            return true  -- sera consommé au prochain frame

        elseif step.type == "tween" then
            seq._tweenDur     = step.duration
            seq._tweenElapsed = 0
            seq._tweenFn      = step.fn
            return true

        elseif step.type == "dialogue" then
            seq._waiting = true
            -- Écoute la fin du dialogue
            local self_seq = seq
            EventBus.once("dialogue_end", function(data)
                if data.speaker == nil or data.speaker == self_seq._steps[self_seq._cursor].speaker then
                    self_seq._waiting = false
                    self_seq._cursor  = self_seq._cursor + 1
                end
            end)
            EventBus.emit("dialogue_start", { speaker = step.speaker, text = step.text })
            return true
        end
    end

    -- Terminé
    seq._active = false
    if seq._onComplete then seq._onComplete() end
    return false
end

-- Appelé par ScriptSystem chaque frame
function Sequence._update(dt)
    local i = 1
    while i <= #_running do
        local still_running = _advanceSeq(_running[i], dt)
        if not still_running then
            table.remove(_running, i)
        else
            i = i + 1
        end
    end
end

-- Nettoyage automatique au changement de scène : arrête toutes les séquences
-- actives pour éviter qu'elles continuent à s'exécuter dans la nouvelle scène.
EventBus.on("scene_changed", function()
    for _, seq in ipairs(_running) do
        seq._active = false
    end
    _running = {}
end)

return Sequence
