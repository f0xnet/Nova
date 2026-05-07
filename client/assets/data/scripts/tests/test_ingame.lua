-- tests/test_ingame.lua
-- Tests interactifs en jeu du système de scripting.
-- Appuyez sur F5 pour lancer la suite complète.
--
-- Chaque test émet une notification à l'écran (vert = OK, rouge = FAIL)
-- et log le résultat dans la console.

local M = {}

-- ── Helpers ────────────────────────────────────────────────────────────────

local function ok(label)
    local msg = "OK  " .. label
    Log.info("[INGAME TEST] " .. msg)
    Notify.show(msg, { color = "lime", duration = 2.5 })
end

local function fail(label)
    local msg = "FAIL  " .. label
    Log.warn("[INGAME TEST] " .. msg)
    Notify.show(msg, { color = "red", duration = 3.5 })
end

local function title(label)
    Log.info("[INGAME TEST] ── " .. label .. " ──")
    Notify.show(label, { color = "gold", duration = 1.5 })
end

-- ── EventBus ───────────────────────────────────────────────────────────────

local function testEventBus()
    title("EventBus")

    -- on / emit / off
    local got = nil
    local function h(d) got = d.v end
    EventBus.on("_t_ig_evt", h)
    EventBus.emit("_t_ig_evt", { v = 7 })
    EventBus.off("_t_ig_evt", h)
    if got == 7 then ok("on/emit/off") else fail("on/emit/off (got=" .. tostring(got) .. ")") end

    -- once
    local n = 0
    EventBus.once("_t_ig_once", function() n = n + 1 end)
    EventBus.emit("_t_ig_once", {})
    EventBus.emit("_t_ig_once", {})
    if n == 1 then ok("once") else fail("once (n=" .. n .. ")") end

    -- off bloque bien
    got = nil
    EventBus.emit("_t_ig_evt", { v = 99 })
    if got == nil then ok("off bloque handler") else fail("off ne bloque pas") end
end

-- ── Timer ──────────────────────────────────────────────────────────────────

local function testTimer()
    title("Timer")

    -- after
    local fired = false
    Timer.after(0.4, function()
        fired = true
        ok("Timer.after(0.4s)")
    end)

    -- every + cancel
    local n = 0
    local id = Timer.every(0.15, function() n = n + 1 end)
    Timer.after(0.6, function()
        Timer.cancel(id)
        local snap = n
        Timer.after(0.3, function()
            if n == snap then
                ok("Timer.every + cancel (fired " .. snap .. "x)")
            else
                fail("cancel ne stoppe pas (avant=" .. snap .. " après=" .. n .. ")")
            end
        end)
    end)
end

-- ── Tween ──────────────────────────────────────────────────────────────────

local function testTween()
    title("Tween")

    local val = 0
    Tween.new(0, 100, 0.5, "linear",
        function(v) val = v end,
        function()
            if math.abs(val - 100) < 1 then
                ok("Tween.new linéaire (val=" .. math.floor(val) .. ")")
            else
                fail("Tween.new (val=" .. val .. ")")
            end
        end)

    -- cancel
    local frozen = 0
    local id = Tween.new(0, 50, 1.0, "linear", function(v) frozen = v end)
    Timer.after(0.2, function()
        Tween.cancel(id)
        local snap = frozen
        Timer.after(0.4, function()
            if math.abs(frozen - snap) < 0.01 then
                ok("Tween.cancel (gelé à " .. math.floor(snap) .. ")")
            else
                fail("Tween.cancel ne gèle pas")
            end
        end)
    end)
end

-- ── Scheduler ──────────────────────────────────────────────────────────────

local function testScheduler()
    title("Scheduler")

    local log = {}
    Scheduler.start(function()
        table.insert(log, "a")
        Scheduler.wait(0.2)
        table.insert(log, "b")
        Scheduler.wait(0.2)
        table.insert(log, "c")
    end)

    Timer.after(0.6, function()
        if log[1]=="a" and log[2]=="b" and log[3]=="c" then
            ok("Scheduler wait+resume")
        else
            fail("Scheduler (log=" .. table.concat(log, ",") .. ")")
        end
    end)
end

-- ── Sequence ───────────────────────────────────────────────────────────────

