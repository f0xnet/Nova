-- intro.lua
-- Amorçage terminal Nova — Protocole Zéro.
-- Chaque ligne apparaît en séquence comme un log système.
-- La ligne INSTABLE se corrige elle-même (Nova auto-ajuste).
-- La mention de Rin est clinique, enfouie — invisible au premier run.

local FADE_FINAL = 1.2
local HOLD_END   = 1.8

function init()
    SceneFX.fadeIn(0.01, function()
        Sound.playMusic("data/music/intro.ogg", false)
        UI.load("data/ui/json/intro.json")

        -- Masquer toutes les lignes avant de les révéler une à une
        local ids = {
            "ln_header1", "ln_header2", "ln_sep1",
            "ln_nova",    "ln_proto",   "ln_sep2",
            "ln_init1",   "ln_init2",   "ln_init3", "ln_init4",
            "ln_sep3",    "ln_anom",    "ln_rin1",  "ln_rin2",
            "ln_sep4",    "ln_active",  "ln_integr"
        }
        for _, id in ipairs(ids) do
            UI.setVisible(id, false)
        end

        local function show(id, t)
            Timer.after(t, function() UI.setVisible(id, true) end)
        end

        -- En-tête programme
        show("ln_header1", 0.3)
        show("ln_header2", 0.5)
        show("ln_sep1",    0.8)

        -- Identification système
        show("ln_nova",    1.0)
        show("ln_proto",   1.4)
        show("ln_sep2",    2.0)

        -- Séquence d'initialisation
        show("ln_init1",   2.2)
        show("ln_init2",   2.8)
        show("ln_init3",   3.4)
        show("ln_init4",   4.0)   -- affiche [INSTABLE]

        -- Nova corrige silencieusement
        Timer.after(4.8, function()
            UI.setText("ln_init4", "> STABILITE CONSCIENCE .............. [72%]")
        end)

        show("ln_sep3",   5.4)

        -- Anomalie Rin — ton identique aux autres lignes, aucune emphase
        show("ln_anom",   5.7)
        show("ln_rin1",   6.2)
        show("ln_rin2",   6.7)
        show("ln_sep4",   7.2)

        -- Conclusion
        show("ln_active", 7.5)
        show("ln_integr", 7.9)

        -- Transition vers le menu
        Timer.after(7.9 + HOLD_END, function()
            Sound.fadeMusic(0, FADE_FINAL)
            SceneFX.fadeIn(FADE_FINAL, function()
                UI.clear()
                Scene.load("data/scenes/menu.scene", "menu")
                Scene.setActive("menu")
            end)
        end)
    end)
end

function update(dt) end
