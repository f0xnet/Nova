-- tests/test_core.lua
-- Tests : EventBus · Timer · Scheduler · Tween
-- Dépendances : tous auto-chargés comme globals.

local T = require("tests/assert")
T.reset()

-- ── EventBus ──────────────────────────────────────────────────────────────────

T.suite("EventBus")

do
    local got = nil
    local function handler(d) got = d.v end
    EventBus.on("_t_basic", handler)
    EventBus.emit("_t_basic", { v = 7 })
    T.eq("on/emit value", 7, got)

    got = nil
    EventBus.off("_t_basic", handler)
    EventBus.emit("_t_basic", { v = 99 })
    T.check("off stops handler", got == nil)
end

do
    local n = 0
    local function h() n = n + 1 end
    EventBus.once("_t_once", h)
    EventBus.emit("_t_once", {})
    EventBus.emit("_t_once", {})
    T.eq("once fires once", 1, n)
end

do
    local n = 0
    local function h1() n = n + 1 end
    local function h2() n = n + 1 end
    EventBus.on("_t_clear", h1)
    EventBus.on("_t_clear", h2)
    EventBus.clear("_t_clear")
    EventBus.emit("_t_clear", {})
    T.eq("clear removes all", 0, n)
end

do
    local order = {}
    EventBus.on("_t_order", function() table.insert(order, 1) end)
    EventBus.on("_t_order", function() table.insert(order, 2) end)
    EventBus.emit("_t_order", {})
    T.eq("handlers fired in order 1", 1, order[1])
    T.eq("handlers fired in order 2", 2, order[2])
    EventBus.clear("_t_order")
end

do
    local got = nil
    EventBus.on("_t_nil", function(d) got = d end)
    EventBus.emit("_t_nil")
    T.check("emit without data passes nil", got == nil)
    EventBus.clear("_t_nil")
end

-- ── Timer ─────────────────────────────────────────────────────────────────────

T.suite("Timer")

do
    local fired = false
    Timer.after(0.1, function() fired = true end)
    Timer.update(0.05)
    T.check("after: not yet at 0.05s", not fired)
    Timer.update(0.06)
    T.check("after: fired at 0.11s", fired)
end

do
    local n = 0
    local id = Timer.every(0.1, function() n = n + 1 end)
    -- Timer.update fait une seule passe par appel ; appeler 4× pour déclencher ≥3 tirs
    Timer.update(0.11)
    Timer.update(0.11)
    Timer.update(0.11)
    Timer.update(0.11)
    T.check("every: fired ≥3 times", n >= 3)
    Timer.cancel(id)
    local prev = n
    Timer.update(0.5)
    T.eq("cancel stops repeating", prev, n)
end

do
    local fired = false
    local id = Timer.after(1.0, function() fired = true end)
    Timer.cancel(id)
    Timer.update(2.0)
    T.check("cancel before fire", not fired)
end

-- ── Scheduler ─────────────────────────────────────────────────────────────────

T.suite("Scheduler")

do
    local log = {}
    Scheduler.start(function()
        table.insert(log, "a")
        Scheduler.wait(0.2)
        table.insert(log, "b")
        Scheduler.wait(0.2)
        table.insert(log, "c")
    end)
    T.eq("immediate first step", "a", log[1])
    T.check("b not yet", log[2] == nil)
    Scheduler.update(0.25)
    T.eq("b after 0.25s", "b", log[2])
    T.check("c not yet", log[3] == nil)
    Scheduler.update(0.25)
    T.eq("c after 0.5s total", "c", log[3])
end

do
    Scheduler.clear()
    local done = false
    Scheduler.start(function()
        Scheduler.wait(10)
        done = true
    end)
    Scheduler.clear()
    Scheduler.update(20)
    T.check("clear stops coroutine", not done)
end

-- ── Tween ─────────────────────────────────────────────────────────────────────

T.suite("Tween")

do
    local val = 0
    local done = false
    Tween.new(0, 100, 1.0, "linear",
        function(v) val = v end,
        function()  done = true end)

    Tween.update(0.5)
    T.near("linear tween at 50%", 50, val, 1)
    T.check("not done at 50%", not done)

    Tween.update(0.6)
    T.near("linear tween complete", 100, val, 1)
    T.check("onComplete called", done)
end

do
    local vals = {}
    Tween.newVec2({x=0, y=0}, {x=10, y=20}, 1.0, "linear",
        function(v) table.insert(vals, {x=v.x, y=v.y}) end)
    Tween.update(0.5)
    T.check("vec2 tween x ~5", #vals > 0 and math.abs(vals[#vals].x - 5) < 1)
    T.check("vec2 tween y ~10", #vals > 0 and math.abs(vals[#vals].y - 10) < 1)
    Tween.update(0.6)
end

do
    local val = 0
    local id = Tween.new(0, 50, 1.0, "linear", function(v) val = v end)
    Tween.update(0.3)
    Tween.cancel(id)
    local snapshot = val
    Tween.update(0.8)
    T.near("cancel freezes value", snapshot, val, 0.01)
end

return T.summary()
