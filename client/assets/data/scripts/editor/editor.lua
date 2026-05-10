-- editor/editor.lua
-- Éditeur de niveau in-game — Ctrl+E pour activer/désactiver.
--
-- Contrôles :
--   Ctrl+E          — toggle éditeur
--   Clic gauche palette — sélectionne un sprite à poser
--   Clic gauche monde  — pose le sprite / sélectionne une entité existante
--   Clic droit + drag  — pan caméra
--   Flèches ↑↓ dans la palette — naviguer
--   WASD / Flèches  — déplace l'entité sélectionnée (d'un pas = snap ou 1px)
--   Delete          — supprime l'entité sélectionnée
--   G               — bascule le snap à la grille (32px)
--   + / -           — zoom
--   Ctrl+S          — sauvegarde la scène courante

local Editor = {}

-- ── État ──────────────────────────────────────────────────────────────────────

local _enabled       = false
local _spriteDefs    = {}     -- id → { texture, width, height, scale, zOrder }
local _spriteList    = {}     -- liste ordonnée des IDs pour la palette
local _paletteIdx    = 1      -- sprite actuellement sélectionné
local _paletteScroll = 0      -- premier sprite visible dans la palette
local _placing       = false  -- mode placement actif
local _placed        = {}     -- { entity, spriteID, zOrder } — entités gérées par l'éditeur
local _selected      = nil    -- entrée _placed sélectionnée
local _snapGrid      = 32     -- 0 = désactivé
local _scenePath     = nil    -- fichier .scene cible pour la sauvegarde
local _sceneName     = "Untitled"
local _saved         = true   -- faux si des changements non sauvegardés existent

-- Pan caméra
local _panning        = false
local _panStartMouse  = { x = 0, y = 0 }
local _panStartCam    = { x = 0, y = 0 }

-- Couleurs
local COL_BG      = Color.new(18,  18,  28,  225)
local COL_ITEM    = Color.new(45,  45,  65,  255)
local COL_SEL_IT  = Color.new(70, 110, 200,  255)
local COL_TEXT    = Color.new(220, 220, 220, 255)
local COL_DIM     = Color.new(130, 130, 150, 255)
local COL_STATUS  = Color.new(18,  18,  28,  210)
local COL_HILIGHT = Color.new(255, 200,  40, 200)
local COL_GHOST   = Color.new(130, 180, 255,  55)
local COL_GHOSTB  = Color.new(130, 180, 255, 200)
local COL_GRID    = Color.new(55,  55,  80,  70)

local PALETTE_W = 220
local ITEM_H    = 26
local VISIBLE   = 22   -- items affichés simultanément dans la palette

-- ── Utilitaires coordonnées ───────────────────────────────────────────────────

local function screenToWorld(sx, sy)
    local ws = Viewport.getWindowSize()
    local c  = Camera.getPosition()
    local z  = Camera.getZoom()
    return c.x + (sx - ws.x * 0.5) / z,
           c.y + (sy - ws.y * 0.5) / z
end

local function worldToScreen(wx, wy)
    local ws = Viewport.getWindowSize()
    local c  = Camera.getPosition()
    local z  = Camera.getZoom()
    return ws.x * 0.5 + (wx - c.x) * z,
           ws.y * 0.5 + (wy - c.y) * z
end

local function snap(v)
    if _snapGrid <= 0 then return v end
    return math.floor(v / _snapGrid + 0.5) * _snapGrid
end

-- ── Chargement des définitions ────────────────────────────────────────────────

