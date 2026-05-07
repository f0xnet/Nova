-- tests/test_systems.lua
-- Tests : Sequence · Conversation · InputBind · I18n · Achievement · Data

local T = require("tests/assert")
T.reset()

-- ── Sequence ──────────────────────────────────────────────────────────────────

T.suite("Sequence")

do
    local log = {}

    local seq = Sequence.new()
    seq:call(function() table.insert(log, "start") end)
    seq:wait(0.2)
    seq:call(function() table.insert(log, "mid") end)
    seq:wait(0.3)
    seq:call(function() table.insert(log, "end") end)

    local completed = false
    seq:play(function() completed = true end)

    T.eq("call fires immediately at play", "start", log[1])
    T.check("mid not yet", log[2] == nil)

    Sequence._update(0.25)
    T.eq("mid fires after 0.25s", "mid", log[2])
    T.check("end not yet", log[3] == nil)

    Sequence._update(0.35)
    T.eq("end fires after 0.6s total", "end", log[3])
    T.check("onComplete called", completed)
end

do
    local tweenVals = {}
    local seq = Sequence.new()
    seq:tween(1.0, function(t) table.insert(tweenVals, t) end)
    seq:play()

    Sequence._update(0.5)
    T.check("tween fn called during update", #tweenVals > 0)
    T.check("tween t in [0,1]", tweenVals[#tweenVals] >= 0 and tweenVals[#tweenVals] <= 1)

    Sequence._update(0.6)
end

do
    local emitted = false
    local function handler() emitted = true end
    EventBus.on("_t_seq_emit", handler)

    local seq = Sequence.new()
    seq:emit("_t_seq_emit", {})
    seq:play()

    T.check("seq:emit fires event immediately", emitted)
    EventBus.off("_t_seq_emit", handler)
end

do
    local log = {}
    local seq = Sequence.new()
    seq:call(function() table.insert(log, 1) end)
    seq:wait(10.0)
    seq:call(function() table.insert(log, 2) end)
    seq:play()
    seq:stop()
    Sequence._update(20.0)
    T.eq("stop halts at current step", 1, #log)
end

-- ── Conversation ──────────────────────────────────────────────────────────────

T.suite("Conversation")

do
    Conversation.define("test_conv", {
        start = "greet",
        nodes = {
            greet = {
                speaker = "guard",
                text    = "Who are you?",
                choices = {
                    { text = "A friend.", next = "friendly" },
                    { text = "Nobody.",   next = "hostile"  },
                },
            },
            friendly = {
                speaker = "guard",
                text    = "Welcome.",
                next    = nil,
            },
            hostile = {
                speaker = "guard",
                text    = "Guards!",
                next    = nil,
            },
        },
    })

    T.check("not running before start", not Conversation.isRunning())

    local nodeEvents = {}
    local function onNode(d) table.insert(nodeEvents, d) end
    EventBus.on("conversation_node", onNode)

    Conversation.start("test_conv")
    T.check("running after start", Conversation.isRunning())
    T.check("node event emitted",  #nodeEvents == 1)
    T.eq("first node speaker", "guard", nodeEvents[1].speaker)
    T.eq("first node text", "Who are you?", nodeEvents[1].text)

    local cur = Conversation.getCurrent()
    T.notNil("getCurrent returns value", cur)
    T.eq("getCurrent choices count", 2, cur.choices and #cur.choices or 0)

    -- Choose option 1
    local endEvents = {}
    local function onEnd(d) table.insert(endEvents, d) end
    EventBus.on("conversation_end", onEnd)

    Conversation.choose(1)
    T.eq("moved to friendly node",  "Welcome.", (Conversation.getCurrent() or {text=""}).text)

    Conversation.advance()
    T.check("ended after nil next", not Conversation.isRunning())
    T.check("conversation_end emitted", #endEvents == 1)
    T.eq("end event id", "test_conv", endEvents[1].id)

    EventBus.off("conversation_node", onNode)
    EventBus.off("conversation_end", onEnd)
end

do
    -- Conversation with linear next (no choices)
    Conversation.define("linear_conv", {
        start = "a",
        nodes = {
            a = { speaker = "npc", text = "Hello.",   next = "b" },
            b = { speaker = "npc", text = "Goodbye.", next = nil },
        },
    })

    Conversation.start("linear_conv")
    T.eq("linear node a text", "Hello.", (Conversation.getCurrent() or {text=""}).text)
    Conversation.advance()
    T.eq("linear node b text", "Goodbye.", (Conversation.getCurrent() or {text=""}).text)
    Conversation.advance()
    T.check("linear ended", not Conversation.isRunning())
end

do
    -- stop()
    Conversation.define("stoppable", {
        start = "x",
        nodes = { x = { speaker = "s", text = "Hi", next = nil } },
    })
    Conversation.start("stoppable")
    Conversation.stop()
    T.check("stop() ends conversation", not Conversation.isRunning())
end

-- ── InputBind ─────────────────────────────────────────────────────────────────

T.suite("InputBind")

do
    -- register and getKey
    InputBind.register("jump",   "Space")
    InputBind.register("attack", "Z")
    T.eq("getKey jump",   "Space", InputBind.getKey("jump"))
    T.eq("getKey attack", "Z",     InputBind.getKey("attack"))

    -- register is idempotent
    InputBind.register("jump", "Enter")
    T.eq("register doesn't overwrite", "Space", InputBind.getKey("jump"))

    -- remap
    InputBind.remap("jump", "Enter")
    T.eq("remap works", "Enter", InputBind.getKey("jump"))

    -- remap event
    local remapped = nil
    local function onRemap(d) remapped = d end
    EventBus.on("input_remapped", onRemap)
    InputBind.remap("attack", "X")
    T.eq("remap event action", "attack", remapped and remapped.action)
    T.eq("remap event key",    "X",      remapped and remapped.key)
    EventBus.off("input_remapped", onRemap)

    -- getAll
    local all = InputBind.getAll()
    T.check("getAll has jump",   all["jump"] ~= nil)
    T.check("getAll has attack", all["attack"] ~= nil)

    -- save / load round-trip
    local saved = InputBind.save()
    T.eq("save includes jump", "Enter", saved["jump"])

    InputBind.remap("jump", "W")
    InputBind.load(saved)
    T.eq("load restores jump", "Enter", InputBind.getKey("jump"))

    -- missing action
    T.isNil("unknown action returns nil", InputBind.getKey("__nonexistent__"))

    -- isPressed/isJustPressed return false for unmapped
    T.check("isPressed unmapped = false", not InputBind.isPressed("__none__"))
    T.check("isJustPressed unmapped = false", not InputBind.isJustPressed("__none__"))
    T.check("isJustReleased unmapped = false", not InputBind.isJustReleased("__none__"))
end

-- ── I18n ──────────────────────────────────────────────────────────────────────

T.suite("I18n")

do
    -- loadTable instead of file-based load
    I18n.loadTable("en", {
        ["ui.start"]  = "Start",
        ["ui.quit"]   = "Quit",
        ["ui.score"]  = "Score: %d",
        ["npc.greet"] = "Hello, traveler!",
    })
    I18n.loadTable("fr", {
        ["ui.start"]  = "Commencer",
        ["ui.quit"]   = "Quitter",
        ["npc.greet"] = "Bonjour, voyageur !",
    })

    I18n.set("en")
    T.eq("get lang", "en", I18n.get())
    T.eq("t basic", "Start", I18n.t("ui.start"))
    T.eq("t with format", "Score: 42", I18n.t("ui.score", 42))
    T.check("has existing key", I18n.has("ui.quit"))
    T.check("has missing key false", not I18n.has("__missing__"))

    -- Missing key falls back to key itself
    T.eq("missing key returns key", "ui.missing", I18n.t("ui.missing"))

    -- Language switch
    local langChanged = nil
    local function onLang(d) langChanged = d.lang end
    EventBus.on("language_changed", onLang)
    I18n.set("fr")
    T.eq("lang changed event", "fr", langChanged)
    T.eq("french translation", "Commencer", I18n.t("ui.start"))
    EventBus.off("language_changed", onLang)

    -- Fallback language
    I18n.fallback("en")
    T.eq("fallback for missing key in fr", "Score: 5", I18n.t("ui.score", 5))

    I18n.set("en")
end

-- ── Achievement ───────────────────────────────────────────────────────────────

T.suite("Achievement")

do
    Achievement.reset()

    Achievement.define("first_blood", {
        name = "First Blood",
        desc = "Kill your first enemy.",
    })
    Achievement.define("pacifist", {
        name   = "Pacifist",
        desc   = "Complete without killing.",
        hidden = true,
    })

    T.check("not unlocked initially", not Achievement.isUnlocked("first_blood"))
    T.check("hidden not unlocked",    not Achievement.isUnlocked("pacifist"))

    local unlockedEvt = nil
    local function onUnlock(d) unlockedEvt = d end
    EventBus.on("achievement_unlocked", onUnlock)

    Achievement.unlock("first_blood")
    T.check("isUnlocked after unlock",      Achievement.isUnlocked("first_blood"))
    T.check("unlock event fired",           unlockedEvt ~= nil)
    T.eq("unlock event id",   "first_blood", unlockedEvt and unlockedEvt.id)
    T.eq("unlock event name", "First Blood", unlockedEvt and unlockedEvt.name)

    -- Double unlock is no-op (no second event)
    unlockedEvt = nil
    Achievement.unlock("first_blood")
    T.check("no event on double unlock", unlockedEvt == nil)

    EventBus.off("achievement_unlocked", onUnlock)

    -- getAll
    local all = Achievement.getAll()
    T.check("getAll has first_blood", all["first_blood"] ~= nil)
    T.check("getAll has pacifist",    all["pacifist"]    ~= nil)
    T.check("first_blood is unlocked in getAll", all["first_blood"].unlocked == true)

    -- getUnlocked
    local unlocked = Achievement.getUnlocked()
    T.check("getUnlocked has first_blood", (function()
        for _, id in ipairs(unlocked) do if id == "first_blood" then return true end end
        return false
    end)())

    -- reset
    Achievement.reset()
    T.check("reset clears unlock", not Achievement.isUnlocked("first_blood"))
end

-- ── Data ──────────────────────────────────────────────────────────────────────

T.suite("Data")

do
    T.check("not loaded initially", not Data.isLoaded("_test_data"))

    -- Direct set (bypasses file load, tests in-memory manipulation)
    -- We'll test by injecting a synthetic table into cache via the set path
    -- First: load with a guaranteed-fail path to check it handles missing files
    local ok = Data.load("_test_data", "__nonexistent_file__.json")
    T.check("load missing file returns false", not ok)
    T.check("isLoaded false after failed load", not Data.isLoaded("_test_data"))

    -- Inject data manually via get-after-set trick:
    -- Data requires load before set, so we fake-load first
    -- by calling Data.set we can only set on already loaded ids.
    -- Instead, test the internal set via a successful get of a loaded id.
    -- Here we test with a real JSON file if present.

    -- Synthetically test in-memory set by loading a known path
    -- (data/definitions may not exist in test env; tolerate gracefully)
    local realLoaded = Data.load("_items_test", "data/definitions/items.json")
    if realLoaded then
        T.check("isLoaded after real load", Data.isLoaded("_items_test"))
        local all = Data.getAll("_items_test")
        T.check("getAll returns table", type(all) == "table")

        -- Test set (modify in-memory)
        Data.set("_items_test", "__test_key__", 99)
        T.eq("set/get in-memory", 99, Data.get("_items_test", "__test_key__"))

        -- query
        local results = Data.query("_items_test", function(v) return v.__test_key__ ~= nil end)
        T.check("query returns table", type(results) == "table")

        Data.unload("_items_test")
        T.check("unload removes", not Data.isLoaded("_items_test"))
    else
        -- Graceful skip: file not present in test environment
        Log.info("[TEST] Data: skipping file-based tests (no data/definitions/items.json)")
        T.check("Data module exists", Data ~= nil)
        T.check("isLoaded returns bool", type(Data.isLoaded("x")) == "boolean")
    end

    -- Warn on get of unloaded id
    local v = Data.get("_test_data", "x")
    T.isNil("get unloaded returns nil", v)
end

return T.summary()
