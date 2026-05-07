-- nova/easing.lua
-- Standalone easing library — complete Robert Penner functions.
-- No dependencies. Available without require: Easing is an auto-loaded global.
--
-- API:
--   Easing.linear(t)
--
--   Easing.inQuad(t)        Easing.outQuad(t)        Easing.inOutQuad(t)
--   Easing.inCubic(t)       Easing.outCubic(t)       Easing.inOutCubic(t)
--   Easing.inQuart(t)       Easing.outQuart(t)        Easing.inOutQuart(t)
--   Easing.inQuint(t)       Easing.outQuint(t)        Easing.inOutQuint(t)
--   Easing.inSine(t)        Easing.outSine(t)         Easing.inOutSine(t)
--   Easing.inExpo(t)        Easing.outExpo(t)         Easing.inOutExpo(t)
--   Easing.inCirc(t)        Easing.outCirc(t)         Easing.inOutCirc(t)
--   Easing.inElastic(t)     Easing.outElastic(t)      Easing.inOutElastic(t)
--   Easing.inBack(t)        Easing.outBack(t)         Easing.inOutBack(t)
--   Easing.inBounce(t)      Easing.outBounce(t)       Easing.inOutBounce(t)
--
--   Easing.apply(name, t)          -- apply easing function by string name, t ∈ [0,1]
--   Easing.get(name)               -- returns the function by name (falls back to linear)
--   Easing.lerp(a, b, t, name)     -- lerp with easing (name defaults to "linear")
--
-- All functions clamp t to [0, 1].
--
-- Examples:
--   local y = Easing.lerp(0, 100, elapsed / duration, "inOutCubic")
--
--   local fn = Easing.get("outBounce")
--   local v  = fn(t)
--
--   local v2 = Easing.apply("inOutElastic", 0.75)

local Easing = {}

-- ─── Internal utilities ───────────────────────────────────────────────────────

local pi   = math.pi
local sin  = math.sin
local cos  = math.cos
local sqrt = math.sqrt
local pow  = function(b, e) return b ^ e end

local function clamp(t)
    return t < 0 and 0 or (t > 1 and 1 or t)
end

-- ─── linear ───────────────────────────────────────────────────────────────────

function Easing.linear(t)
    t = clamp(t)
    return t
end

-- ─── quad ─────────────────────────────────────────────────────────────────────

function Easing.inQuad(t)
    t = clamp(t)
    return t * t
end

function Easing.outQuad(t)
    t = clamp(t)
    return 1 - (1 - t) * (1 - t)
end

function Easing.inOutQuad(t)
    t = clamp(t)
    if t < 0.5 then
        return 2 * t * t
    else
        return 1 - pow(-2 * t + 2, 2) / 2
    end
end

-- ─── cubic ────────────────────────────────────────────────────────────────────

function Easing.inCubic(t)
    t = clamp(t)
    return t * t * t
end

function Easing.outCubic(t)
    t = clamp(t)
    return 1 - pow(1 - t, 3)
end

function Easing.inOutCubic(t)
    t = clamp(t)
    if t < 0.5 then
        return 4 * t * t * t
    else
        return 1 - pow(-2 * t + 2, 3) / 2
    end
end

-- ─── quart ────────────────────────────────────────────────────────────────────

function Easing.inQuart(t)
    t = clamp(t)
    return t * t * t * t
end

function Easing.outQuart(t)
    t = clamp(t)
    return 1 - pow(1 - t, 4)
end

function Easing.inOutQuart(t)
    t = clamp(t)
    if t < 0.5 then
        return 8 * t * t * t * t
    else
        return 1 - pow(-2 * t + 2, 4) / 2
    end
end

-- ─── quint ────────────────────────────────────────────────────────────────────

function Easing.inQuint(t)
    t = clamp(t)
    return pow(t, 5)
end

function Easing.outQuint(t)
    t = clamp(t)
    return 1 - pow(1 - t, 5)
end

function Easing.inOutQuint(t)
    t = clamp(t)
    if t < 0.5 then
        return 16 * pow(t, 5)
    else
        return 1 - pow(-2 * t + 2, 5) / 2
    end
end

-- ─── sine ─────────────────────────────────────────────────────────────────────

function Easing.inSine(t)
    t = clamp(t)
    return 1 - cos(t * pi / 2)
end

function Easing.outSine(t)
    t = clamp(t)
    return sin(t * pi / 2)
end

function Easing.inOutSine(t)
    t = clamp(t)
    return -(cos(pi * t) - 1) / 2
end

-- ─── expo ─────────────────────────────────────────────────────────────────────

function Easing.inExpo(t)
    t = clamp(t)
    if t == 0 then return 0 end
    return pow(2, 10 * t - 10)
end

function Easing.outExpo(t)
    t = clamp(t)
    if t == 1 then return 1 end
    return 1 - pow(2, -10 * t)
end

function Easing.inOutExpo(t)
    t = clamp(t)
    if t == 0 then return 0 end
    if t == 1 then return 1 end
    if t < 0.5 then
        return pow(2, 20 * t - 10) / 2
    else
        return (2 - pow(2, -20 * t + 10)) / 2
    end
end

-- ─── circ ─────────────────────────────────────────────────────────────────────

function Easing.inCirc(t)
    t = clamp(t)
    return 1 - sqrt(1 - t * t)
end

function Easing.outCirc(t)
    t = clamp(t)
    return sqrt(1 - pow(t - 1, 2))
end

