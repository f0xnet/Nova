-- tests/test_engine.lua
-- Tests d'intégration moteur : Anim · Trigger · Nav · Spatial · Pool · Debug · UI · Camera · Scene
--
-- Ces tests vérifient la surface d'API et le comportement à runtime.
-- Certains nécessitent des entités réelles ou un SceneManager actif ;
-- les assertions critiques sont guardées par des vérifications préalables.

local T = require("tests/assert")
T.reset()

-- Helpers
local function hasRegistry()
    return Registry ~= nil
end

local function spawnTestEntity()
    if not hasRegistry() then return nil end
    local e = Registry:createEntity()
    if not e then return nil end
    e:addComponent("TransformComponent")
    e:addComponent("SpriteComponent")
    local t = e:getTransform()
    if t then t.position.x = 100; t.position.y = 100 end
    return e
end

-- ── Anim ──────────────────────────────────────────────────────────────────────

T.suite("Anim")

T.check("Anim module exists", Anim ~= nil)
T.check("Anim.play is function",    type(Anim.play)    == "function")
T.check("Anim.stop is function",    type(Anim.stop)    == "function")
T.check("Anim.setSpeed is function",type(Anim.setSpeed)== "function")
T.check("Anim.onFinish is function",type(Anim.onFinish)== "function")
T.check("Anim._update is function", type(Anim._update) == "function")

if hasRegistry() then
    local e = spawnTestEntity()
    if e then
        e:addComponent("AnimationComponent")
        local anim = e:getAnimation()
        if anim then
            Anim.play(e.id, "run", true)
            T.eq("Anim.play sets animationID", "run", anim.animationID)
            T.check("Anim.play sets playing", anim.playing)
            T.check("Anim.play sets loop",    anim.loop)

            Anim.stop(e.id)
            T.check("Anim.stop clears playing", not anim.playing)
        else
            Log.info("[TEST] Anim: no AnimationComponent (skip component tests)")
        end
        Registry:destroyEntity(e.id)
    end
else
    Log.info("[TEST] Anim: no Registry (skip runtime tests)")
end

-- ── Trigger ───────────────────────────────────────────────────────────────────

T.suite("Trigger")

T.check("Trigger module exists",          Trigger ~= nil)
T.check("Trigger.create is function",     type(Trigger.create)   == "function")
T.check("Trigger.remove is function",     type(Trigger.remove)   == "function")
T.check("Trigger.enable is function",     type(Trigger.enable)   == "function")
T.check("Trigger.disable is function",    type(Trigger.disable)  == "function")
T.check("Trigger.clear is function",      type(Trigger.clear)    == "function")
T.check("Trigger._update is function",    type(Trigger._update)  == "function")

