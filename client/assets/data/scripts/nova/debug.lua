-- nova/debug.lua
-- Overlay de débogage visuel + outils d'inspection et d'assertion.
-- Disponible sans require : Debug est un global auto-chargé.
-- Désactiver en production : Debug.enabled = false
--
-- API visuelle :
--   Debug.enabled                            -- active/désactive (défaut: true)
--   Debug.drawRect(x, y, w, h, color, dur)  -- rect wireframe (dur en secondes, défaut 0=1 frame)
--   Debug.fillRect(x, y, w, h, color, dur)  -- rect rempli
--   Debug.drawLine(x1, y1, x2, y2, color, dur, thickness)
--   Debug.drawCircle(x, y, r, color, dur)   -- cercle (approximé par polygon)
--   Debug.label(x, y, text, color, dur)     -- texte
--   Debug.watch(key, value)                 -- affiche une valeur en coin d'écran (jusqu'au prochain frame)
--   Debug.clear()                           -- efface toutes les commandes en attente
--   Debug._flush()                          -- dessine et décrémente les durées (appelé par ScriptSystem)
--
-- API d'inspection :
--   Debug.traceback(msg, level)             -- pile d'appel Lua sous forme de string
--   Debug.inspect(val, depth)              -- sérialise n'importe quelle valeur en string lisible
--   Debug.assert(cond, msg)               -- assertion avec traceback intégré dans l'erreur
--
-- Coordonnées en espace écran (pixels, origin en haut-gauche de la fenêtre).
-- Utilise DebugDraw (binding C++) pour les draw calls réels.
--
-- Exemple :
--   -- Visualiser un trigger
--   Debug.drawRect(zone.x, zone.y, zone.w, zone.h, Color.Green, 0)
--
--   -- Afficher les stats d'une entité
--   Debug.watch("hp",    Stats.getStat(player.id, "health", 0))
--   Debug.watch("state", stateMachine:getState())
--
--   -- Tracer un chemin Nav
--   for i = 1, #path - 1 do
--       Debug.drawLine(path[i].x, path[i].y, path[i+1].x, path[i+1].y, Color.Yellow, 0)
--   end

local Debug = {}

Debug.enabled = true

local _commands = {}  -- { type, args..., timer }
local _watches  = {}  -- key → tostring(value)  (cleared each flush)

