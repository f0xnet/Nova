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

-- ── Bindings de contrôle ───────────────────────────────────────────────────
Log.info("[main] début du chargement de main.lua")

InputBind.register("interact",   "E")
InputBind.register("timeAdvance","T")
InputBind.register("devConsole", "N")
Log.info("[main] InputBind registered (interact=E, timeAdvance=T, devConsole=N)")

local IngameTest = require("tests/test_ingame")
Log.info("[main] IngameTest required: " .. type(IngameTest))

local DevConsole = require("nova/dev_console")
Log.info("[main] DevConsole required: " .. type(DevConsole) .. " isOpen=" .. type(DevConsole and DevConsole.isOpen))

-- Exposé globalement pour la commande 'test' de la console
RunTests = function() IngameTest.runAll() end

-- Portée d'interaction avec un PNJ (en pixels)
local INTERACT_RANGE = 100

-- ── Named handlers ─────────────────────────────────────────────────────────

function OnKeyDown(key)
    Log.info("[main] OnKeyDown: key='" .. tostring(key) .. "' DevConsole.isOpen()=" .. tostring(DevConsole.isOpen()))

    -- La console capture toute la saisie quand elle est ouverte
    if DevConsole.isOpen() then
        DevConsole.onKeyDown(key)
        return
    end

    -- Ouvre la console (touche N)
    if key == InputBind.getKey("devConsole") then
        Log.info("[main] OnKeyDown: touche console détectée, toggle")
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
local _updateLogCount = 0
function update(dt)
    if _updateLogCount < 3 then
        _updateLogCount = _updateLogCount + 1
        Log.info("[main] update() frame#" .. _updateLogCount .. " dt=" .. tostring(dt))
    end
    IngameTest.update(dt)
    DevConsole.draw()
end

Log.info("[main] fin du chargement de main.lua")