local function loadSpriteDefs()
    local data = Resources.loadJSON("data/definitions/Sprites.json")
    if not data or not data.sprites then
        Log.warn("[Editor] Impossible de lire Sprites.json")
        return
    end
    _spriteDefs = {}
    _spriteList = {}
    for _, s in ipairs(data.sprites) do
        _spriteDefs[s.id] = s
        table.insert(_spriteList, s.id)
    end
    Log.info("[Editor] " .. #_spriteList .. " sprites chargés")
end

-- ── Gestion des entités ───────────────────────────────────────────────────────

-- Importe les entités sprite existantes dans la scène (sans ScriptComponent).
local function importSceneEntities()
    _placed = {}
    for _, e in ipairs(Registry:getAllEntities()) do
        local s = e:getSprite()
        if s and s.textureID ~= "" and not e:hasComponent("ScriptComponent")
                                    and _spriteDefs[s.textureID] then
            table.insert(_placed, { entity = e, spriteID = s.textureID, zOrder = s.zOrder })
        end
    end
end

local function spawnSprite(spriteID, wx, wy)
    local def = _spriteDefs[spriteID]
    if not def then
        Log.warn("[Editor] spriteID inconnu : " .. tostring(spriteID))
        return nil
    end

    local e = Registry:createEntity()
    e:addComponent("TransformComponent")
    e:addComponent("SpriteComponent")

    local t  = e:getTransform()
    t.position.x = wx
    t.position.y = wy
    local w = def.width  * (def.scale or 1.0)
    local h = def.height * (def.scale or 1.0)
    t.origin.x = w * 0.5
    t.origin.y = h * 0.5

    local s = e:getSprite()
    s.textureID     = spriteID
    s.textureHandle = Resources.loadTexture(def.texture)
    s.size.x        = w
    s.size.y        = h
    s.zOrder        = def.zOrder or 0

    local entry = { entity = e, spriteID = spriteID, zOrder = def.zOrder or 0 }
    table.insert(_placed, entry)
    _saved = false
    return entry
end

local function removeEntry(entry)
    for i, v in ipairs(_placed) do
        if v == entry then
            World.destroy(v.entity.id)
            table.remove(_placed, i)
            _saved = false
            return
        end
    end
end

-- ── Sauvegarde JSON ───────────────────────────────────────────────────────────

local function jsonStr(s)
    return '"' .. tostring(s):gsub('\\', '\\\\'):gsub('"', '\\"'):gsub('\n', '\\n') .. '"'
end

local function saveScene()
    local path = _scenePath
    if not path then
        Log.warn("[Editor] Aucun chemin de scène défini (Editor.setScene)")
        return false
    end

    local lines = {
        '{',
        '  "name": ' .. jsonStr(_sceneName) .. ',',
        '  "type": "interior",',
        '  "backgroundColor": [0, 0, 0, 255],',
        '  "script": "",',
        '  "entities": [',
    }

    for i, entry in ipairs(_placed) do
        local t = entry.entity:getTransform()
        if t then
            local comma = (i < #_placed) and "," or ""
            table.insert(lines, '    {')
            table.insert(lines, '      "type": "sprite",')
            table.insert(lines, '      "spriteID": ' .. jsonStr(entry.spriteID) .. ',')
            table.insert(lines, string.format('      "x": %d,', math.floor(t.position.x)))
            table.insert(lines, string.format('      "y": %d,', math.floor(t.position.y)))
            table.insert(lines, string.format('      "zOrder": %d', entry.zOrder))
            table.insert(lines, '    }' .. comma)
        end
    end

    table.insert(lines, '  ]')
    table.insert(lines, '}')

    local ok, err = pcall(function()
        local f = assert(io.open(path, "w"), "Impossible d'ouvrir " .. path)
        f:write(table.concat(lines, "\n"))
        f:close()
    end)

    if ok then
        _saved = true
        Log.info("[Editor] Sauvegardé → " .. path)
        return true
    else
        Log.error("[Editor] Sauvegarde échouée : " .. tostring(err))
        return false
    end
end

-- ── API publique ──────────────────────────────────────────────────────────────

function Editor.setScene(path, name)
    _scenePath = path
    _sceneName = name or "Untitled"
end

function Editor.enable()
    if _enabled then return end
    _enabled  = true
    _placing  = false
    _selected = nil
    _panning  = false
    _saved    = true
    Game.setTimescale(0)
    Camera.unfollow()
    if #_spriteList == 0 then loadSpriteDefs() end
    importSceneEntities()
    Log.info("[Editor] Activé — " .. #_placed .. " entités importées")
end

function Editor.disable()
    if not _enabled then return end
    _enabled  = false
    _selected = nil
    _placing  = false
    _panning  = false
    Game.setTimescale(1)
    Log.info("[Editor] Désactivé")
end

function Editor.toggle()
    if _enabled then Editor.disable() else Editor.enable() end
end

function Editor.isEnabled() return _enabled end

-- ── Handlers input ────────────────────────────────────────────────────────────

function Editor.onMouseDown(button, sx, sy)
    if not _enabled then return end

    if button == "right" then
        _panning = true
        local mp = Input.getMousePosition()
        _panStartMouse.x = mp.x
        _panStartMouse.y = mp.y
        local c = Camera.getPosition()
        _panStartCam.x = c.x
        _panStartCam.y = c.y
        return
    end

    if button ~= "left" then return end

    -- Clic dans la palette
    if sx <= PALETTE_W then
        local itemY = sy - 44
        if itemY >= 0 then
            local idx = _paletteScroll + math.floor(itemY / ITEM_H) + 1
            if idx >= 1 and idx <= #_spriteList then
                _paletteIdx = idx
                _placing    = true
                _selected   = nil
            end
        end
        return
    end

    -- Clic dans la zone monde
    local wx, wy = screenToWorld(sx, sy)
    wx, wy = snap(wx), snap(wy)

    if _placing then
        spawnSprite(_spriteList[_paletteIdx], wx, wy)
    else
        -- Sélectionne l'entité la plus proche du clic (dans un rayon de 40px écran)
        local best, bestD = nil, 40
        for _, entry in ipairs(_placed) do
            local t = entry.entity:getTransform()
            if t then
                local esx, esy = worldToScreen(t.position.x, t.position.y)
                local d = math.sqrt((esx - sx)^2 + (esy - sy)^2)
                if d < bestD then best, bestD = entry, d end
            end
        end
        _selected = best
    end
end

function Editor.onMouseUp(button)
    if not _enabled then return end
    if button == "right" then _panning = false end
end

function Editor.onKeyDown(key)
    if not _enabled then return end

    if key == "Escape" then
        if _placing    then _placing  = false
        elseif _selected then _selected = nil
        else Editor.disable() end
        return
    end

    if key == "Delete" and _selected then
        removeEntry(_selected)
        _selected = nil
        return
    end

    if key == "G" then
        _snapGrid = (_snapGrid > 0) and 0 or 32
        return
    end

    if key == "S" and Input.isKeyPressed("LControl") then
        saveScene()
        return
    end

    -- Navigation palette
    if key == "PageUp" or (key == "Up" and not _selected) then
        if _paletteIdx > 1 then
            _paletteIdx = _paletteIdx - 1
            _placing    = true
            if _paletteIdx < _paletteScroll + 1 then
                _paletteScroll = math.max(0, _paletteScroll - 1)
            end
        end
        return
    end
    if key == "PageDown" or (key == "Down" and not _selected) then
        if _paletteIdx < #_spriteList then
            _paletteIdx = _paletteIdx + 1
            _placing    = true
            if _paletteIdx > _paletteScroll + VISIBLE then
                _paletteScroll = _paletteScroll + 1
            end
        end
        return
    end

    -- Zoom
    if key == "Add" then
        Camera.setZoom(math.min(Camera.getZoom() * 1.25, 8.0))
        return
    end
    if key == "Subtract" then
        Camera.setZoom(math.max(Camera.getZoom() / 1.25, 0.1))
        return
    end

    -- Déplacement de l'entité sélectionnée
    if _selected then
        local step = math.max(_snapGrid, 1)
        local t = _selected.entity:getTransform()
        if not t then return end
        if     key == "A" or key == "Left"  then t.position.x = t.position.x - step
        elseif key == "D" or key == "Right" then t.position.x = t.position.x + step
        elseif key == "W" or key == "Up"    then t.position.y = t.position.y - step
        elseif key == "S" or key == "Down"  then t.position.y = t.position.y + step
        end
        _saved = false
    end
end

-- ── Mise à jour (pan caméra) ──────────────────────────────────────────────────

function Editor.update(dt)
    if not _enabled or not _panning then return end
    local mp = Input.getMousePosition()
    local z  = Camera.getZoom()
    Camera.setPosition(
        _panStartCam.x - (mp.x - _panStartMouse.x) / z,
        _panStartCam.y - (mp.y - _panStartMouse.y) / z
    )
end

-- ── Rendu ─────────────────────────────────────────────────────────────────────

local function drawGrid(ws)
    if _snapGrid <= 0 then return end
    local z        = Camera.getZoom()
    local c        = Camera.getPosition()
    local gridStep = _snapGrid * z
    if gridStep < 6 then return end

    local ox = (ws.x * 0.5 - c.x * z) % gridStep
    local oy = (ws.y * 0.5 - c.y * z) % gridStep

    local x = PALETTE_W + ox % gridStep
    while x < ws.x do
        Debug.fillRect(x, 0, 1, ws.y - 22, COL_GRID, 990)
        x = x + gridStep
    end
    local y = oy % gridStep
    while y < ws.y - 22 do
        Debug.fillRect(PALETTE_W, y, ws.x - PALETTE_W, 1, COL_GRID, 990)
        y = y + gridStep
    end
end

local function drawPalette(ws)
    Debug.fillRect(0, 0, PALETTE_W, ws.y, COL_BG, 1000)

    local title = _saved and "EDITEUR  [Ctrl+E]" or "EDITEUR  [Ctrl+E] *"
    Debug.label(8, 6,  title, COL_TEXT, 1002)
    local snapTxt = _snapGrid > 0 and ("Snap " .. _snapGrid .. "px") or "Snap OFF"
    Debug.label(8, 22, string.format("%s   Zoom %.1fx", snapTxt, Camera.getZoom()), COL_DIM, 1002)

    local y = 44
    for i = 1, VISIBLE do
        local idx = _paletteScroll + i
        if idx > #_spriteList then break end
        local id  = _spriteList[idx]
        local col = (idx == _paletteIdx and _placing) and COL_SEL_IT or COL_ITEM
        Debug.fillRect(4, y, PALETTE_W - 8, ITEM_H - 2, col, 1001)
        Debug.label(10, y + 6, id, COL_TEXT, 1002)
        y = y + ITEM_H
    end

    if #_spriteList > VISIBLE then
        Debug.label(8, ws.y - 38,
            string.format("↑↓  %d / %d", _paletteIdx, #_spriteList), COL_DIM, 1001)
    end
end

local function drawStatus(ws)
    local mode
    if _placing then
        mode = "POSE : " .. (_spriteList[_paletteIdx] or "?")
    elseif _selected then
        local t = _selected.entity:getTransform()
        local px = t and math.floor(t.position.x) or 0
        local py = t and math.floor(t.position.y) or 0
        mode = string.format("SÉLECT : %s  (%d, %d)  WASD=déplace  Del=suppr", _selected.spriteID, px, py)
    else
        local mp = Input.getMousePosition()
        local wx, wy = screenToWorld(mp.x, mp.y)
        mode = string.format("Monde (%.0f, %.0f)  | clic palette=poser  |  clic entité=sélect", snap(wx), snap(wy))
    end

    local right = string.format("%d entités   Ctrl+S sauvegarder   G grille   +/- zoom   Echap quitter", #_placed)
    Debug.fillRect(PALETTE_W, ws.y - 22, ws.x - PALETTE_W, 22, COL_STATUS, 1000)
    Debug.label(PALETTE_W + 6, ws.y - 16, mode, COL_TEXT, 1001)
    local rw = #right * 7
    Debug.label(ws.x - rw - 6, ws.y - 16, right, COL_DIM, 1001)
end

local function drawSelectionHighlight()
    if not _selected then return end
    local t = _selected.entity:getTransform()
    local s = _selected.entity:getSprite()
    if not t or not s then return end
    local z  = Camera.getZoom()
    local hw = s.size.x * 0.5 * z
    local hh = s.size.y * 0.5 * z
    local sx, sy = worldToScreen(t.position.x, t.position.y)
    local th = 2
    -- Cadre jaune autour du sprite
    Debug.fillRect(sx - hw - th, sy - hh - th, hw * 2 + th * 2, th, COL_HILIGHT, 1001)
    Debug.fillRect(sx - hw - th, sy + hh,       hw * 2 + th * 2, th, COL_HILIGHT, 1001)
    Debug.fillRect(sx - hw - th, sy - hh - th, th, hh * 2 + th * 2, COL_HILIGHT, 1001)
    Debug.fillRect(sx + hw,       sy - hh - th, th, hh * 2 + th * 2, COL_HILIGHT, 1001)
end

local function drawGhostSprite(ws)
    if not _placing then return end
    local mp = Input.getMousePosition()
    if mp.x <= PALETTE_W then return end
    local wx, wy = screenToWorld(mp.x, mp.y)
    wx, wy = snap(wx), snap(wy)
    local sx, sy = worldToScreen(wx, wy)

    local def = _spriteDefs[_spriteList[_paletteIdx]]
    local z   = Camera.getZoom()
    local hw  = ((def and def.width  * (def.scale or 1)) or 64) * 0.5 * z
    local hh  = ((def and def.height * (def.scale or 1)) or 64) * 0.5 * z

    Debug.fillRect(sx - hw, sy - hh, hw * 2, hh * 2, COL_GHOST,  1001)
    Debug.fillRect(sx - hw, sy - hh, hw * 2, 1,       COL_GHOSTB, 1002)
    Debug.fillRect(sx - hw, sy + hh, hw * 2, 1,       COL_GHOSTB, 1002)
    Debug.fillRect(sx - hw, sy - hh, 1, hh * 2,       COL_GHOSTB, 1002)
    Debug.fillRect(sx + hw, sy - hh, 1, hh * 2,       COL_GHOSTB, 1002)
    -- Croix centrale
    Debug.fillRect(sx - 6, sy, 12, 1, COL_GHOSTB, 1002)
    Debug.fillRect(sx, sy - 6, 1, 12, COL_GHOSTB, 1002)
end

function Editor.draw()
    if not _enabled then return end
    local ws = Viewport.getWindowSize()
    drawGrid(ws)
    drawSelectionHighlight()
    drawGhostSprite(ws)
    drawPalette(ws)
    drawStatus(ws)
end

-- ── Auto-branchement EventBus ─────────────────────────────────────────────────
-- Enregistre les handlers une seule fois au chargement du module.

EventBus.on("mouse_down", function(data)
    Editor.onMouseDown(data.button, data.x, data.y)
end)

EventBus.on("mouse_up", function(data)
    Editor.onMouseUp(data.button)
end)

EventBus.on("key_down_E", function()
    if Input.isKeyPressed("LControl") or Input.isKeyPressed("RControl") then
        Editor.toggle()
    end
end)

-- key_down générique → délègue à onKeyDown
EventBus.on("key_down", function(data)
    if _enabled then Editor.onKeyDown(data.key) end
end)

return Editor
