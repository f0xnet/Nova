-- nova/dev_console.lua
-- Console développeur in-game, ouvrable avec N.
-- Disponible sans require : DevConsole est un global auto-chargé.
--
-- API :
--   DevConsole.open()                      ouvre la console
--   DevConsole.close()                     ferme la console
--   DevConsole.toggle()                    bascule ouvert/fermé
--   DevConsole.isOpen()                    retourne l'état courant
--   DevConsole.print(text)                 ajoute une ligne de sortie
--   DevConsole.register(name, fn, help)    enregistre une commande custom
--   DevConsole.execute(raw)                exécute une ligne de commande brute
--   DevConsole.onKeyDown(key)              à appeler depuis OnKeyDown quand la console est ouverte
--   DevConsole._flush()                    rendu via DebugDraw (appelé depuis Debug._flush)
--
-- Intégration dans main.lua :
--   function OnKeyDown(key)
--       if DevConsole.isOpen() then DevConsole.onKeyDown(key); return end
--       ...
--   end
--
-- Les caractères imprimables (lettres, chiffres, ponctuation, espace) sont reçus
-- via l'événement EventBus "text_entered" alimenté par les events TextEntered SFML.
-- Les touches spéciales (Enter, Backspace, Escape, flèches) passent par OnKeyDown.

local DevConsole = {}

-- ── Constantes ──────────────────────────────────────────────────────────────

local MAX_LINES = 16
local LINE_H    = 18
local MARGIN    = 8

-- ── État interne ─────────────────────────────────────────────────────────────

local _open    = false
local _lines   = {}
local _input   = ""
local _cmds    = {}
local _history = {}
local _histIdx = 0

-- ── API publique ──────────────────────────────────────────────────────────────

function DevConsole.open()   _open = true  end
function DevConsole.close()  _open = false end
function DevConsole.toggle() _open = not _open end
function DevConsole.isOpen() return _open end

