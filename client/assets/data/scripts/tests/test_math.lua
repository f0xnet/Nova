-- tests/test_math.lua
-- Tests : Math · Table · Random · Easing · Color (extensions)

local T = require("tests/assert")
T.reset()

-- ── Math ──────────────────────────────────────────────────────────────────────

T.suite("Math")

T.near("lerp 0.5",   5,  Math.lerp(0, 10, 0.5))
T.near("lerp 0",     0,  Math.lerp(0, 10, 0))
T.near("lerp 1",    10,  Math.lerp(0, 10, 1))
T.near("clamp lo",   0,  Math.clamp(-5, 0, 10))
T.near("clamp hi",  10,  Math.clamp(15, 0, 10))
T.near("clamp mid",  5,  Math.clamp(5, 0, 10))
T.near("smoothstep 0", 0, Math.smoothstep(0, 1, 0))
T.near("smoothstep 1", 1, Math.smoothstep(0, 1, 1))
T.near("smoothstep 0.5", 0.5, Math.smoothstep(0, 1, 0.5), 0.01)
T.eq("sign pos",  1,  Math.sign(7))
T.eq("sign neg", -1,  Math.sign(-3))
T.eq("sign zero", 0, Math.sign(0))
T.near("map 0.5→5", 5, Math.map(0.5, 0, 1, 0, 10))
T.near("map 0→0",   0, Math.map(0,   0, 1, 0, 10))
T.near("map 1→10", 10, Math.map(1,   0, 1, 0, 10))
T.near("round up",  3, Math.round(2.6))
T.near("round dn",  2, Math.round(2.4))
T.near("deg2rad",   math.pi,  Math.deg2rad(180), 1e-5)
T.near("rad2deg",   180,      Math.rad2deg(math.pi), 1e-3)
T.near("approach exact",     5, Math.approach(3, 5, 10))
T.near("approach step",      4, Math.approach(3, 5, 1))
T.near("approach neg step",  2, Math.approach(3, 1, 1))
T.near("pingpong mid",  0.5, Math.pingpong(0.5, 1))
T.near("pingpong wrap", 0.5, Math.pingpong(1.5, 1))
do
    local v = Math.randomRange(5, 10)
    T.check("randomRange in [5,10]", v >= 5 and v <= 10)
    local i = Math.randomRange(1, 1)
    T.eq("randomRange equal bounds", 1, i)
end
T.near("distToRange inside",   0,  Math.distToRange(5, 0, 10))
T.near("distToRange below",   -5,  Math.distToRange(-5, 0, 10))
T.near("distToRange above",    5,  Math.distToRange(15, 0, 10))
T.near("normalizeAngle 370",  10,  Math.normalizeAngle(370))
T.near("normalizeAngle neg",  350, Math.normalizeAngle(-10))
T.near("angleTo east",         0,  Math.angleTo(0,0,1,0), 0.01)
T.near("angleTo north",       90,  Math.angleTo(0,0,0,1), 0.01)

-- ── Table ─────────────────────────────────────────────────────────────────────

T.suite("Table")

local seq = {10, 20, 30, 40, 50}

T.check("contains yes",    Table.contains(seq, 30))
T.check("contains no",     not Table.contains(seq, 99))
local fv, fi = Table.find(seq, function(v) return v == 30 end)
T.eq("find value",  30, fv)
T.eq("find index",  3,  fi)
local mv, mi = Table.find(seq, function(v) return v == 99 end)
T.isNil("find missing val",   mv)
T.isNil("find missing index", mi)

