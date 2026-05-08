-- nova/scene_fx.lua
-- Effets visuels plein-écran : fondu au noir, flash, transition de scène.
-- Disponible sans require : SceneFX est un global auto-chargé.
-- Dessiné via DebugDraw (au-dessus de tout, UI incluse).
--
-- API :
--   SceneFX.fadeIn(duration, onComplete)   -- fondu vers le noir   (transparent → noir)
--   SceneFX.fadeOut(duration, onComplete)  -- fondu depuis le noir (noir → transparent)
--   SceneFX.transition(fn, duration)       -- fadeIn → fn() → fadeOut
--   SceneFX.flash(color, duration)         -- éclat de couleur rapide (défaut blanc)
--   SceneFX.isActive()                     -- vrai si un effet est en cours
--   SceneFX._update(dt)                    -- appelé par ScriptSystem chaque frame
--
-- Exemple :
--   -- Transition de scène avec fondu
--   SceneFX.transition(function()
--       Scene.setActive("dungeon_01")
--   end, 0.6)
--
--   -- Flash de dégât (rouge)
--   SceneFX.flash(Color.new(255, 0, 0, 180), 0.25)

local SceneFX = {}

local _alpha   = 0          -- 0..255
local _r, _g, _b = 0, 0, 0
local _tweenId = nil

function SceneFX._update(dt)
    if _alpha <= 0 then return end
    local ws = Viewport.getWindowSize()
    Debug.fillRect(0, 0, ws.x, ws.y,
        Color.new(_r, _g, _b, math.min(255, math.floor(_alpha))), 0)
end

local function cancelCurrent()
    if _tweenId then
        Tween.cancel(_tweenId)
        _tweenId = nil
    end
end

function SceneFX.fadeIn(duration, onComplete)
    cancelCurrent()
    _r, _g, _b = 0, 0, 0
    _tweenId = Tween.new(_alpha, 255, duration or 0.5, "linear",
        function(v) _alpha = v end,
        function()
            _alpha   = 255
            _tweenId = nil
            if onComplete then onComplete() end
        end)
end

function SceneFX.fadeOut(duration, onComplete)
    cancelCurrent()
    _r, _g, _b = 0, 0, 0
    _tweenId = Tween.new(_alpha, 0, duration or 0.5, "linear",
        function(v) _alpha = v end,
        function()
            _alpha   = 0
            _tweenId = nil
            if onComplete then onComplete() end
        end)
end

function SceneFX.transition(fn, duration)
    SceneFX.fadeIn(duration, function()
        if fn then fn() end
        SceneFX.fadeOut(duration)
    end)
end

function SceneFX.flash(color, duration)
    cancelCurrent()
    local c = color or Color.White
    _r, _g, _b = c.r, c.g, c.b
    _alpha = c.a or 200
    _tweenId = Tween.new(_alpha, 0, duration or 0.3, "easeOut",
        function(v) _alpha = v end,
        function()
            _alpha   = 0
            _tweenId = nil
        end)
end

function SceneFX.isActive()
    return _tweenId ~= nil or _alpha > 0
end

return SceneFX
