-- data/scripts/main.lua
-- Script global du jeu — logique de jeu non liée à une entité spécifique.
-- Chargé automatiquement par Game.cpp au démarrage.
--
-- Gère :
--   E        — interaction avec les PNJ / avance le dialogue
--   T        — avance le temps d'une heure
--   N        — ouvre/ferme la console développeur (DevConsole)
--
-- Les effets PostFX, les tests et les autres outils développeur sont
-- accessibles depuis la console (touche N → tape 'help').
--
-- Utilise les named handlers auto-câblés par __wireNamedHandlers(env, 0).
-- Pour les contrôles remappables, utilise InputBind.

-- ── Bindings de contrôle ───────────────────────────────────────────────────
InputBind.register("interact",   "E")
InputBind.register("timeAdvance","T")
InputBind.register("devConsole", "N")

local IngameTest = require("tests/test_ingame")

-- Exposé globalement pour que DevConsole puisse l'appeler via 'test'
RunTests = function() IngameTest.runAll() end

-- Portée d'interaction avec un PNJ (en pixels)
local INTERACT_RANGE = 100

-- Si le moteur n'a pas préchargé DevConsole (ancienne version de l'EXE), le charger ici.
-- On écrit dans _G pour que debug.lua (env global) puisse appeler DevConsole._flush().
if not DevConsole then
    local ok, m = pcall(require, "nova/dev_console")
    if ok and type(m) == "table" then
        _G.DevConsole = m
        Log.info("[main] DevConsole chargé manuellement OK")
    else
        Log.error("[main] DevConsole ECHEC : " .. tostring(m))
    end
else
    Log.info("[main] DevConsole déjà présent via moteur")
end
Log.info("[main] DevConsole apres init = " .. tostring(type(DevConsole)))

-- ── Named handlers ─────────────────────────────────────────────────────────

-- Appelé pour toute touche pressée (OnKeyDown est câblé par __wireNamedHandlers)
function OnKeyDown(key)
    Log.info("[main] OnKeyDown = " .. tostring(key))
    if not DevConsole then
        Log.error("[main] OnKeyDown: DevConsole est nil !")
        return
    end

    -- La console développeur capture toute la saisie quand elle est ouverte
    if DevConsole.isOpen() then
        DevConsole.onKeyDown(key)
        return
    end

    -- Ouvre/ferme la console développeur (touche N)
    if key == InputBind.getKey("devConsole") then
        DevConsole.toggle()
        return
    end

    -- Interaction / dialogue (touche E)
    if key == InputBind.getKey("interact") then
        if Dialogue.isActive() then
            Dialogue.advance()
        else
            local player = World.findByTag("player")
            if player then
                local npc, dist = World.nearest(player, "npc")
                if npc and dist <= INTERACT_RANGE then
                    Dialogue.start(npc.id)
                end
            end
        end

    -- Avance le temps d'une heure (touche T)
    elseif key == InputBind.getKey("timeAdvance") then
        Time.advance(1)
        Log.info("Heure : " .. tostring(Time.getHour()) .. "h00")
    end
end

-- ── Logique de mise à jour ─────────────────────────────────────────────────
-- Appelé chaque frame par ScriptSystem
function update(dt)
    IngameTest.update(dt)
end