local fil = Table.filter(seq, function(v) return v > 25 end)
T.eq("filter count",  3, #fil)
T.eq("filter first", 30, fil[1])

local m = Table.map({1, 2, 3}, function(v) return v * v end)
T.eq("map[1]", 1, m[1])
T.eq("map[2]", 4, m[2])
T.eq("map[3]", 9, m[3])

local sum = Table.reduce({1, 2, 3, 4, 5}, function(acc, v) return acc + v end, 0)
T.eq("reduce sum", 15, sum)

T.eq("count seq",  5, Table.count(seq))
T.eq("count dict", 3, Table.count(Table.keys({a=1, b=2, c=3})))

local rev = Table.reverse({1, 2, 3})
T.eq("reverse[1]", 3, rev[1])
T.eq("reverse[3]", 1, rev[3])

local u = Table.unique({1, 2, 2, 3, 3, 3})
T.eq("unique count", 3, #u)

local mg = Table.merge({a=1, b=2}, {b=9, c=3})
T.eq("merge existing", 9, mg.b)
T.eq("merge new",      3, mg.c)
T.eq("merge kept",     1, mg.a)

local flat = Table.flatten({{1, 2}, {3, {4, 5}}})
T.eq("flatten count", 5, #flat)
T.eq("flatten[4]",    4, flat[4])

local g = Table.groupBy({1, 2, 3, 4, 5}, function(v) return v % 2 == 0 and "even" or "odd" end)
T.eq("groupBy even count", 2, #g["even"])
T.eq("groupBy odd count",  3, #g["odd"])

do
    local orig = {x=1, y=2}
    local cp   = Table.copy(orig)
    cp.x = 99
    T.eq("copy independent",  1, orig.x)
    T.eq("copy has values",   1, (function() local c2 = Table.copy({x=1,y=2}); return c2.x end)())
end

-- ── Random ────────────────────────────────────────────────────────────────────

T.suite("Random")

Random.seed(12345)

local ri = Random.int(1, 6)
T.check("int in [1,6]", ri >= 1 and ri <= 6)

local rf = Random.float(0.0, 1.0)
T.check("float in [0,1]", rf >= 0.0 and rf <= 1.0)

local rf2 = Random.float(5.0, 10.0)
T.check("float in [5,10]", rf2 >= 5.0 and rf2 <= 10.0)

local pk = Random.pick({"alpha", "beta", "gamma"})
T.check("pick not nil", pk ~= nil)
T.check("pick is string", type(pk) == "string")

local trues = 0
for _ = 1, 200 do if Random.chance(0.5) then trues = trues + 1 end end
T.check("chance(0.5) is ~50% (between 70-130)", trues >= 70 and trues <= 130)

local all_false = true
for _ = 1, 50 do if Random.chance(0) then all_false = false end end
T.check("chance(0) never true", all_false)

local all_true = true
for _ = 1, 50 do if not Random.chance(1) then all_true = false end end
T.check("chance(1) always true", all_true)

local d6 = Random.dice(6)
T.check("dice d6 in [1,6]", d6 >= 1 and d6 <= 6)

local d3x2 = Random.dice(3, 6)
T.check("dice 3d6 in [3,18]", d3x2 >= 3 and d3x2 <= 18)

-- weighted
local w = Random.weighted({
    { value = "common", weight = 90 },
    { value = "rare",   weight = 10 },
})
T.check("weighted returns value", w == "common" or w == "rare")

-- gaussian
local g = Random.gaussian(0, 1)
T.check("gaussian returns number", type(g) == "number")
-- 99.7% of values within ±4 sigma for mean=0, std=1
local extremes = 0
for _ = 1, 100 do
    if math.abs(Random.gaussian(0, 1)) > 5 then extremes = extremes + 1 end
end
T.check("gaussian: almost never extreme", extremes <= 2)

-- shuffle
local orig = {1, 2, 3, 4, 5}
local sh = Random.shuffle(orig)
T.eq("shuffle preserves length", 5, #sh)
T.check("shuffle doesn't modify original", orig[1] == 1)

-- Reseed produces same sequence
Random.seed(999)
local a1 = Random.int(1, 1000)
Random.seed(999)
local a2 = Random.int(1, 1000)
T.eq("same seed → same result", a1, a2)

-- ── Easing ────────────────────────────────────────────────────────────────────

T.suite("Easing")

T.near("linear(0)",   0, Easing.linear(0))
T.near("linear(1)",   1, Easing.linear(1))
T.near("linear(0.5)", 0.5, Easing.linear(0.5))

T.near("inQuad(0.5)",  0.25, Easing.inQuad(0.5))
T.near("outQuad(0.5)", 0.75, Easing.outQuad(0.5))
T.near("inQuad(0)",    0,    Easing.inQuad(0))
T.near("inQuad(1)",    1,    Easing.inQuad(1))

T.near("inCubic(0.5)",  0.125, Easing.inCubic(0.5))
T.near("outCubic(0.5)", 0.875, Easing.outCubic(0.5))

-- Boundary clamping
T.near("inQuad clamped at -1", 0, Easing.inQuad(-1))
T.near("inQuad clamped at  2", 1, Easing.inQuad(2))

-- apply dispatcher
T.near("apply linear",  0.5,  Easing.apply("linear",  0.5))
T.near("apply inQuad",  0.25, Easing.apply("inQuad",  0.5))
T.near("apply outQuad", 0.75, Easing.apply("outQuad", 0.5))

-- get function
local fn = Easing.get("inCubic")
T.check("get returns function", type(fn) == "function")
T.near("get function result", 0.125, fn(0.5))

-- unknown name falls back to linear
local fallback = Easing.get("__nonexistent__")
T.check("get unknown returns function", type(fallback) == "function")
T.near("get unknown is linear", 0.5, fallback(0.5))

-- lerp with easing
T.near("lerp linear",  5.0, Easing.lerp(0, 10, 0.5, "linear"))
T.near("lerp inQuad", 2.5, Easing.lerp(0, 10, 0.5, "inQuad"))

-- Bounce is defined
T.check("outBounce at 1 = 1", math.abs(Easing.outBounce(1) - 1) < 0.01)
T.near("outBounce at 0", 0, Easing.outBounce(0))

-- ── Color extensions ──────────────────────────────────────────────────────────

T.suite("Color_ext")

local red   = Color.new(255, 0, 0, 255)
local blue  = Color.new(0, 0, 255, 255)

local mid = Color.lerp(red, blue, 0.5)
T.near("lerp r at 0.5", 128, mid.r, 2)
T.near("lerp g at 0.5",   0, mid.g, 2)
T.near("lerp b at 0.5", 128, mid.b, 2)

local c0 = Color.lerp(red, blue, 0)
T.eq("lerp t=0 r", 255, c0.r)
T.eq("lerp t=0 b",   0, c0.b)

local c1x = Color.lerp(red, blue, 1)
T.eq("lerp t=1 r",   0, c1x.r)
T.eq("lerp t=1 b", 255, c1x.b)

local hex = Color.fromHex("#FF8000")
T.eq("fromHex r", 255, hex.r)
T.near("fromHex g", 128, hex.g, 2)
T.eq("fromHex b",   0, hex.b)
T.eq("fromHex a", 255, hex.a)

local hexA = Color.fromHex("#FF000080")
T.eq("fromHex+alpha r", 255, hexA.r)
T.near("fromHex+alpha a", 128, hexA.a, 2)

local hexStr = Color.toHex(Color.new(255, 128, 0, 255))
T.check("toHex starts #", hexStr:sub(1,1) == "#")
T.eq("toHex length", 7, #hexStr)

local wa = Color.withAlpha(red, 0.5)
T.eq("withAlpha r", 255, wa.r)
T.near("withAlpha a", 128, wa.a, 2)

local dim = Color.multiply(Color.new(200, 100, 50, 255), 0.5)
T.near("multiply r", 100, dim.r, 2)
T.near("multiply g",  50, dim.g, 2)

local white = Color.fromHSV(0, 0, 1)
T.near("fromHSV white r", 255, white.r, 2)
T.near("fromHSV white g", 255, white.g, 2)
T.near("fromHSV white b", 255, white.b, 2)

local orange = Color.fromHSV(30, 1, 1)
T.eq("fromHSV orange r", 255, orange.r)
T.eq("fromHSV orange b",   0, orange.b)

return T.summary()
