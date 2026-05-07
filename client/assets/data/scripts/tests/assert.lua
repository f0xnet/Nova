-- tests/assert.lua
-- Micro framework d'assertion pour les tests Nova.
--
-- Usage :
--   local T = require("tests/assert")
--   T.suite("MonModule")
--   T.check("flag is set", Flag.get("x"))
--   T.eq("count is 5", 5, Inventory.count(1, "coin"))
--   T.near("lerp mid", 0.5, Math.lerp(0, 1, 0.5))
--   return T.summary()   -- retourne { pass, fail }

local T = {}

local _pass  = 0
local _fail  = 0
local _suite = "?"

function T.reset()
    _pass  = 0
    _fail  = 0
    _suite = "?"
end

function T.suite(name)
    _suite = name
    Log.info("[TEST] ── " .. name .. " ──")
end

function T.check(desc, cond)
    if cond then
        _pass = _pass + 1
    else
        _fail = _fail + 1
        Log.warn("[TEST] FAIL  [" .. _suite .. "] " .. desc)
    end
end

function T.eq(desc, expected, actual)
    local ok = (expected == actual)
    if not ok then
        desc = desc .. "  expected=" .. tostring(expected) .. "  got=" .. tostring(actual)
    end
    T.check(desc, ok)
end

function T.near(desc, expected, actual, tol)
    tol = tol or 1e-4
    local ok = type(actual) == "number" and math.abs(actual - expected) <= tol
    if not ok then
        desc = desc .. "  expected≈" .. tostring(expected) .. "  got=" .. tostring(actual)
    end
    T.check(desc, ok)
end

function T.notNil(desc, val)
    T.check(desc, val ~= nil)
end

function T.isNil(desc, val)
    T.check(desc, val == nil)
end

function T.summary()
    local total = _pass + _fail
    if _fail == 0 then
        Log.info(string.format("[TEST] ALL PASS  %d/%d", _pass, total))
    else
        Log.warn(string.format("[TEST] FAILURES  %d pass / %d fail  (total %d)", _pass, _fail, total))
    end
    return { pass = _pass, fail = _fail }
end

return T
