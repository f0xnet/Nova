-- data/scripts/main.lua
-- Script global du jeu — logique de jeu non liée à une entité spécifique.
-- Chargé automatiquement par Game.cpp au démarrage.
--
-- Gère :
--   E        — interaction avec les PNJ / avance le dialogue
--   T        — avance le temps d'une heure
--   1/2/3/4  — bascule les effets PostFX
--
-- Utilise les named handlers auto-câblés par __wireNamedHandlers(env, 0).
-- Pour les contrôles remappables, utilise InputBind.

-- ── Bindings de contrôle ───────────────────────────────────────────────────
InputBind.register("interact", "E")
InputBind.register("timeAdvance", "T")
InputBind.register("toggleSSAO",     "1")
InputBind.register("toggleBloom",    "2")
InputBind.register("toggleGrading",  "3")
InputBind.register("toggleLighting", "4")
InputBind.register("ingameTests",    "F5")

local IngameTest = require("tests/test_ingame")

-- Portée d'interaction avec un PNJ (en pixels)
local INTERACT_RANGE = 100

-- ── Named handlers ─────────────────────────────────────────────────────────

-- Appelé pour toute touche pressée (OnKeyDown est câblé par __wireNamedHandlers)
function OnKeyDown(key)
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

    -- Tests interactifs en jeu (F5)
    elseif key == InputBind.getKey("ingameTests") then
        IngameTest.runAll()

    -- Bascule les effets PostFX (touches 1-4)
    elseif key == InputBind.getKey("toggleSSAO") then
        PostFX.toggleSSAO()
        Log.info("SSAO basculé")
    elseif key == InputBind.getKey("toggleBloom") then
        PostFX.toggleBloom()
        Log.info("Bloom basculé")
    elseif key == InputBind.getKey("toggleGrading") then
        PostFX.toggleGrading()
        Log.info("Color grading basculé")
    elseif key == InputBind.getKey("toggleLighting") then
        PostFX.toggleLighting()
        Log.info("Lighting dynamique basculé")
    end
end

-- ── Logique de mise à jour ─────────────────────────────────────────────────
-- Appelé chaque frame par ScriptSystem
function update(dt)
    -- Placeholder : logique globale future (ex: day-night cycle automatique)
end