local function testSequence()
    title("Sequence")

    -- call + wait + onComplete
    local log = {}
    local done = false
    local seq = Sequence.new()
    seq:call(function() table.insert(log, 1) end)
    seq:wait(0.25)
    seq:call(function() table.insert(log, 2) end)
    seq:wait(0.25)
    seq:call(function() table.insert(log, 3) end)
    seq:play(function() done = true end)

    if log[1] == 1 then ok("call immédiat au play()") else fail("call immédiat") end

    Timer.after(0.35, function()
        if log[2] == 2 then ok("call après 1er wait") else fail("call après 1er wait") end
    end)
    Timer.after(0.65, function()
        if log[3] == 3 and done then
            ok("enchaînement + onComplete")
        else
            fail("enchaînement (log=" .. #log .. " done=" .. tostring(done) .. ")")
        end
    end)

    -- emit immédiat
    local emitted = false
    local function h() emitted = true end
    EventBus.on("_t_ig_seq_emit", h)
    local seq2 = Sequence.new()
    seq2:emit("_t_ig_seq_emit", {})
    seq2:play()
    EventBus.off("_t_ig_seq_emit", h)
    if emitted then ok("emit immédiat") else fail("emit immédiat") end

    -- stop
    local halted = {}
    local seq3 = Sequence.new()
    seq3:call(function() table.insert(halted, 1) end)
    seq3:wait(5.0)
    seq3:call(function() table.insert(halted, 2) end)
    seq3:play()
    seq3:stop()
    Timer.after(0.1, function()
        if #halted == 1 then ok("stop() halt") else fail("stop() (halted=" .. #halted .. ")") end
    end)
end

-- ── Camera ─────────────────────────────────────────────────────────────────

local function testCamera()
    title("Camera")

    local seq = Sequence.new()

    seq:call(function()
        title("Camera.shake")
        Camera.shake(0.5, 18)
    end)
    seq:wait(0.9)

    seq:call(function()
        title("Camera.zoomTo x2")
        Camera.zoomTo(2.0, 0.6, "easeInOut")
    end)
    seq:wait(0.9)

    seq:call(function()
        title("Camera.zoomTo x1")
        Camera.zoomTo(1.0, 0.6, "easeOut")
    end)
    seq:wait(0.9)

    seq:call(function()
        title("Camera.moveTo +200,+100")
        local p = Camera.getPosition()
        Camera.moveTo(p.x + 200, p.y + 100, 0.7, "easeInOut")
    end)
    seq:wait(0.9)

    seq:call(function()
        Camera.reset()
        ok("Camera shake/zoom/moveTo/reset")
    end)

    seq:play()
end

-- ── Conversation ───────────────────────────────────────────────────────────

local function testConversation()
    title("Conversation")

    Conversation.define("_ingame_conv_test", {
        start = "greet",
        nodes = {
            greet = {
                speaker = "Guard",
                text    = "[TEST] Halte ! Qui va là ?",
                choices = {
                    { text = "Un ami.", next = "friend" },
                    { text = "Personne.", next = "hostile" },
                },
            },
            friend  = { speaker = "Guard", text = "[TEST] Passez, ami.", next = nil },
            hostile = { speaker = "Guard", text = "[TEST] Aux armes !",  next = nil },
        },
    })

    local nodes = {}
    local function onNode(d)
        table.insert(nodes, d.text)
        Notify.show("[" .. d.speaker .. "] " .. d.text, { color = "cyan", duration = 1.8 })
    end
    local function onEnd()
        EventBus.off("conversation_node", onNode)
        EventBus.off("conversation_end",  onEnd)
        if #nodes == 2 then
            ok("Conversation start/choose/advance (" .. #nodes .. " nœuds)")
        else
            fail("Conversation (#nodes=" .. #nodes .. ")")
        end
    end
    EventBus.on("conversation_node", onNode)
    EventBus.on("conversation_end",  onEnd)

    local ok_start = Conversation.start("_ingame_conv_test")
    if not ok_start then fail("Conversation.start retourne false") return end

    if not Conversation.isRunning() then fail("isRunning() false après start") return end

    local seq = Sequence.new()
    seq:wait(2.0)
    seq:call(function() Conversation.choose(1) end)
    seq:wait(1.5)
    seq:call(function() Conversation.advance() end)
    seq:play()
end

-- ── Run all ────────────────────────────────────────────────────────────────

function M.runAll()
    Notify.show("=== INGAME TESTS START ===", { color = "gold", duration = 2.0 })
    Log.info("[INGAME TEST] ════════════════ START ════════════════")

    local seq = Sequence.new()

    seq:call(testEventBus)
    seq:wait(0.5)

    seq:call(testTimer)
    seq:wait(2.5)

    seq:call(testTween)
    seq:wait(2.0)

    seq:call(testScheduler)
    seq:wait(1.5)

    seq:call(testSequence)
    seq:wait(2.0)

    seq:call(testCamera)
    seq:wait(5.5)

    seq:call(testConversation)
    seq:wait(5.5)

    seq:call(function()
        Notify.show("=== INGAME TESTS DONE ===", { color = "gold", duration = 3.0 })
        Log.info("[INGAME TEST] ════════════════  DONE  ════════════════")
    end)

    seq:play()
end

return M
