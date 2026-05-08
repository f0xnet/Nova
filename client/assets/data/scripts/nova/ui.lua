-- nova/ui.lua
-- Couche haut niveau pour l'interface utilisateur.
-- Enrichit le global UI (C++) avec callbacks, chargement et raccourcis.
-- Chargé APRÈS que Game.cpp a initialisé lua["UI"] (post-init).
--
-- API ajoutée au-dessus de l'UI C++ :
--
-- Réaction aux actions (clic bouton, action déclenchée) :
--   UI.onClick(componentId, fn)       -- fn() appelé quand ce composant déclenche une action
--   UI.onAction(action, fn)           -- fn(value, id) appelé pour cette action spécifique
--   UI.on(event, fn)                  -- EventBus.on("ui_" .. event, fn) — générique
--   UI.off(event)                     -- EventBus.off("ui_" .. event)
--
-- Lecture/écriture valeurs :
--   UI.getValue(id)     → string | float | nil   (TextInput/Slider/Button)
--   UI.setValue(id, v)  → (Slider=float, Button=string)
--   UI.getText(id)      → string   (composant Text)
--   UI.setText(id, s)   — déjà présent depuis C++
--
-- Chargement layouts :
--   UI.load(path)       → bool   (charge un JSON d'UI au runtime)
--   UI.removeUI(id)     — décharge un layout
--   UI.removeGroup(id)  — supprime un groupe
--   UI.clear()          — supprime tous les composants
--
-- Visibilité (déjà présents, alias ajoutés) :
--   UI.show(id)   → alias UI.setVisible(id, true)
--   UI.hide(id)   → alias UI.setVisible(id, false)
--
-- Named handler (auto-câblé par __wireNamedHandlers) :
--   function OnUIAction(action, value, id) end
--
-- EventBus events reçus (émis par le C++) :
--   "ui_action"            { action, value, id }
--   "ui_action_<action>"   { value, id }
--   "ui_click_<id>"        { action, value }
--
-- Exemple :
--   -- Bouton "play" → transition vers la scène principale
--   UI.load("data/ui/json/main_menu.json")
--   UI.onAction("play", function(value, id)
--       Scene.transition("world_01")
--   end)
--
--   -- Slider volume → applique immédiatement
--   UI.onClick("slider_volume", function()
--       local vol = UI.getValue("slider_volume")
--       Audio.setVolume(vol)
--   end)
--
--   -- Named handler global (dans n'importe quel script)
--   function OnUIAction(action, value, id)
--       if action == "quit" then
--           -- cleanup avant quitter
--       end
--   end
--
--   -- HUD dynamique
--   UI.load("data/ui/json/hud.json")
--   UI.setText("label_hp", "HP: " .. Stats.getStat(player.id, "health", 100))

-- Capture le tableau UI C++ AVANT de le remplacer
local _ui = UI

-- Crée le nouveau tableau enrichi (hérite toutes les méthodes C++ via __index)
local UI_module = setmetatable({}, {
    __index = function(_, k) return _ui[k] end,
    __newindex = function(_, k, v) _ui[k] = v end,
})

-- ── Raccourcis visibilité ─────────────────────────────────────────────────

function UI_module.show(id)
    _ui.setVisible(id, true)
end

function UI_module.hide(id)
    _ui.setVisible(id, false)
end

-- ── Callbacks actions / clics ──────────────────────────────────────────────

-- fn() quand le composant <id> déclenche une action (clic bouton, etc.)
function UI_module.onClick(componentId, fn)
    EventBus.on("ui_click_" .. componentId, function(data)
        fn(data.value, data.action)
    end)
end

-- fn(value, componentId) pour une action nommée spécifique
function UI_module.onAction(action, fn)
    EventBus.on("ui_action_" .. action, function(data)
        fn(data.value, data.id)
    end)
end

-- EventBus générique préfixé "ui_"
function UI_module.on(event, fn)
    EventBus.on("ui_" .. event, fn)
end

function UI_module.off(event, fn)
    if fn then
        EventBus.off("ui_" .. event, fn)
    else
        EventBus.clear("ui_" .. event)
    end
end

-- ── Alias getText (pointe vers getString C++) ─────────────────────────────

function UI_module.getText(id)
    return _ui.getString(id)
end

return UI_module
