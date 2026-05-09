-- nova/dev_console.lua
-- Console développeur in-game, ouvrable avec N.
-- Chargée par main.lua via require — main.lua appelle DevConsole.draw() chaque frame.
--
-- API :
--   DevConsole.open()                         ouvre la console
--   DevConsole.close()                        ferme la console
--   DevConsole.toggle()                       bascule
--   DevConsole.isOpen()                       → bool
--   DevConsole.print(text)                    ajoute une ligne de sortie
--   DevConsole.register(name, fn, help)       enregistre une commande
--   DevConsole.execute(raw)                   exécute une ligne de commande
--   DevConsole.onKeyDown(key)                 routage clavier — appelé par main.lua quand ouvert
--   DevConsole.draw()                         rendu — appelé par main.lua dans update(dt)
--
-- Le rendu utilise Debug.fillRect / Debug.label (mêmes APIs que scene_test.lua),
-- donc la console s'affiche par-dessus la scène via le pipeline DebugDraw.

local M = {}

-- ── Constantes ───────────────────────────────────────────────────────────────

local MAX_LINES = 16
local LINE_H    = 18
local MARGIN    = 8

-- ── État interne ─────────────────────────────────────────────────────────────

local _open    = false
local _input   = ""
local _lines   = {}
local _cmds    = {}
local _history = {}
local _histIdx = 0

-- ── État ─────────────────────────────────────────────────────────────────────

function M.open()   _open = true  end
function M.close()  _open = false end
function M.toggle() _open = not _open end
function M.isOpen() return _open end

-- ── Sortie ───────────────────────────────────────────────────────────────────

