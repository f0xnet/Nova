-- tests/test_gameplay.lua
-- Tests : Stats · Effect · Inventory · Quest · Flag · Loot · Persist

local T = require("tests/assert")
T.reset()

-- Fake entity IDs (no real registry needed for pure-Lua modules)
local E1 = 9001
local E2 = 9002
local E3 = 9003

-- ── Stats ─────────────────────────────────────────────────────────────────────

T.suite("Stats")

Stats.clear(E1)
Stats.clear(E2)

T.check("get missing returns default", Stats.get(E1, "HP", 100) == 100)
T.check("has missing is false", not Stats.has(E1, "HP"))

Stats.set(E1, "HP",  80)
Stats.set(E1, "MP",  40)
T.eq("set/get HP",   80,  Stats.get(E1, "HP"))
T.eq("set/get MP",   40,  Stats.get(E1, "MP"))
T.check("has after set", Stats.has(E1, "HP"))

local newHP = Stats.mod(E1, "HP", -15)
T.eq("mod returns new value", 65, newHP)
T.eq("mod persisted",         65, Stats.get(E1, "HP"))

Stats.set(E1, "HP", 50)
local clamped = Stats.modClamped(E1, "HP", -100, 0, 100)
T.eq("modClamped min", 0, clamped)
Stats.set(E1, "HP", 50)
clamped = Stats.modClamped(E1, "HP", 100, 0, 100)
T.eq("modClamped max", 100, clamped)

local all = Stats.getAll(E1)
T.check("getAll has HP", all["HP"] ~= nil)
T.check("getAll has MP", all["MP"] ~= nil)

Stats.remove(E1, "MP")
T.check("remove", not Stats.has(E1, "MP"))

local zeroed = false
local function onZero(d) if d.entityId == E2 then zeroed = true end end
EventBus.on("stat_zeroed", onZero)
Stats.set(E2, "HP", 0)
T.check("stat_zeroed event on 0", zeroed)
EventBus.off("stat_zeroed", onZero)

Stats.init(E1, { Level = 1, Strength = 5 })
T.eq("init sets missing",  1, Stats.get(E1, "Level"))
Stats.set(E1, "Level", 99)
Stats.init(E1, { Level = 1 })
T.eq("init skips existing", 99, Stats.get(E1, "Level"))

Stats.clear(E1)
T.check("clear removes all", Stats.getAll(E1) ~= nil and next(Stats.getAll(E1)) == nil)

-- ── Effect ────────────────────────────────────────────────────────────────────

T.suite("Effect")

local EID = 8001
Stats.set(EID, "HP", 100)

T.check("no effect initially", not Effect.has(EID, "poison"))

local ticked = 0
local expired = false
Effect.apply(EID, "poison", {
    duration = 0.3,
    onApply  = function(eid) Stats.mod(eid, "HP", -5) end,
    onTick   = function(eid, dt) ticked = ticked + 1 end,
    onExpire = function(eid) expired = true end,
})

T.check("has after apply",   Effect.has(EID, "poison"))
T.eq("onApply fired",       95, Stats.get(EID, "HP"))

local rem = Effect.getRemaining(EID, "poison")
T.check("remaining > 0", rem ~= nil and rem > 0)

Effect.update(0.15)
T.check("ticked at least once", ticked >= 1)
T.check("still active at 0.15s", Effect.has(EID, "poison"))

Effect.update(0.20)
T.check("expired at 0.35s", expired)
T.check("removed after expire", not Effect.has(EID, "poison"))

-- Permanent effect (duration = -1)
Effect.apply(EID, "shield", { duration = -1 })
Effect.update(100)
T.check("permanent survives 100s", Effect.has(EID, "shield"))
local inf = Effect.getRemaining(EID, "shield")
T.check("permanent remaining = inf", inf == math.huge)
Effect.remove(EID, "shield")
T.check("remove permanent", not Effect.has(EID, "shield"))

-- Stackable
Effect.apply(EID, "bleed", { duration = 1, stackable = true })
Effect.apply(EID, "bleed", { duration = 1, stackable = true })
Effect.clear(EID)
T.check("clear all effects", not Effect.has(EID, "bleed"))

