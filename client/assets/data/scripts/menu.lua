-- menu.lua
-- Menu principal : câble les boutons et lance la scène appropriée.
--
-- Assets attendus :
--   data/music/menu.ogg               musique de menu (loopée)
--   data/ui/json/main_menu.json       layout UI (fond + logo + 3 boutons)
--
-- Actions câblées :
--   new_game  → charge et active test.scene avec fondu
--   continue  → TODO charger la sauvegarde
--   options   → TODO afficher le panneau d'options

local function wireButtons()
    UI.onAction("new_game", function()
        Sound.fadeMusic(0, 0.8)
        Scene.load("data/scenes/test.scene", "test")
        Scene.transition("test", {
            fade     = true,
            duration = 0.8,
            onBeforeChange = function()
                UI.removeUI("main_menu_ui")
            end
        })
    end)

    UI.onAction("continue", function()
        Log.info("[menu] Continue — non implémenté")
    end)

    UI.onAction("options", function()
        Log.info("[menu] Options — non implémenté")
    end)
end

function init()
    if _G._fromIntro then
        -- L'intro contrôle la caméra et le dezoom.
        -- On enregistre la finalisation et on attend qu'elle soit appelée.
        _G._menuFinalize = function()
            Sound.playMusic("data/music/menu.ogg", true)
            UI.load("data/ui/json/main_menu.json")
            wireButtons()
        end
        return
    end

    -- Initialisation normale (accès direct au menu, sans passer par l'intro)
    SceneFX.fadeOut(0.8)
    Sound.playMusic("data/music/menu.ogg", true)
    UI.load("data/ui/json/main_menu.json")
    wireButtons()
end

function update(dt) end
