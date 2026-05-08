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

function init()
    -- Révèle le menu (l'écran était noir en sortie d'intro ou de partie)
    SceneFX.fadeOut(0.8)

    Sound.playMusic("data/music/menu.ogg", true)
    UI.load("data/ui/json/main_menu.json")

    -- ── Nouvelle Partie ──────────────────────────────────────────────────────
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

    -- ── Continuer ────────────────────────────────────────────────────────────
    UI.onAction("continue", function()
        -- TODO : vérifier qu'une sauvegarde existe, puis charger la scène sauvegardée
        Log.info("[menu] Continue — non implémenté")
    end)

    -- ── Options ──────────────────────────────────────────────────────────────
    UI.onAction("options", function()
        -- TODO : charger et afficher le panneau d'options
        Log.info("[menu] Options — non implémenté")
    end)
end

function update(dt) end
