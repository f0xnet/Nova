-- intro.lua
-- Séquence d'intro : fondu noir → logo → pause → fondu noir → menu principal.
--
-- Assets attendus :
--   data/music/intro.ogg              musique d'intro (non loopée)
--   data/ui/json/intro.json           layout UI avec le logo
--
-- Durées :
--   FADE_DURATION  secondes pour chaque fondu (entrée / sortie)
--   HOLD_DURATION  secondes d'affichage du logo

local FADE_DURATION = 1.5
local HOLD_DURATION = 3.0

function init()
    -- On commence avec un overlay noir instantané, puis on révèle le logo.
    -- SceneFX.fadeIn(0) → alpha 255 immédiatement (noir plein écran).
    -- SceneFX.fadeOut  → retire l'overlay et révèle la scène derrière.
    SceneFX.fadeIn(0.01, function()

        Sound.playMusic("data/music/intro.ogg", false)
        UI.load("data/ui/json/intro.json")

        -- Révèle le logo
        SceneFX.fadeOut(FADE_DURATION, function()

            -- Pause : logo affiché
            Timer.after(HOLD_DURATION, function()

                -- Fondu audio + fondu visuel simultanés
                Sound.fadeMusic(0, FADE_DURATION)
                SceneFX.fadeIn(FADE_DURATION, function()

                    -- Écran noir : décharge l'UI d'intro, charge le menu
                    UI.clear()
                    Scene.load("data/scenes/menu.scene", "menu")
                    Scene.setActive("menu")
                    -- Le menu est maintenant actif (menu.lua init() appelé).
                    -- SceneFX reste noir ; menu.lua fait le fadeOut lui-même.

                end)
            end)
        end)
    end)
end

function update(dt) end
