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
InputBind.register("interact",   "E")
InputBind.register("timeAdvance","T")
InputBind.register("devConsole", "N")

local IngameTest = require("tests/test_ingame")
local DevConsole = require("nova/dev_console")

-- Exposé globalement pour la commande 'test' de la console
RunTests = function() IngameTest.runAll() end

-- Portée d'interaction avec un PNJ (en pixels)
local INTERACT_RANGE = 100

-- ── Named handlers ─────────────────────────────────────────────────────────

function OnKeyDown(key)
    -- La console capture toute la saisie quand elle est ouverte
    if DevConsole.isOpen() then
        DevConsole.onKeyDown(key)
        return
    end

    -- Ouvre la console (touche N)
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
    DevConsole.draw()
end
