-- scene_test.lua
-- Script de scène chargé automatiquement quand "Test Scene" devient active.
-- Affiche un panneau de confirmation à l'écran via le module Debug.

local LINE_H    = 22
local PAD       = 10
local PANEL_W   = 340

local _lines = {
    { text = "scene_test.lua — OK",          color = Color.new(80, 220, 80)   },
    { text = "Script de scène chargé !",      color = Color.new(200, 200, 200) },
    { text = "Scene : Test Scene",            color = Color.new(160, 160, 255) },
}

local _elapsed = 0.0
local _shown   = true

function init()
    Log.info("[scene_test] init() appelé — script de scène actif")
end

function update(dt)
    _elapsed = _elapsed + dt

    if not _shown then return end

    local ws     = Viewport.getWindowSize()
    local panelH = #_lines * LINE_H + PAD * 2
    local panelX = 16
    local panelY = 16

    Debug.fillRect(panelX, panelY, PANEL_W, panelH, Color.new(0, 0, 0, 200), 0)
    Debug.drawRect(panelX, panelY, PANEL_W, panelH, Color.new(80, 200, 80, 200), 0)

    for i, line in ipairs(_lines) do
        Debug.label(panelX + PAD, panelY + PAD + (i - 1) * LINE_H, line.text, line.color, 0)
    end

    -- Masque le panneau après 8 secondes
    if _elapsed >= 8.0 then
        _shown = false
    end
end
