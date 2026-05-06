-- nova/camera.lua
-- Contrôle de la caméra — follow, shake, zoom, moveTo
-- Utilise le binding C++ Viewport (accès direct au moteur).
-- Disponible sans require : Camera est un global auto-chargé.
--
-- API :
--   Camera.follow(entity)           -- suit une entité chaque frame (nil = libère)
--   Camera.unfollow()               -- arrête le suivi
--   Camera.setPosition(x, y)        -- téléporte la caméra (désactive le follow)
--   Camera.getPosition()            -- Vec2f {x, y}
--   Camera.moveTo(x, y, dur, ease)  -- déplacement fluide via Tween
--   Camera.shake(duration, force)   -- tremblement d'écran (ex: explosion)
--   Camera.setZoom(scale)           -- 1.0 = normal, 2.0 = zoomé x2, 0.5 = dézoomé
--   Camera.getZoom()                -- scale courant
--   Camera.zoomTo(scale, dur, ease) -- zoom fluide via Tween
--   Camera.reset()                  -- rétablit zoom par défaut, libère le follow
--
-- Exemple :
--   -- Suit le joueur avec légère vibration à l'impact
--   Camera.follow(Game.getPlayer())
--
--   -- Cutscène : zoom sur le boss puis revient
--   Camera.moveTo(boss:getPosition().x, boss:getPosition().y, 2.0, "easeInOut")
--   Timer.after(3.0, function()
--       Camera.follow(Game.getPlayer())
--       Camera.zoomTo(1.0, 1.0, "easeOut")
--   end)
--
--   -- Tremblement à l'explosion
--   EventBus.on("explosion", function(data)
--       Camera.shake(0.4, 12)
--   end)

local Camera = {}

local _following    = nil
local _shakeTime    = 0
local _shakeDur     = 0
local _shakeForce   = 0
local _offsetX      = 0
local _offsetY      = 0
local _defaultW     = nil
local _defaultH     = nil

-- Appelé automatiquement par ScriptSystem chaque frame
function Camera._update(dt)
    -- Capture la taille de référence au premier frame
    if not _defaultW then
        local sz = Viewport.getSize()
        _defaultW, _defaultH = sz.x, sz.y
    end

    -- Tremblement d'écran
    if _shakeTime > 0 then
        _shakeTime = math.max(0, _shakeTime - dt)
        local decay = (_shakeDur > 0) and (_shakeTime / _shakeDur) or 0
        local amt   = _shakeForce * decay
        _offsetX = (math.random() * 2 - 1) * amt
        _offsetY = (math.random() * 2 - 1) * amt
    else
        _offsetX, _offsetY = 0, 0
    end

    -- Suivi d'entité
    if _following then
        local pos = _following:getPosition()
        Viewport.setCenter(pos.x + _offsetX, pos.y + _offsetY)
    elseif (_offsetX ~= 0 or _offsetY ~= 0) then
        local c = Viewport.getCenter()
        Viewport.setCenter(c.x + _offsetX, c.y + _offsetY)
    end
end

function Camera.follow(e)
    _following = e
end

function Camera.unfollow()
    _following = nil
end

function Camera.getPosition()
    return Viewport.getCenter()
end

function Camera.setPosition(x, y)
    _following = nil
    Viewport.setCenter(x, y)
end

-- Déplacement fluide via Tween
function Camera.moveTo(x, y, duration, easing)
    _following = nil
    local c = Viewport.getCenter()
    Tween.newVec2(
        { x = c.x, y = c.y },
        { x = x,   y = y   },
        duration or 1.0,
        easing or "easeOut",
        function(v) Viewport.setCenter(v.x, v.y) end
    )
end

-- Tremblement d'écran
function Camera.shake(duration, force)
    _shakeDur   = duration or 0.4
    _shakeTime  = _shakeDur
    _shakeForce = force or 8
end

function Camera.setZoom(scale)
    if not _defaultW then return end
    Viewport.setSize(_defaultW / scale, _defaultH / scale)
end

function Camera.getZoom()
    if not _defaultW then return 1.0 end
    return _defaultW / Viewport.getSize().x
end

-- Zoom fluide via Tween
function Camera.zoomTo(targetScale, duration, easing)
    local currentZoom = Camera.getZoom()
    Tween.new(currentZoom, targetScale, duration or 1.0, easing or "easeOut",
        function(z) Camera.setZoom(z) end)
end

-- Remet à zéro (zoom + follow)
function Camera.reset()
    _following = nil
    if _defaultW then
        Viewport.setSize(_defaultW, _defaultH)
    end
end

return Camera
