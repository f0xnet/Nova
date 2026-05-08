-- nova/scene.lua
-- Gestion de scènes haut niveau — enveloppe le SceneManager C++ (global Scene).
-- Remplace le global Scene par une version enrichie avec transitions et tracking.
-- Disponible sans require : Scene est un global auto-chargé.
--
-- API :
--   Scene.load(path, name)               -- charge une scène depuis un fichier JSON
--   Scene.setActive(name)                -- active une scène chargée
--   Scene.transition(name, opts)         -- change de scène avec effet de fondu
--   Scene.current()                      -- nom de la scène courante
--   Scene.unload(name)                   -- décharge une scène de la mémoire
--   Scene.has(name)                      -- vrai si la scène est chargée
--   Scene.count()                        -- nombre de scènes chargées
--   Scene.reload()                       -- re-active la scène courante
--
-- opts pour transition :
--   fade      bool    -- fondu au noir via PostFX (défaut: true)
--   duration  number  -- durée de chaque demi-fondu en secondes (défaut: 0.5)
--
-- EventBus events émis :
--   "scene_changing"  { from, to }
--   "scene_changed"   { name }
--
-- Exemple :
--   -- Chargement + transition avec fondu
--   Scene.load("data/scenes/dungeon_01.scene", "dungeon_01")
--   Scene.transition("dungeon_01", { fade = true, duration = 0.6 })
--
--   -- Réagir au changement
--   EventBus.on("scene_changed", function(data)
--       Log.info("Scène active : " .. data.name)
--       Camera.follow(Game.getPlayer())
--   end)

-- Capture le SceneManager C++ (enregistré comme "Scene" par LuaBindings)
-- avant que ce module remplace le global "Scene".
local _sm = Scene

local Scene_module = {}

-- Initialise avec le nom de la scène déjà active (chargée par Game.cpp)
local _currentName = (_sm and _sm:getActiveName()) or ""

-- ── Abonnements EventBus à portée de scène ────────────────────────────────
-- Scene.listen() enregistre un handler tagué avec le nom de scène courant.
-- À chaque setActive(), seuls les handlers de la scène sortante sont retirés.
-- Les scènes simultanées gardent leurs propres handlers intacts.
local _sceneListeners = {}  -- { event, fn, scope }

function Scene_module.listen(event, handler)
    EventBus.on(event, handler)
    _sceneListeners[#_sceneListeners + 1] = { event = event, fn = handler, scope = _currentName }
end

local function _clearListenersForScope(scope)
    local remaining = {}
    for _, l in ipairs(_sceneListeners) do
        if l.scope == scope then
            EventBus.off(l.event, l.fn)
        else
            remaining[#remaining + 1] = l
        end
    end
    _sceneListeners = remaining
end

function Scene_module.load(path, name)
    return _sm:load(path, name)
end

function Scene_module.setActive(name)
    local prev = _currentName
    _currentName = name
    EventBus.emit("scene_changing", { from = prev, to = name })
    -- Nettoyage ciblé : seuls les ressources de la scène SORTANTE sont libérées.
    -- Les autres scènes actives simultanément ne sont pas affectées.
    if prev ~= "" then
        Timer.cancelScope(prev)
        Scheduler.cancelScope(prev)
        _clearListenersForScope(prev)
    end
    _sm:setActive(name)
    EventBus.emit("scene_changed", { name = name })
end

-- Transition avec fondu au noir via SceneFX (plein écran, UI incluse)
-- opts.onBeforeChange : callback appelé pendant le noir, avant setActive
--   → usage typique : UI.removeUI("mon_ui") pour nettoyer l'UI sortante
function Scene_module.transition(name, opts)
    opts = opts or {}
    local fade           = opts.fade ~= false
    local duration       = opts.duration or 0.5
    local onBeforeChange = opts.onBeforeChange

    if not _sm:hasScene(name) then
        Log.warn("[Scene.transition] scène '" .. name .. "' non chargée, appel load() d'abord")
        return
    end

    if fade then
        SceneFX.transition(function()
            if onBeforeChange then onBeforeChange() end
            Scene_module.setActive(name)
        end, duration)
    else
        if onBeforeChange then onBeforeChange() end
        Scene_module.setActive(name)
    end
end

function Scene_module.current()
    return _currentName
end

-- Retourne le scope de la scène active (= son nom).
-- Utilisé par les scripts qui veulent tagger leurs timers/coroutines
-- manuellement : Timer.after(1.0, fn, Scene.scope())
function Scene_module.scope()
    return _currentName
end

function Scene_module.unload(name)
    if name == _currentName then _currentName = "" end
    _sm:unload(name)
end

function Scene_module.has(name)
    return _sm:hasScene(name)
end

function Scene_module.count()
    return _sm:sceneCount()
end

-- Re-active la scène courante (utile pour forcer un rechargement des scripts)
function Scene_module.reload()
    if _currentName ~= "" then
        Scene_module.setActive(_currentName)
    end
end

-- Pathfinding via le WaypointGraph de la scène active
-- Retourne une table de { x, y } formant le chemin entre deux positions
function Scene_module.findPath(x1, y1, x2, y2)
    return _sm:findPath(x1, y1, x2, y2)
end

return Scene_module