function DevConsole.print(text)
    for line in tostring(text):gmatch("[^\n]+") do
        _lines[#_lines + 1] = line
    end
    while #_lines > MAX_LINES * 4 do table.remove(_lines, 1) end
end

function DevConsole.register(name, fn, help)
    _cmds[name:lower()] = { fn = fn, help = help or "" }
end

function DevConsole.execute(raw)
    raw = (raw or ""):match("^%s*(.-)%s*$")
    if raw == "" then return end
    DevConsole.print("> " .. raw)
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
        if not ok then DevConsole.print("  Erreur : " .. tostring(err)) end
    else
        DevConsole.print("  Commande inconnue : '" .. name .. "' (tape 'help')")
    end
end

-- _textEnteredWired : devient true dès que le moteur envoie le premier event text_entered.
-- Quand true, la saisie passe exclusivement par text_entered (ponctuation incluse).
-- Quand false (ancien EXE), le fallback alphanumérique OnKeyDown prend le relais.
local _textEnteredWired = false

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

-- Gère les touches spéciales (Enter, Backspace, Escape, flèches).
-- Sur ancien EXE : aussi le fallback alphanumérique (A-Z, 0-9, Space).
function DevConsole.onKeyDown(key)
    if not _open then return end

    if key == "Escape" then
        DevConsole.close()
    elseif key == "Enter" then
        DevConsole.execute(_input)
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
    elseif not _textEnteredWired then
        -- Fallback alphanumérique désactivé dès que text_entered SFML est détecté
        local ch = keyToChar(key)
        if ch then _input = _input .. ch end
    end
end

-- Reçoit les caractères imprimables depuis les events TextEntered SFML (ponctuation incluse).
-- Désactive le fallback alphanumérique dès le premier caractère reçu.
EventBus.on("text_entered", function(data)
    _textEnteredWired = true
    if _open and data and data.char then
        _input = _input .. data.char
    end
end)

-- ── Rendu via DebugDraw ───────────────────────────────────────────────────────

-- Initialisés au premier appel de _flush() pour éviter tout échec au chargement du module.
local COL_BG, COL_HINT, COL_OUT, COL_INPUT
local function ensureColors()
    if COL_BG then return end
    COL_BG    = Color(0,   0,   0,   210)
    COL_HINT  = Color(150, 150, 150, 255)
    COL_OUT   = Color.Green
    COL_INPUT = Color.Yellow
end

function DevConsole._flush()
    if not _open then return end
    ensureColors()

    local sz     = Viewport.getWindowSize()
    local sw     = sz.x
    local panelH = MARGIN + LINE_H + MAX_LINES * LINE_H + LINE_H + MARGIN

    DebugDraw.rect(0, 0, sw, panelH, COL_BG, true)
    DebugDraw.text(MARGIN, MARGIN, "DEV CONSOLE  [Echap pour fermer]", COL_HINT)

    local startIdx = math.max(1, #_lines - MAX_LINES + 1)
    for i = 0, MAX_LINES - 1 do
        local line = _lines[startIdx + i]
        if line then
            DebugDraw.text(MARGIN, MARGIN + LINE_H + i * LINE_H, line, COL_OUT)
        end
    end

    local inputY = MARGIN + LINE_H + MAX_LINES * LINE_H
    DebugDraw.text(MARGIN, inputY, "> " .. _input .. "_", COL_INPUT)
end

function DevConsole._update(dt) end

-- ── Commandes built-in ────────────────────────────────────────────────────────

DevConsole.register("help", function(args)
    local name = args[1] and args[1]:lower()
    if name and _cmds[name] then
        DevConsole.print("  " .. name .. " : " .. _cmds[name].help)
    else
        local names = {}
        for k in pairs(_cmds) do names[#names + 1] = k end
        table.sort(names)
        DevConsole.print("Commandes : " .. table.concat(names, ", "))
    end
end, "help [cmd] — liste les commandes disponibles")

DevConsole.register("clear", function()
    _lines = {}
end, "clear — vide la console")

DevConsole.register("reload", function(args)
    local id = tonumber(args[1])
    if id then
        ScriptRegistry.reload(id)
        DevConsole.print("  Script entité " .. id .. " rechargé.")
    else
        ScriptRegistry.reloadAll()
        DevConsole.print("  Tous les scripts rechargés.")
    end
end, "reload [entityId] — recharge un script d'entité ou tous les scripts")

DevConsole.register("lua", function(args)
    local code = table.concat(args, " ")
    if code == "" then DevConsole.print("  Usage : lua <code>"); return end
    local fn, err = load(code, "console", "t", _G)
    if not fn then DevConsole.print("  Syntaxe : " .. tostring(err)); return end
    local ok, result = pcall(fn)
    if not ok then
        DevConsole.print("  Erreur : " .. tostring(result))
    elseif result ~= nil then
        DevConsole.print("  = " .. tostring(result))
    end
end, "lua <code> — évalue du code Lua dans l'environnement global")

DevConsole.register("test", function()
    if RunTests then
        RunTests()
        DevConsole.print("  Tests lancés.")
    else
        DevConsole.print("  RunTests non disponible.")
    end
end, "test — lance les tests en jeu (IngameTest.runAll)")

DevConsole.register("fx", function(args)
    local map = {
        ssao     = function() PostFX.toggleSSAO()     end,
        bloom    = function() PostFX.toggleBloom()    end,
        grading  = function() PostFX.toggleGrading()  end,
        lighting = function() PostFX.toggleLighting() end,
    }
    local name = args[1] and args[1]:lower()
    if name and map[name] then
        map[name]()
        DevConsole.print("  PostFX '" .. name .. "' basculé.")
    else
        DevConsole.print("  fx <ssao|bloom|grading|lighting>")
    end
end, "fx <ssao|bloom|grading|lighting> — bascule un effet PostFX")

DevConsole.register("time", function(args)
    local h = tonumber(args[1])
    if h then
        Time.setHour(math.floor(h) % 24)
        DevConsole.print("  Heure définie à " .. tostring(math.floor(h) % 24) .. "h.")
    else
        DevConsole.print("  Heure courante : " .. tostring(Time.getHour()) .. "h")
    end
end, "time [h] — affiche ou définit l'heure du jeu (0-23)")

DevConsole.register("timescale", function(args)
    local s = tonumber(args[1])
    if s then
        Game.setTimescale(s)
        DevConsole.print("  Timescale : " .. s)
    else
        DevConsole.print("  Timescale courante : " .. tostring(Game.getTimescale()))
    end
end, "timescale [val] — affiche ou change la vitesse du temps (0=pause, 1=normal)")

DevConsole.register("pause", function()
    Game.setTimescale(0)
    DevConsole.print("  Jeu en pause (timescale=0).")
end, "pause — fige le temps du jeu")

DevConsole.register("resume", function()
    Game.setTimescale(1)
    DevConsole.print("  Reprise (timescale=1).")
end, "resume — reprend le jeu à vitesse normale")

DevConsole.register("stats", function(args)
    local tag    = args[1] or "player"
    local entity = World.findByTag(tag)
    if not entity then
        DevConsole.print("  Entité introuvable : '" .. tag .. "'")
        return
    end
    DevConsole.print("  '" .. tag .. "'  id=" .. tostring(entity.id))
    local pos = World.getPosition(entity)
    if pos then
        DevConsole.print(string.format("    position = (%.1f, %.1f)", pos.x, pos.y))
    end
end, "stats [tag] — affiche les informations d'une entité (défaut: player)")

DevConsole.register("entities", function(args)
    local tag = args[1]
    if not tag then
        DevConsole.print("  Usage : entities <tag>")
        return
    end
    local list = World.findAllByTag(tag)
    if not list or #list == 0 then
        DevConsole.print("  Aucune entité avec le tag '" .. tag .. "'.")
        return
    end
    DevConsole.print("  " .. #list .. " entité(s) [" .. tag .. "] :")
    for _, e in ipairs(list) do
        local pos = World.getPosition(e)
        if pos then
            DevConsole.print(string.format("    id=%-6s  (%.0f, %.0f)", tostring(e.id), pos.x, pos.y))
        else
            DevConsole.print("    id=" .. tostring(e.id))
        end
    end
end, "entities <tag> — liste les entités par tag avec leur position")

DevConsole.register("scene", function(args)
    local name = args[1]
    if name then
        Scene.load(name)
        DevConsole.print("  Chargement de la scène : " .. name)
    else
        DevConsole.print("  Scène courante : " .. tostring(Scene.current()))
    end
end, "scene [nom] — affiche ou charge une scène")

DevConsole.register("tp", function(args)
    local x, y = tonumber(args[1]), tonumber(args[2])
    if not x or not y then
        DevConsole.print("  Usage : tp <x> <y>")
        return
    end
    local player = World.findByTag("player")
    if not player then
        DevConsole.print("  Joueur introuvable.")
        return
    end
    World.setPosition(player, x, y)
    DevConsole.print(string.format("  Joueur téléporté en (%.0f, %.0f)", x, y))
end, "tp <x> <y> — téléporte le joueur à la position donnée")

DevConsole.print("Console prête. Tape 'help' pour la liste des commandes.")

return DevConsole
