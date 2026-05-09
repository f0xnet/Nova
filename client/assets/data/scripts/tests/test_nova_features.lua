-- tests/test_nova_features.lua
-- Tests : BT (Behavior Trees) · Persist.migration · Debug.inspect

local T = require("tests/assert")
T.reset()

-- ── BT ────────────────────────────────────────────────────────────────────────

T.suite("BT")

T.eq("SUCCESS = 1", 1, BT.SUCCESS)
T.eq("FAILURE = 0", 0, BT.FAILURE)
T.eq("RUNNING = 2", 2, BT.RUNNING)

-- action
do
    local called = false
    local node = BT.action(function(e, dt) called = true; return BT.SUCCESS end)
    local s = BT.run(node, nil, 0.016)
    T.check("action: appelée",            called)
    T.eq("action: retourne SUCCESS", BT.SUCCESS, s)
end

-- condition
do
    T.eq("condition true  → SUCCESS", BT.SUCCESS, BT.run(BT.condition(function() return true  end), nil, 0))
    T.eq("condition false → FAILURE", BT.FAILURE, BT.run(BT.condition(function() return false end), nil, 0))
end

-- sequence
do
    local log = {}
    local seq = BT.sequence(
        BT.action(function() table.insert(log, 1); return BT.SUCCESS end),
        BT.action(function() table.insert(log, 2); return BT.SUCCESS end),
        BT.action(function() table.insert(log, 3); return BT.SUCCESS end)
    )
    T.eq("sequence tous ok → SUCCESS", BT.SUCCESS, BT.run(seq, nil, 0))
    T.eq("sequence: tous les enfants exécutés", 3, #log)

    local seq2 = BT.sequence(
        BT.action(function() return BT.SUCCESS end),
        BT.action(function() return BT.FAILURE end),
        BT.action(function() table.insert(log, "ne doit pas run") return BT.SUCCESS end)
    )
    T.eq("sequence s'arrête sur FAILURE", BT.FAILURE, BT.run(seq2, nil, 0))
    T.eq("sequence: 3e enfant non exécuté", 3, #log)

    local seq3 = BT.sequence(
        BT.action(function() return BT.SUCCESS end),
        BT.action(function() return BT.RUNNING end)
    )
    T.eq("sequence retourne RUNNING", BT.RUNNING, BT.run(seq3, nil, 0))
end

-- selector
do
    local sel = BT.selector(
        BT.action(function() return BT.FAILURE end),
        BT.action(function() return BT.FAILURE end),
        BT.action(function() return BT.SUCCESS end)
    )
    T.eq("selector 3e réussit → SUCCESS", BT.SUCCESS, BT.run(sel, nil, 0))

    local selFail = BT.selector(
        BT.action(function() return BT.FAILURE end),
        BT.action(function() return BT.FAILURE end)
    )
    T.eq("selector tous échouent → FAILURE", BT.FAILURE, BT.run(selFail, nil, 0))

    local selRun = BT.selector(
        BT.action(function() return BT.FAILURE end),
        BT.action(function() return BT.RUNNING end)
    )
    T.eq("selector RUNNING avant SUCCESS", BT.RUNNING, BT.run(selRun, nil, 0))
end

-- parallel
do
    T.eq("parallel tous OK → SUCCESS",
        BT.SUCCESS,
        BT.run(BT.parallel(
            BT.action(function() return BT.SUCCESS end),
            BT.action(function() return BT.SUCCESS end)
        ), nil, 0))

    T.eq("parallel un FAILURE → FAILURE",
        BT.FAILURE,
        BT.run(BT.parallel(
            BT.action(function() return BT.SUCCESS end),
            BT.action(function() return BT.FAILURE end)
        ), nil, 0))

    T.eq("parallel avec RUNNING → RUNNING",
        BT.RUNNING,
        BT.run(BT.parallel(
            BT.action(function() return BT.SUCCESS end),
            BT.action(function() return BT.RUNNING  end)
        ), nil, 0))
end

-- invert
do
    T.eq("invert SUCCESS → FAILURE", BT.FAILURE,
        BT.run(BT.invert(BT.action(function() return BT.SUCCESS end)), nil, 0))
    T.eq("invert FAILURE → SUCCESS", BT.SUCCESS,
        BT.run(BT.invert(BT.action(function() return BT.FAILURE end)), nil, 0))
    T.eq("invert RUNNING → RUNNING", BT.RUNNING,
        BT.run(BT.invert(BT.action(function() return BT.RUNNING  end)), nil, 0))
end

-- succeed / fail
do
    T.eq("succeed force SUCCESS",
        BT.SUCCESS,
        BT.run(BT.succeed(BT.action(function() return BT.FAILURE end)), nil, 0))
    T.eq("fail force FAILURE",
        BT.FAILURE,
        BT.run(BT.fail(BT.action(function() return BT.SUCCESS end)), nil, 0))
end

-- loop
do
    local count = 0
    local node  = BT.loop(BT.action(function()
        count = count + 1
        return BT.SUCCESS
    end), 3)

    T.eq("loop iter 1 → RUNNING", BT.RUNNING, BT.run(node, nil, 0))
    T.eq("loop iter 2 → RUNNING", BT.RUNNING, BT.run(node, nil, 0))
    T.eq("loop iter 3 → SUCCESS", BT.SUCCESS,  BT.run(node, nil, 0))
    T.eq("loop: exactement 3 exécutions", 3, count)
end

-- arbre composé
do
    local chased  = false
    local patrolled = false

    local canSee = false   -- état de la simulation

    local tree = BT.selector(
        BT.sequence(
            BT.condition(function() return canSee end),
            BT.action(function() chased = true; return BT.SUCCESS end)
        ),
        BT.action(function() patrolled = true; return BT.SUCCESS end)
    )

    BT.run(tree, nil, 0)
    T.check("arbre: patrol quand pas de vue", patrolled)
    T.check("arbre: pas de chase sans vue",   not chased)

    chased = false; patrolled = false
    canSee = true
    BT.run(tree, nil, 0)
    T.check("arbre: chase quand vue", chased)
    T.check("arbre: pas de patrol si chase", not patrolled)
end

-- ── Persist.migration ─────────────────────────────────────────────────────────

T.suite("Persist.migration")

do
    -- 1. Remettre à v1 et sauvegarder un dataset v1
    Persist.setVersion(1)
    Persist.clear()
    Persist.set("old_hp",  75)
    Persist.set("level",    3)
    -- Le fichier est maintenant écrit avec __persist_version = 1

    -- 2. Déclarer la migration v1 → v2 (renommer old_hp → health)
    Persist.setVersion(2)
    Persist.onMigrate(1, 2, function(d)
        d["health"] = d["old_hp"]
        d["old_hp"] = nil
        return d
    end)

    -- 3. Reload : doit détecter v1 < v2 et migrer
    Persist.load()

    T.isNil("migration: ancienne clé supprimée",    Persist.get("old_hp"))
    T.eq("migration: nouvelle clé présente",   75,  Persist.get("health"))
    T.eq("migration: autre clé préservée",      3,  Persist.get("level"))

    -- 4. Vérifier que la sauvegarde migrée persiste (reload une 2e fois sans re-migrer)
    local reloadCount = 0
    -- Enregistrer une 2e migration pour v3 qui n'arrivera pas (compteur)
    Persist.setVersion(3)
    Persist.onMigrate(2, 3, function(d) reloadCount = reloadCount + 1; return d end)
    Persist.load()   -- le fichier est maintenant à v2, doit migrer vers v3
    T.eq("migration enchaînée v2→v3",  1, reloadCount)

    -- 5. Reset propre
    Persist.setVersion(1)
    Persist.clear()
    -- Vider les migrations (recréer les tables internes via la closure n'est pas
    -- possible, mais reset à v1 + clear suffit pour isoler les autres tests)
end

-- ── Debug.inspect ─────────────────────────────────────────────────────────────

T.suite("Debug.inspect")

T.eq("inspect nil",    "nil",    Debug.inspect(nil))
T.eq("inspect true",   "true",   Debug.inspect(true))
T.eq("inspect false",  "false",  Debug.inspect(false))
T.eq("inspect int",    "42",     Debug.inspect(42))
T.check("inspect float",         Debug.inspect(3.14):find("3%.") ~= nil)
T.check("inspect string quoted", Debug.inspect("hi"):find('"hi"') ~= nil)
T.eq("inspect function",         "<function>", Debug.inspect(function() end))

do
    local s = Debug.inspect({ x = 1, y = 2 })
    T.check("inspect table contient x", s:find("x") ~= nil)
    T.check("inspect table contient 1", s:find("1") ~= nil)
end

do
    local s = Debug.inspect({ a = { b = { c = 99 } } }, 3)
    T.check("inspect table nested",  s:find("99") ~= nil)
end

do
    local s = Debug.inspect({ a = { b = { c = 99 } } }, 1)
    T.check("inspect depth limite",  s:find("{…}") ~= nil)
end

return T.summary()