local function _cmd(t)
    _commands[#_commands + 1] = t
end

function Debug.drawRect(x, y, w, h, color, duration)
    if not Debug.enabled then return end
    _cmd({ k="rect", x=x, y=y, w=w, h=h, color=color, filled=false,
           timer=(duration or 0) })
end

function Debug.fillRect(x, y, w, h, color, duration)
    if not Debug.enabled then return end
    _cmd({ k="rect", x=x, y=y, w=w, h=h, color=color, filled=true,
           timer=(duration or 0) })
end

function Debug.drawLine(x1, y1, x2, y2, color, duration, thickness)
    if not Debug.enabled then return end
    _cmd({ k="line", x1=x1, y1=y1, x2=x2, y2=y2, color=color,
           thickness=(thickness or 1), timer=(duration or 0) })
end

-- Cercle approximé par N segments de ligne
function Debug.drawCircle(x, y, r, color, duration, segments)
    if not Debug.enabled then return end
    segments = segments or 16
    local dur = duration or 0
    local step = (2 * math.pi) / segments
    for i = 0, segments - 1 do
        local a1 = i * step
        local a2 = (i + 1) * step
        _cmd({ k="line",
               x1 = x + math.cos(a1) * r, y1 = y + math.sin(a1) * r,
               x2 = x + math.cos(a2) * r, y2 = y + math.sin(a2) * r,
               color = color, thickness = 1, timer = dur })
    end
end

function Debug.label(x, y, text, color, duration)
    if not Debug.enabled then return end
    _cmd({ k="text", x=x, y=y, text=tostring(text), color=color,
           timer=(duration or 0) })
end

-- Affiche une valeur en "watch window" (coin haut-gauche)
-- Cleared automatically after each flush (use in update() every frame)
function Debug.watch(key, value)
    if not Debug.enabled then return end
    _watches[key] = tostring(value)
end

function Debug.clear()
    _commands = {}
    _watches  = {}
end

-- Appelé par ScriptSystem.renderDebug() pendant la phase de rendu
function Debug._flush()
    if not Debug.enabled then return end

    -- Dessine les commandes persistantes (durée > 0) + immédiates (durée == 0)
    local remaining = {}
    for _, cmd in ipairs(_commands) do
        if cmd.k == "rect" then
            DebugDraw.rect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color, cmd.filled)
        elseif cmd.k == "line" then
            DebugDraw.line(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color, cmd.thickness)
        elseif cmd.k == "text" then
            DebugDraw.text(cmd.x, cmd.y, cmd.text, cmd.color)
        end

        if cmd.timer > 0 then
            -- Décrémenter — ScriptSystem passe dt=0 à _flush, pas de dt disponible ici
            -- La durée sera decrementée au prochain _update via _tickCommands
            remaining[#remaining + 1] = cmd
        end
        -- timer == 0 → one-shot, pas conservé
    end
    _commands = remaining

    -- Affiche les watches en haut-gauche (30px de marge, 18px par ligne)
    local margin = 10
    local lineH  = 18
    local row    = 0
    for key, val in pairs(_watches) do
        local label = key .. " = " .. val
        DebugDraw.text(margin, margin + row * lineH, label, nil)  -- nil → blanc
        row = row + 1
    end
    _watches = {}
end

-- Appelé par ScriptSystem._update (via updateNovaRuntime) pour décrémenter les timers
function Debug._update(dt)
    if not Debug.enabled then return end
    for _, cmd in ipairs(_commands) do
        if cmd.timer > 0 then
            cmd.timer = cmd.timer - dt
            if cmd.timer < 0 then cmd.timer = 0 end
        end
    end
end

-- ─── Inspection ───────────────────────────────────────────────────────────────

-- Retourne la pile d'appel Lua sous forme de string.
-- msg    : message préfixé (optionnel)
-- level  : niveau de départ dans la pile (défaut 2 = l'appelant de traceback)
function Debug.traceback(msg, level)
    if type(debug) == "table" and debug.traceback then
        return debug.traceback(msg or "", (level or 1) + 1)
    end
    return msg or "(traceback non disponible — lib debug non chargée)"
end

-- Sérialise n'importe quelle valeur en string lisible.
-- depth : profondeur max pour les tables (défaut 3)
function Debug.inspect(val, depth, _indent)
    depth   = depth   or 3
    _indent = _indent or ""
    local t = type(val)
    if     t == "nil"      then return "nil"
    elseif t == "boolean"  then return tostring(val)
    elseif t == "number"   then
        return (math.floor(val) == val) and tostring(math.floor(val))
                                        or string.format("%.4g", val)
    elseif t == "string"   then return string.format("%q", val)
    elseif t == "function" then return "<function>"
    elseif t == "userdata" then return "<userdata>"
    elseif t == "table"    then
        if depth <= 0 then return "{…}" end
        local parts = {}
        local ni    = _indent .. "  "
        for k, v in pairs(val) do
            local key = (type(k) == "string") and k or ("[" .. tostring(k) .. "]")
            table.insert(parts, ni .. key .. " = " .. Debug.inspect(v, depth - 1, ni))
        end
        if #parts == 0 then return "{}" end
        return "{\n" .. table.concat(parts, ",\n") .. "\n" .. _indent .. "}"
    end
    return tostring(val)
end

-- Assertion avec traceback intégré dans le message d'erreur.
-- Si cond est faux, loggue l'erreur ET lève une erreur Lua.
function Debug.assert(cond, msg)
    if not cond then
        local tb = Debug.traceback(msg or "assertion échouée", 2)
        Log.error("[Debug.assert] " .. tb)
        error(tb, 2)
    end
end

return Debug