do
    local entered = {}
    local exited  = {}
    local function onEnter(d) table.insert(entered, d) end
    local function onExit(d)  table.insert(exited, d)  end
    EventBus.on("trigger_enter", onEnter)
    EventBus.on("trigger_exit",  onExit)

    Trigger.create("_test_zone", { x = 0, y = 0, w = 200, h = 200 }, {
        onEnter = function(eid) end,
    })

    -- No entities → no events on _update
    if hasRegistry() then
        Trigger._update(0.016)
        T.check("no events without entities in zone", #entered == 0)
    end

    -- disable / enable
    Trigger.disable("_test_zone")
    -- disabled zone should not fire
    T.check("disable doesn't crash", true)
    Trigger.enable("_test_zone")

    Trigger.remove("_test_zone")
    T.check("remove doesn't crash", true)

    -- once flag — create a zone that should deactivate after first trigger
    Trigger.create("_once_zone", { x = 0, y = 0, w = 9999, h = 9999 }, {
        once    = true,
        onEnter = function(eid) end,
    })
    Trigger._update(0.016)
    Trigger.remove("_once_zone")

    -- clear
    Trigger.create("_z1", { x = 0, y = 0, w = 10, h = 10 }, {})
    Trigger.create("_z2", { x = 0, y = 0, w = 10, h = 10 }, {})
    Trigger.clear()
    T.check("clear: no crash", true)

    EventBus.off("trigger_enter", onEnter)
    EventBus.off("trigger_exit",  onExit)
end

-- ── Nav ───────────────────────────────────────────────────────────────────────

T.suite("Nav")

T.check("Nav module exists",             Nav ~= nil)
T.check("Nav.moveTo is function",        type(Nav.moveTo)         == "function")
T.check("Nav.stop is function",          type(Nav.stop)           == "function")
T.check("Nav.isMoving is function",      type(Nav.isMoving)       == "function")
T.check("Nav.findPath is function",      type(Nav.findPath)       == "function")
T.check("Nav.moveAlongPath is function", type(Nav.moveAlongPath)  == "function")
T.check("Nav._update is function",       type(Nav._update)        == "function")

if hasRegistry() then
    local e = spawnTestEntity()
    if e then
        T.check("not moving before moveTo", not Nav.isMoving(e.id))

        Nav.moveTo(e.id, { x = 500, y = 500 }, 100)
        T.check("isMoving after moveTo", Nav.isMoving(e.id))

        Nav.stop(e.id)
        T.check("not moving after stop", not Nav.isMoving(e.id))

        -- moveAlongPath with direct path
        local path = { {x=100, y=100}, {x=200, y=200}, {x=300, y=300} }
        Nav.moveAlongPath(e.id, path, 200)
        T.check("isMoving along path", Nav.isMoving(e.id))

        -- advance along path with _update
        Nav._update(5.0)
        -- after enough time, entity should reach destination
        -- (no assert on exact position since we don't know entity speed result)

        Registry:destroyEntity(e.id)
    end
else
    Log.info("[TEST] Nav: no Registry (skip runtime tests)")
end

-- ── Spatial ───────────────────────────────────────────────────────────────────

T.suite("Spatial")

T.check("Spatial module exists",            Spatial ~= nil)
T.check("Spatial.findInRadius is function", type(Spatial.findInRadius) == "function")
T.check("Spatial.findInRect is function",   type(Spatial.findInRect)   == "function")
T.check("Spatial.nearest is function",      type(Spatial.nearest)      == "function")
T.check("Spatial.count is function",        type(Spatial.count)        == "function")
T.check("Spatial._update is function",      type(Spatial._update)      == "function")
T.check("Spatial.CELL_SIZE is number",      type(Spatial.CELL_SIZE)    == "number")

if hasRegistry() then
    -- Place entities and rebuild spatial hash
    local e1 = spawnTestEntity()
    local e2 = spawnTestEntity()
    if e1 and e2 then
        local t1 = e1:getTransform(); if t1 then t1.position.x = 50;  t1.position.y = 50  end
        local t2 = e2:getTransform(); if t2 then t2.position.x = 500; t2.position.y = 500 end
        -- Assign tags
        e1:addComponent("TagComponent")
        e2:addComponent("TagComponent")
        local tag1 = e1:getTag(); if tag1 then tag1.tag = "enemy" end
        local tag2 = e2:getTag(); if tag2 then tag2.tag = "enemy" end

        Spatial._update(0)  -- rebuild grid

        local near = Spatial.findInRadius(50, 50, 100, "enemy")
        T.check("findInRadius finds nearby entity", #near >= 1)

        local far = Spatial.findInRadius(50, 50, 10, "enemy")
        T.check("findInRadius with tiny radius may be 0 or 1",
                type(far) == "table")

        local rect = Spatial.findInRect(0, 0, 200, 200, "enemy")
        T.check("findInRect returns table", type(rect) == "table")

        local n = Spatial.nearest(50, 50, "enemy")
        T.check("nearest returns entity or nil",
                n == nil or type(n) == "userdata")

        local cnt = Spatial.count("enemy")
        T.check("count returns number", type(cnt) == "number" and cnt >= 0)

        Registry:destroyEntity(e1.id)
        Registry:destroyEntity(e2.id)
    end
else
    Log.info("[TEST] Spatial: no Registry (skip runtime tests)")
end

-- ── Pool ──────────────────────────────────────────────────────────────────────

T.suite("Pool")

T.check("Pool module exists",           Pool ~= nil)
T.check("Pool.create is function",      type(Pool.create)    == "function")
T.check("Pool.acquire is function",     type(Pool.acquire)   == "function")
T.check("Pool.release is function",     type(Pool.release)   == "function")
T.check("Pool.available is function",   type(Pool.available) == "function")
T.check("Pool.size is function",        type(Pool.size)      == "function")
T.check("Pool.clear is function",       type(Pool.clear)     == "function")
T.check("Pool.clearAll is function",    type(Pool.clearAll)  == "function")

if hasRegistry() then
    Pool.create("_test_pool", function()
        local e = Registry:createEntity()
        e:addComponent("TransformComponent")
        e:addComponent("SpriteComponent")
        return e
    end, 3)

    T.eq("pool size after create",     3, Pool.size("_test_pool"))
    T.eq("pool available after create",3, Pool.available("_test_pool"))

    local a1 = Pool.acquire("_test_pool")
    T.check("acquire returns entity",     a1 ~= nil)
    T.eq("available after 1 acquire",    2, Pool.available("_test_pool"))

    local a2 = Pool.acquire("_test_pool")
    local a3 = Pool.acquire("_test_pool")
    T.eq("available after 3 acquires",   0, Pool.available("_test_pool"))

    local a4 = Pool.acquire("_test_pool")
    T.check("nil when pool exhausted",    a4 == nil)

    if a1 then Pool.release("_test_pool", a1) end
    T.eq("available after release",      1, Pool.available("_test_pool"))

    Pool.clear("_test_pool")
    T.eq("size after clear",             0, Pool.size("_test_pool"))
else
    Log.info("[TEST] Pool: no Registry (skip runtime tests)")
end

-- ── Debug ─────────────────────────────────────────────────────────────────────

T.suite("Debug")

T.check("Debug module exists",          Debug ~= nil)
T.check("Debug.drawRect is function",   type(Debug.drawRect)   == "function")
T.check("Debug.drawLine is function",   type(Debug.drawLine)   == "function")
T.check("Debug.drawCircle is function", type(Debug.drawCircle) == "function")
T.check("Debug.label is function",      type(Debug.label)      == "function")
T.check("Debug.watch is function",      type(Debug.watch)      == "function")
T.check("Debug.clear is function",      type(Debug.clear)      == "function")
T.check("Debug._update is function",    type(Debug._update)    == "function")
T.check("Debug._flush is function",     type(Debug._flush)     == "function")
T.check("Debug.fillRect is function",   type(Debug.fillRect)   == "function")

do
    -- Verify queueing does not error
    Debug.drawRect(10, 10, 50, 50)
    Debug.drawLine(0, 0, 100, 100)
    Debug.drawCircle(50, 50, 30)
    Debug.label(10, 10, "test")
    Debug.watch("hp", 100)
    Debug._update(0.016)
    Debug.clear()
    T.check("Debug: no error after draw calls", true)
end

-- ── UI (bridge) ───────────────────────────────────────────────────────────────

T.suite("UI_bridge")

T.check("UI module exists",           UI ~= nil)
T.check("UI.show is function",        type(UI.show)      == "function")
T.check("UI.hide is function",        type(UI.hide)      == "function")
T.check("UI.onClick is function",     type(UI.onClick)   == "function")
T.check("UI.onAction is function",    type(UI.onAction)  == "function")
T.check("UI.on is function",          type(UI.on)        == "function")
T.check("UI.off is function",         type(UI.off)       == "function")
T.check("UI.getText is function",     type(UI.getText)   == "function")
T.check("UI.load is function",        type(UI.load)      == "function")

do
    -- Test onClick/onAction wiring via EventBus
    local clicked = false
    UI.onClick("_test_btn", function() clicked = true end)
    EventBus.emit("ui_click__test_btn", { action = "click", value = "" })
    T.check("onClick handler fires on event", clicked)

    local actionValue = nil
    UI.onAction("_test_action", function(v) actionValue = v end)
    EventBus.emit("ui_action__test_action", { value = "42", id = "_test_btn" })
    T.eq("onAction receives value", "42", actionValue)

    -- generic UI.on
    local genericEvt = nil
    UI.on("_custom", function(d) genericEvt = d end)
    EventBus.emit("ui__custom", { x = 7 })
    T.check("UI.on generic event", genericEvt ~= nil and genericEvt.x == 7)

    -- UI.off without handler clears all
    UI.off("_custom")
    genericEvt = nil
    EventBus.emit("ui__custom", {})
    T.check("UI.off clears handlers", genericEvt == nil)

    -- Cleanup
    EventBus.clear("ui_click__test_btn")
    EventBus.clear("ui_action__test_action")
end

-- ── Camera ────────────────────────────────────────────────────────────────────

T.suite("Camera")

T.check("Camera module exists",              Camera ~= nil)
T.check("Camera.follow is function",         type(Camera.follow)      == "function")
T.check("Camera.unfollow is function",       type(Camera.unfollow)    == "function")
T.check("Camera.shake is function",          type(Camera.shake)       == "function")
T.check("Camera.setZoom is function",        type(Camera.setZoom)     == "function")
T.check("Camera.setPosition is function",    type(Camera.setPosition) == "function")
T.check("Camera.getPosition is function",    type(Camera.getPosition) == "function")
T.check("Camera.moveTo is function",         type(Camera.moveTo)      == "function")
T.check("Camera.zoomTo is function",         type(Camera.zoomTo)      == "function")
T.check("Camera.getZoom is function",        type(Camera.getZoom)     == "function")
T.check("Camera.reset is function",          type(Camera.reset)       == "function")
T.check("Camera.path is function",           type(Camera.path)        == "function")
T.check("Camera._update is function",        type(Camera._update)     == "function")

do
    Camera.setPosition(200, 150)
    Camera.shake(0.2, 10)
    Camera._update(0.05)
    Camera._update(0.05)

    Camera.setZoom(1.5)
    local z = Camera.getZoom()
    T.check("getZoom returns number", type(z) == "number")

    -- path with empty list fires onDone immediately
    local pathDone = false
    Camera.path({}, 100, "linear", function() pathDone = true end)
    T.check("path empty fires onDone", pathDone)

    -- path with waypoints starts tween
    Camera.path({{x=300, y=300}, {x=400, y=400}}, 500, "linear")
    Tween.update(2.0)  -- advance tweens

    Camera.reset()
    T.check("Camera: no error after full cycle", true)
end

-- ── Scene ─────────────────────────────────────────────────────────────────────

T.suite("Scene")

T.check("Scene module exists",              Scene ~= nil)
T.check("Scene.load is function",           type(Scene.load)       == "function")
T.check("Scene.transition is function",     type(Scene.transition) == "function")
T.check("Scene.current is function",        type(Scene.current)    == "function")
T.check("Scene.has is function",            type(Scene.has)        == "function")
T.check("Scene.count is function",          type(Scene.count)      == "function")
T.check("Scene.unload is function",         type(Scene.unload)     == "function")
T.check("Scene.reload is function",         type(Scene.reload)     == "function")
T.check("Scene.findPath is function",       type(Scene.findPath)   == "function")

do
    -- current() returns string (may be empty)
    local cur = Scene.current()
    T.check("current returns string", type(cur) == "string")

    -- count returns number
    local cnt = Scene.count()
    T.check("count returns number", type(cnt) == "number")

    -- has with unknown name returns false
    T.check("has unknown scene = false", not Scene.has("__nonexistent_scene__"))

    -- findPath with no waypoints returns empty table (not an error)
    local path = Scene.findPath(0, 0, 100, 100)
    T.check("findPath returns table", type(path) == "table")
end

return T.summary()