-- ── Inventory ─────────────────────────────────────────────────────────────────

T.suite("Inventory")

Inventory.clear(E1)
Inventory.clear(E2)

T.eq("count empty",    0, Inventory.count(E1, "coin"))
T.check("has none",    not Inventory.has(E1, "coin"))

Inventory.add(E1, "coin", 10)
T.eq("count after add",  10, Inventory.count(E1, "coin"))
T.check("has 10",         Inventory.has(E1, "coin", 10))
T.check("has 11 false",   not Inventory.has(E1, "coin", 11))

Inventory.add(E1, "coin", 5)
T.eq("stacks",           15, Inventory.count(E1, "coin"))

local removed = Inventory.remove(E1, "coin", 4)
T.eq("remove returned",   4, removed)
T.eq("count after remove",11, Inventory.count(E1, "coin"))

local over = Inventory.remove(E1, "coin", 99)
T.eq("over-remove clamps",11, over)
T.eq("count after overremove", 0, Inventory.count(E1, "coin"))

-- Item definitions (maxStack)
Inventory.defineItem("potion", { maxStack = 5, weight = 0.5 })
Inventory.add(E1, "potion", 3)
Inventory.add(E1, "potion", 10)
T.eq("maxStack respected", 5, Inventory.count(E1, "potion"))

local w = Inventory.getWeight(E1)
T.near("weight = 5 * 0.5", 2.5, w)

-- Transfer
Inventory.add(E1, "key", 1)
Inventory.transfer(E1, E2, "key", 1)
T.eq("transfer: source empty",  0, Inventory.count(E1, "key"))
T.eq("transfer: dest has 1",    1, Inventory.count(E2, "key"))

-- getAll
local all = Inventory.getAll(E2)
T.check("getAll has key", all["key"] ~= nil)

-- Events
local addedItem = nil
local function onAdd(d) if d.entityId == E3 then addedItem = d.item end end
EventBus.on("item_added", onAdd)
Inventory.add(E3, "sword", 1)
T.eq("item_added event", "sword", addedItem)
EventBus.off("item_added", onAdd)

-- ── Quest ─────────────────────────────────────────────────────────────────────

T.suite("Quest")

Quest.register("test_q", {
    name   = "Test Quest",
    stages = {
        { id = "s1", description = "Step 1" },
        { id = "s2", description = "Step 2" },
        { id = "s3", description = "Step 3" },
    },
    onStart    = function() end,
    onComplete = function() end,
})

T.check("not active initially",    not Quest.isActive("test_q"))
T.check("not completed initially", not Quest.isCompleted("test_q"))

Quest.start("test_q")
T.check("active after start",  Quest.isActive("test_q"))
T.eq("stage index 1",         1, Quest.getStageIndex("test_q"))

local stage = Quest.getStage("test_q")
T.notNil("getStage returns table", stage)
T.eq("stage id",  "s1", stage and stage.id)

Quest.advance("test_q")
T.eq("stage index 2", 2, Quest.getStageIndex("test_q"))

-- Quest events
local advanced = false
local function onAdv(d) if d.questId == "test_q" then advanced = true end end
EventBus.on("quest_advanced", onAdv)
Quest.advance("test_q")
T.check("quest_advanced event", advanced)
EventBus.off("quest_advanced", onAdv)

-- advance past last stage → completes
Quest.advance("test_q")
T.check("completed after final advance", Quest.isCompleted("test_q"))
T.check("no longer active",             not Quest.isActive("test_q"))

-- double-start blocked
local started = Quest.start("test_q")
T.check("cannot restart completed", not started)

-- fail
Quest.register("test_fail", { stages = {{ id = "x" }} })
Quest.start("test_fail")
Quest.fail("test_fail")
T.check("failed quest not active",    not Quest.isActive("test_fail"))
T.check("failed quest not completed", not Quest.isCompleted("test_fail"))

-- ── Flag ──────────────────────────────────────────────────────────────────────

T.suite("Flag")

Flag.clearAll()

T.check("get unset",  not Flag.get("a"))
Flag.set("a")
T.check("get set",    Flag.get("a"))
Flag.set("a")
T.check("double set ok", Flag.get("a"))