function M.print(text)
    for line in tostring(text):gmatch("[^\n]+") do
        _lines[#_lines + 1] = line
    end
    while #_lines > MAX_LINES * 4 do table.remove(_lines, 1) end
end

-- ── Commandes ────────────────────────────────────────────────────────────────

function M.register(name, fn, help)
    _cmds[name:lower()] = { fn = fn, help = help or "" }
end

function M.execute(raw)
    raw = (raw or ""):match("^%s*(.-)%s*$")
    if raw == "" then return end
    M.print("> " .. raw)
    table.insert(_history, 1, raw)
    if #_history > 50 then table.remove(_history) end
    _histIdx = 0

    local parts = {}
    for tok in raw:gmatch("%S+") do parts[#parts + 1] = tok end
    local name = parts[1] and parts[1]:lower() or ""
    table.remove(parts, 1)

    local cmd = _cmds[name]
    if cmd then
        local ok, err = pcall(cmd.fn, parts)
        if not ok then M.print("  Erreur : " .. tostring(err)) end
    else
        M.print("  Commande inconnue : '" .. name .. "' (tape 'help')")
    end
end

-- ── Saisie ──────────────────────────────────────────────────────────────────
-- Mapping touche → caractère pour A-Z, 0-9, Space (limite imposée par KeyCode).
-- La ponctuation est inaccessible avec ce mapping mais les commandes built-in
-- n'en ont pas besoin (lettres, chiffres, espaces uniquement).

local function keyToChar(key)
    local shift = Input.isKeyPressed("LShift") or Input.isKeyPressed("RShift")
    if #key == 1 and key >= "A" and key <= "Z" then
        return shift and key or key:lower()
    end
    if #key == 1 and key >= "0" and key <= "9" then
        if shift then
            local d = { ["1"]="!", ["2"]="@", ["3"]="#", ["4"]="$", ["5"]="%",
                        ["6"]="^", ["7"]="&", ["8"]="*", ["9"]="(", ["0"]=")" }
            return d[key] or key
        end
        return key
    end
    if key == "Space" then return " " end
    return nil
end

function M.onKeyDown(key)
    if not _open then return end

    if key == "Escape" then
        M.close()
    elseif key == "Enter" then
        M.execute(_input)
        _input   = ""
        _histIdx = 0
    elseif key == "Backspace" then
        if #_input > 0 then _input = _input:sub(1, -2) end
    elseif key == "Up" then
        if _histIdx < #_history then
            _histIdx = _histIdx + 1
            _input   = _history[_histIdx]
        end
    elseif key == "Down" then
        if _histIdx > 1 then
            _histIdx = _histIdx - 1
            _input   = _history[_histIdx]
        elseif _histIdx == 1 then
            _histIdx = 0
            _input   = ""
        end
    else
        local ch = keyToChar(key)
        if ch then _input = _input .. ch end
    end
end

-- ── Rendu ────────────────────────────────────────────────────────────────────
-- IMPORTANT : Color.new(...) — le call direct Color(...) ne fonctionne pas
-- avec la version de sol2 utilisée. Tous les scripts qui marchent (scene_test,
-- intro, particles) utilisent Color.new — on suit la même convention.

local COL_BG    = Color.new(0,   0,   0,   210)
local COL_HINT  = Color.new(150, 150, 150, 255)
local COL_OUT   = Color.new(120, 220, 120, 255)
local COL_INPUT = Color.new(255, 240, 100, 255)

function M.draw()
    if not _open then return end

    local ws     = Viewport.getWindowSize()
    local sw     = ws.x
    local panelH = MARGIN + LINE_H + MAX_LINES * LINE_H + LINE_H + MARGIN

    Debug.fillRect(0, 0, sw, panelH, COL_BG, 0)
    Debug.label(MARGIN, MARGIN, "DEV CONSOLE  [Echap pour fermer]", COL_HINT, 0)

    local startIdx = math.max(1, #_lines - MAX_LINES + 1)
    for i = 0, MAX_LINES - 1 do
        local line = _lines[startIdx + i]
        if line then
            Debug.label(MARGIN, MARGIN + LINE_H + i * LINE_H, line, COL_OUT, 0)
        end
    end

    local inputY = MARGIN + LINE_H + MAX_LINES * LINE_H
    Debug.label(MARGIN, inputY, "> " .. _input .. "_", COL_INPUT, 0)
end

-- ── Commandes built-in ───────────────────────────────────────────────────────

M.register("help", function(args)
    local name = args[1] and args[1]:lower()
    if name and _cmds[name] then
        M.print("  " .. name .. " : " .. _cmds[name].help)
    else
        local names = {}
        for k in pairs(_cmds) do names[#names + 1] = k end
        table.sort(names)
        M.print("Commandes : " .. table.concat(names, ", "))
    end
end, "help [cmd] — liste les commandes disponibles")

M.register("clear", function()
    _lines = {}
end, "clear — vide la console")

M.register("lua", function(args)
    local code = table.concat(args, " ")
    if code == "" then M.print("  Usage : lua <code>"); return end
    local fn, err = load(code, "console", "t", _G)
    if not fn then M.print("  Syntaxe : " .. tostring(err)); return end
    local ok, result = pcall(fn)
    if not ok then
        M.print("  Erreur : " .. tostring(result))
    elseif result ~= nil then
        M.print("  = " .. tostring(result))
    end
end, "lua <code> — évalue du code Lua dans _G")

M.register("test", function()
    if RunTests then
        RunTests()
        M.print("  Tests lancés.")
    else
        M.print("  RunTests non disponible.")
    end
end, "test — lance les tests en jeu")

M.register("fx", function(args)
    local map = {
        ssao     = function() PostFX.toggleSSAO()     end,
        bloom    = function() PostFX.toggleBloom()    end,
        grading  = function() PostFX.toggleGrading()  end,
        lighting = function() PostFX.toggleLighting() end,
    }
    local name = args[1] and args[1]:lower()
    if name and map[name] then
        map[name]()
        M.print("  PostFX '" .. name .. "' basculé.")
    else
        M.print("  fx <ssao|bloom|grading|lighting>")
    end
end, "fx <effet> — bascule un PostFX")

M.register("time", function(args)
    local h = tonumber(args[1])
    if h then
        Time.setHour(math.floor(h) % 24)
        M.print("  Heure : " .. tostring(math.floor(h) % 24) .. "h")
    else
        M.print("  Heure courante : " .. tostring(Time.getHour()) .. "h")
    end
end, "time [h] — affiche ou change l'heure (0-23)")

M.register("timescale", function(args)
    local s = tonumber(args[1])
    if s then
        Game.setTimescale(s)
        M.print("  Timescale : " .. s)
    else
        M.print("  Timescale : " .. tostring(Game.getTimescale()))
    end
end, "timescale [val] — vitesse du temps (0=pause, 1=normal)")

M.register("pause",  function() Game.setTimescale(0); M.print("  Pause")    end, "pause — fige le temps")
M.register("resume", function() Game.setTimescale(1); M.print("  Reprise") end, "resume — reprend le temps")

M.register("scene", function(args)
    local name = args[1]
    if name then
        Scene.load(name)
        M.print("  Chargement de la scène : " .. name)
    else
        M.print("  Scène courante : " .. tostring(Scene.current()))
    end
end, "scene [nom] — affiche ou charge une scène")

M.register("tp", function(args)
    local x, y = tonumber(args[1]), tonumber(args[2])
    if not x or not y then
        M.print("  Usage : tp <x> <y>")
        return
    end
    local p = World.findByTag("player")
    if not p then M.print("  Joueur introuvable."); return end
    World.setPosition(p, x, y)
    M.print(string.format("  Téléporté en (%.0f, %.0f)", x, y))
end, "tp <x> <y> — téléporte le joueur")

M.register("reload", function(args)
    local id = tonumber(args[1])
    if id then
        ScriptRegistry.reload(id)
        M.print("  Script entité " .. id .. " rechargé.")
    else
        ScriptRegistry.reloadAll()
        M.print("  Tous les scripts rechargés.")
    end
end, "reload [entityId] — recharge un script")

M.register("stats", function(args)
    local tag = args[1] or "player"
    local entity = World.findByTag(tag)
    if not entity then
        M.print("  Entité introuvable : '" .. tag .. "'")
        return
    end
    M.print("  '" .. tag .. "'  id=" .. tostring(entity.id))
    local pos = World.getPosition(entity)
    if pos then
        M.print(string.format("    position = (%.1f, %.1f)", pos.x, pos.y))
    end
end, "stats [tag] — info sur une entité (défaut: player)")

M.print("Console prête. Tape 'help' pour la liste des commandes.")
return M
