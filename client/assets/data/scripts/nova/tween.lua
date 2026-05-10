-- nova/tween.lua
-- Interpolation fluide entre valeurs — position, alpha, couleur, etc.
-- Disponible sans require : Tween est un global auto-chargé.
--
-- API :
--   local id = Tween.new(from, to, duration, easing, onUpdate, onComplete)
--   local id = Tween.newVec2(fromVec2, toVec2, duration, easing, onUpdate, onComplete)
--   Tween.cancel(id)
--   Tween.update(dt)   -- appelé automatiquement par ScriptSystem
--
-- Easings : "linear", "easeIn", "easeOut", "easeInOut",
--           "easeInCubic", "easeOutCubic", "bounce"
--
-- Exemple :
--   -- Déplace une entité de (0,0) à (200,100) en 1 seconde
--   Tween.newVec2({x=0,y=0}, {x=200,y=100}, 1.0, "easeOut",
--       function(v)
--           local t = entity:getTransform()
--           if t then t.position.x = v.x; t.position.y = v.y end
--       end)

local Tween = {}
local tweens = {}
local nextId = 1

local easings = {
    linear       = function(t) return t end,
    easeIn       = function(t) return t * t end,
    easeOut      = function(t) return 1 - (1 - t) * (1 - t) end,
    easeInOut    = function(t)
        return t < 0.5 and 2*t*t or 1 - (-2*t + 2)^2 / 2
    end,
    easeInCubic  = function(t) return t * t * t end,
    easeOutCubic = function(t) return 1 - (1 - t)^3 end,
    bounce       = function(t)
        local n1, d1 = 7.5625, 2.75
        if     t < 1 / d1       then return n1 * t * t
        elseif t < 2 / d1       then t = t - 1.5  / d1; return n1*t*t + 0.75
        elseif t < 2.5 / d1     then t = t - 2.25 / d1; return n1*t*t + 0.9375
        else                         t = t - 2.625 / d1; return n1*t*t + 0.984375
        end
    end,
}

-- from/to : number ou table {x,y} ou toute table avec des champs numériques
-- onUpdate(currentValue) est appelé chaque frame avec la valeur interpolée
-- onComplete() est appelé à la fin (optionnel)
function Tween.new(from, to, duration, easing, onUpdate, onComplete)
    local id = nextId; nextId = nextId + 1
    tweens[id] = {
        from = from, to = to,
        duration = math.max(duration, 0.0001),
        elapsed  = 0,
        fn       = easings[easing] or easings.linear,
        onUpdate = onUpdate,
        onComplete = onComplete,
    }
    return id
end

-- Raccourci pour les Vec2f / tables {x, y}
function Tween.newVec2(from, to, duration, easing, onUpdate, onComplete)
    return Tween.new(
        {x = from.x, y = from.y},
        {x = to.x,   y = to.y},
        duration, easing, onUpdate, onComplete
    )
end

function Tween.cancel(id)
    tweens[id] = nil
end

function Tween.update(dt)
    local completed = {}
    for id, tw in pairs(tweens) do
        tw.elapsed = tw.elapsed + dt
        local t = math.min(tw.elapsed / tw.duration, 1.0)
        local v = tw.fn(t)

        -- Calcule la valeur courante
        local current
        if type(tw.from) == "number" then
            current = tw.from + (tw.to - tw.from) * v
        else
            current = {}
            for k, fromVal in pairs(tw.from) do
                current[k] = fromVal + (tw.to[k] - fromVal) * v
            end
        end

        local ok, err = pcall(tw.onUpdate, current)
        if not ok then
            Log.error("Tween.update: " .. tostring(err))
            tweens[id] = nil  -- tue le tween plutôt que de spam-logguer chaque frame
            goto continue
        end

        if t >= 1.0 then
            tweens[id] = nil
            if tw.onComplete then
                table.insert(completed, tw.onComplete)
            end
        end
        ::continue::
    end
    -- Appelle les onComplete APRÈS l'itération pour éviter de modifier `tweens`
    -- pendant pairs() — ce qui causerait "invalid key to 'next'".
    for _, fn in ipairs(completed) do
        local ok, err = pcall(fn)
        if not ok then Log.error("Tween.onComplete: " .. tostring(err)) end
    end
end

return Tween