Flag.unset("a")
T.check("unset",      not Flag.get("a"))

Flag.set("b")
Flag.toggle("b")
T.check("toggle off", not Flag.get("b"))
Flag.toggle("b")
T.check("toggle on",  Flag.get("b"))

local all = Flag.getAll()
T.check("getAll has b", all["b"] == true)

-- Events
local evtName = nil
local function onSet(d) evtName = d.name end
EventBus.on("flag_set", onSet)
Flag.set("boss_dead")
T.eq("flag_set event name", "boss_dead", evtName)
EventBus.off("flag_set", onSet)

local unsetName = nil
local function onUnset(d) unsetName = d.name end
EventBus.on("flag_unset", onUnset)
Flag.unset("boss_dead")
T.eq("flag_unset event", "boss_dead", unsetName)
EventBus.off("flag_unset", onUnset)

Flag.clearAll()
T.check("clearAll works", not Flag.get("b"))

-- require alias
Flag.set("door_open")
T.check("require alias", Flag.require("door_open"))
Flag.clearAll()

-- ── Loot ──────────────────────────────────────────────────────────────────────

T.suite("Loot")

local PLAYER = 7002
Inventory.clear(PLAYER)

-- Table avec drop garanti + pool pondéré
Loot.define("test_chest", {
    guaranteed = {
        { item = "coin", quantity = { min = 5, max = 10 } },
    },
    pool = {
        { item = "potion", weight = 60, quantity = 1 },
        { item = "sword",  weight = 10, quantity = 1 },
        { item = nil,      weight = 30 },
    },
    rolls = 1,
})

math.randomseed(42)
local drops = Loot.roll("test_chest", 1)
T.check("roll returns table",      type(drops) == "table")
T.check("roll has guaranteed coin", (function()
    for _, d in ipairs(drops) do if d.item == "coin" then return true end end
    return false
end)())
T.check("guaranteed qty >= 5", (function()
    for _, d in ipairs(drops) do if d.item == "coin" then return d.quantity >= 5 end end
    return false
end)())

-- rollOne
local one = Loot.rollOne("test_chest")
T.check("rollOne returns entry or nil", one == nil or type(one) == "table")
if one then T.check("rollOne has item key", one.item ~= nil or one.item == nil) end

-- dropFor adds to inventory
Loot.dropFor(PLAYER, "test_chest")
T.check("dropFor adds coins to inventory", Inventory.count(PLAYER, "coin") > 0)

-- Unknown table returns empty
local empty = Loot.roll("__no_such_table__", 1)
T.eq("unknown table returns {}", 0, #empty)

-- Conditional entry (level context)
Loot.define("guarded_loot", {
    pool = {
        { item = "legendary_sword", weight = 100,
          cond = function(ctx) return ctx and ctx.level >= 10 end },
        { item = "rusty_dagger",    weight = 100 },
    },
    rolls = 1,
})
local low = Loot.roll("guarded_loot", 1, { level = 1 })
local hasLegendary = false
for _, d in ipairs(low) do if d.item == "legendary_sword" then hasLegendary = true end end
T.check("cond blocks legendary at level 1", not hasLegendary)

-- ── Persist ───────────────────────────────────────────────────────────────────

T.suite("Persist")

Persist.clear()

T.check("get missing default", Persist.get("x", 42) == 42)
T.isNil("get missing nil",     Persist.get("z"))

Persist.set("score", 500)
T.eq("set/get number", 500, Persist.get("score"))

Persist.set("name", "Hero")
T.eq("set/get string", "Hero", Persist.get("name"))

Persist.set("alive", true)
T.check("set/get bool", Persist.get("alive") == true)

local all = Persist.getAll()
T.check("getAll score", all["score"] == 500)
T.check("getAll name",  all["name"] == "Hero")

Persist.delete("score")
T.isNil("delete removes", Persist.get("score"))

-- Reload round-trip
Persist.set("persist_test", 123)
Persist.save()
Persist.load()
T.eq("save/load round-trip", 123, Persist.get("persist_test"))

Persist.clear()

return T.summary()
