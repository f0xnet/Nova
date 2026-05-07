-- nova/math_ext.lua
-- Utilitaires mathématiques complémentaires à la stdlib Lua.
-- Disponible sans require : Math est un global auto-chargé.
--
-- API :
--   Math.lerp(a, b, t)               -- interpolation linéaire
--   Math.clamp(v, min, max)          -- borne une valeur
--   Math.smoothstep(a, b, t)         -- interpolation en S (douce)
--   Math.sign(v)                     -- -1, 0 ou 1
--   Math.chance(p)                   -- true avec probabilité p (0..1)
--   Math.randomRange(min, max)       -- nombre aléatoire dans [min, max]
--   Math.pingpong(t, length)         -- rebond 0→length→0
--   Math.approach(cur, target, step) -- avance vers target de step max
--   Math.map(v, inMin, inMax, outMin, outMax) -- remapping de plage
--   Math.round(v, decimals)          -- arrondi à N décimales
--   Math.deg2rad(deg)                -- degrés → radians
--   Math.rad2deg(rad)                -- radians → degrés

local Math = {}

function Math.lerp(a, b, t)
    return a + (b - a) * t
end

function Math.clamp(v, min, max)
    return math.max(min, math.min(max, v))
end

-- Hermite smoothstep : transition en S entre 0 et 1
function Math.smoothstep(edge0, edge1, t)
    t = Math.clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0)
    return t * t * (3 - 2 * t)
end

-- Smoother step (quintic) — encore plus doux
function Math.smootherstep(edge0, edge1, t)
    t = Math.clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0)
    return t * t * t * (t * (t * 6 - 15) + 10)
end

function Math.sign(v)
    if v > 0 then return 1
    elseif v < 0 then return -1
    else return 0 end
end

-- Retourne true avec une probabilité de p (0.0–1.0)
function Math.chance(p)
    return math.random() < p
end

-- Entier si les bornes sont entières, float sinon
function Math.randomRange(min, max)
    if math.floor(min) == min and math.floor(max) == max then
        return math.random(min, max)
    end
    return min + math.random() * (max - min)
end

-- Oscille entre 0 et length : 0→length→0→length…
function Math.pingpong(t, length)
    t = t % (length * 2)
    return length - math.abs(t - length)
end

-- Avance de current vers target d'un pas maximum de delta
function Math.approach(current, target, delta)
    if current < target then
        return math.min(current + delta, target)
    else
        return math.max(current - delta, target)
    end
end

-- Remapping de plage : [inMin, inMax] → [outMin, outMax]
function Math.map(value, inMin, inMax, outMin, outMax)
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin)
end

function Math.round(v, decimals)
    local mult = 10 ^ (decimals or 0)
    return math.floor(v * mult + 0.5) / mult
end

function Math.deg2rad(deg)  return deg * (math.pi / 180.0) end
function Math.rad2deg(rad)  return rad * (180.0 / math.pi) end

-- Distance d'un point à un segment 1D [min, max] (0 si dans le segment)
function Math.distToRange(v, min, max)
    if v < min then return v - min
    elseif v > max then return v - max
    else return 0 end
end

-- Normalise un angle en degrés dans [0, 360)
function Math.normalizeAngle(deg)
    return deg % 360
end

-- Angle entre deux positions (en degrés)
function Math.angleTo(x1, y1, x2, y2)
    return Math.rad2deg(math.atan(y2 - y1, x2 - x1))
end

return Math