function Easing.inOutCirc(t)
    t = clamp(t)
    if t < 0.5 then
        return (1 - sqrt(1 - pow(2 * t, 2))) / 2
    else
        return (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2
    end
end

-- ─── elastic ──────────────────────────────────────────────────────────────────

local ELASTIC_C4 = (2 * pi) / 3
local ELASTIC_C5 = (2 * pi) / 4.5

function Easing.inElastic(t)
    t = clamp(t)
    if t == 0 then return 0 end
    if t == 1 then return 1 end
    return -pow(2, 10 * t - 10) * sin((t * 10 - 10.75) * ELASTIC_C4)
end

function Easing.outElastic(t)
    t = clamp(t)
    if t == 0 then return 0 end
    if t == 1 then return 1 end
    return pow(2, -10 * t) * sin((t * 10 - 0.75) * ELASTIC_C4) + 1
end

function Easing.inOutElastic(t)
    t = clamp(t)
    if t == 0 then return 0 end
    if t == 1 then return 1 end
    if t < 0.5 then
        return -(pow(2, 20 * t - 10) * sin((20 * t - 11.125) * ELASTIC_C5)) / 2
    else
        return  (pow(2, -20 * t + 10) * sin((20 * t - 11.125) * ELASTIC_C5)) / 2 + 1
    end
end

-- ─── back ─────────────────────────────────────────────────────────────────────

local BACK_C1 = 1.70158
local BACK_C2 = BACK_C1 * 1.525
local BACK_C3 = BACK_C1 + 1

function Easing.inBack(t)
    t = clamp(t)
    return BACK_C3 * t * t * t - BACK_C1 * t * t
end

function Easing.outBack(t)
    t = clamp(t)
    return 1 + BACK_C3 * pow(t - 1, 3) + BACK_C1 * pow(t - 1, 2)
end

function Easing.inOutBack(t)
    t = clamp(t)
    if t < 0.5 then
        return (pow(2 * t, 2) * ((BACK_C2 + 1) * 2 * t - BACK_C2)) / 2
    else
        return (pow(2 * t - 2, 2) * ((BACK_C2 + 1) * (2 * t - 2) + BACK_C2) + 2) / 2
    end
end

-- ─── bounce ───────────────────────────────────────────────────────────────────

local BOUNCE_N1 = 7.5625
local BOUNCE_D1 = 2.75

local function outBounceRaw(t)
    if t < 1 / BOUNCE_D1 then
        return BOUNCE_N1 * t * t
    elseif t < 2 / BOUNCE_D1 then
        t = t - 1.5 / BOUNCE_D1
        return BOUNCE_N1 * t * t + 0.75
    elseif t < 2.5 / BOUNCE_D1 then
        t = t - 2.25 / BOUNCE_D1
        return BOUNCE_N1 * t * t + 0.9375
    else
        t = t - 2.625 / BOUNCE_D1
        return BOUNCE_N1 * t * t + 0.984375
    end
end

function Easing.outBounce(t)
    return outBounceRaw(clamp(t))
end

function Easing.inBounce(t)
    t = clamp(t)
    return 1 - outBounceRaw(1 - t)
end

function Easing.inOutBounce(t)
    t = clamp(t)
    if t < 0.5 then
        return (1 - outBounceRaw(1 - 2 * t)) / 2
    else
        return (1 + outBounceRaw(2 * t - 1)) / 2
    end
end

-- ─── Lookup table ─────────────────────────────────────────────────────────────

local _registry = {
    linear        = Easing.linear,

    inQuad        = Easing.inQuad,
    outQuad       = Easing.outQuad,
    inOutQuad     = Easing.inOutQuad,

    inCubic       = Easing.inCubic,
    outCubic      = Easing.outCubic,
    inOutCubic    = Easing.inOutCubic,

    inQuart       = Easing.inQuart,
    outQuart      = Easing.outQuart,
    inOutQuart    = Easing.inOutQuart,

    inQuint       = Easing.inQuint,
    outQuint      = Easing.outQuint,
    inOutQuint    = Easing.inOutQuint,

    inSine        = Easing.inSine,
    outSine       = Easing.outSine,
    inOutSine     = Easing.inOutSine,

    inExpo        = Easing.inExpo,
    outExpo       = Easing.outExpo,
    inOutExpo     = Easing.inOutExpo,

    inCirc        = Easing.inCirc,
    outCirc       = Easing.outCirc,
    inOutCirc     = Easing.inOutCirc,

    inElastic     = Easing.inElastic,
    outElastic    = Easing.outElastic,
    inOutElastic  = Easing.inOutElastic,

    inBack        = Easing.inBack,
    outBack       = Easing.outBack,
    inOutBack     = Easing.inOutBack,

    inBounce      = Easing.inBounce,
    outBounce     = Easing.outBounce,
    inOutBounce   = Easing.inOutBounce,
}

-- ─── Public helpers ───────────────────────────────────────────────────────────

--- Returns the easing function for `name`, or Easing.linear if unknown.
function Easing.get(name)
    return _registry[name] or _registry.linear
end

--- Applies the easing function `name` to t (t ∈ [0,1]).
function Easing.apply(name, t)
    return Easing.get(name)(t)
end

--- Linear interpolation between `a` and `b` with easing `name` applied to t.
--- `name` is optional and defaults to "linear".
function Easing.lerp(a, b, t, name)
    local v = Easing.apply(name or "linear", t)
    return a + (b - a) * v
end

return Easing
