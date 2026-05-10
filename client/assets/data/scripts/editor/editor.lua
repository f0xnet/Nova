-- editor/editor.lua
-- Éditeur de niveau in-game — Ctrl+E pour activer/désactiver.
--
-- Contrôles globaux :
--   Ctrl+E              — toggle éditeur
--   Ctrl+S              — sauvegarder la scène courante
--   Echap               — désélectionner / fermer l'éditeur
--   G                   — bascule grille (32 px)
--   + / -               — zoom caméra
--   Clic droit + drag   — pan caméra
--
-- Palette / placement :
--   Clic gauche palette — choisit le sprite à poser
--   Clic gauche monde   — pose le sprite (mode pose) ou sélectionne entité
--   ↑ ↓                 — naviguer dans la palette
--
-- Sélection :
--   WASD / Flèches      — déplace l'entité (pas = grille ou 1 px)
--   Suppr               — supprime l'entité sélectionnée
--   [ ] / ( )           — décrémente / incrémente le layer (zOrder)
--   0–9                 — fixe le layer directement
--
-- Note rendu : toutes les primitives passent par Debug.* (font 28 px).

local Editor = {}

-- ═════════════════════════════════════════════════════════════════════════════
-- État
-- ═════════════════════════════════════════════════════════════════════════════

local _enabled       = false
local _spriteDefs    = {}
local _spriteList    = {}
local _paletteIdx    = 1
local _paletteScroll = 0
local _placing       = false
local _placed        = {}
local _selected      = nil
local _snapGrid      = 32
local _layer         = 3
local _scenePath     = nil
local _sceneName     = "Untitled"
local _saved         = true

local _panning       = false
local _panStartMouse = { x = 0, y = 0 }
local _panStartCam   = { x = 0, y = 0 }

-- ═════════════════════════════════════════════════════════════════════════════
-- Thème (inspiré VS Code / Tiled / LDtk)
-- ═════════════════════════════════════════════════════════════════════════════

local C_BG          = Color.new( 24,  24,  32, 240)  -- fond panneaux
local C_BG_DEEPER   = Color.new( 18,  18,  24, 250)  -- fond toolbar / status
local C_BG_ITEM     = Color.new( 36,  38,  50, 255)  -- ligne palette par défaut
local C_BG_HOVER    = Color.new( 50,  54,  72, 255)  -- ligne survolée
local C_BG_ACTIVE   = Color.new( 56,  96, 168, 255)  -- ligne active (placement)
local C_BG_SELECT   = Color.new( 80,  60, 140, 255)  -- entité sélectionnée
local C_BORDER      = Color.new( 60,  62,  84, 255)
local C_BORDER_LITE = Color.new( 90,  92, 120, 255)
local C_ACCENT      = Color.new(108, 168, 255, 255)
local C_TEXT        = Color.new(232, 232, 240, 255)
local C_TEXT_DIM    = Color.new(168, 168, 188, 255)
local C_TEXT_MUTE   = Color.new(112, 112, 138, 255)
local C_WARN        = Color.new(255, 182, 107, 255)
local C_OK          = Color.new(143, 227, 136, 255)
local C_GHOST       = Color.new(108, 168, 255,  60)
local C_GHOST_B     = Color.new(108, 168, 255, 220)
local C_HIGHLIGHT   = Color.new(255, 200,  60, 230)
local C_GRID        = Color.new( 80,  85, 110,  55)

-- ═════════════════════════════════════════════════════════════════════════════
-- Constantes layout (police 28 px, donc lignes ≥ 36 px)
-- ═════════════════════════════════════════════════════════════════════════════

local FONT_H        = 28
local TOOLBAR_H     = 48
local STATUS_H      = 44
local SECTION_H     = 36              -- en-tête "SPRITES"
local ITEM_H        = 40              -- ligne palette
local PALETTE_W     = 280
local PROP_W        = 320
local PROP_H        = 220
local PAD           = 10
local TXT_PAD_Y     = math.floor((ITEM_H - FONT_H) / 2)  -- centrage vertical

-- ═════════════════════════════════════════════════════════════════════════════
-- Coordonnées
-- ═════════════════════════════════════════════════════════════════════════════

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

local function pointInRect(px, py, rx, ry, rw, rh)
    return px >= rx and px < rx + rw and py >= ry and py < ry + rh
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Définitions et entités
-- ═════════════════════════════════════════════════════════════════════════════

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

    -- ⚠ Le moteur attend size = pixels texture (non-scaled),
    --   origin = (width/2, height/2), transform.scale = facteur.
    local t = e:getTransform()
    t.position.x = wx
    t.position.y = wy
    t.origin.x   = def.width  * 0.5
    t.origin.y   = def.height * 0.5
    local sc     = def.scale or 1.0
    t.scale.x, t.scale.y = sc, sc

    local s = e:getSprite()
    s.textureID     = spriteID
    s.textureHandle = Resources.loadTexture(def.texture)
    s.size.x        = def.width
    s.size.y        = def.height
    s.zOrder        = _layer

    local entry = { entity = e, spriteID = spriteID, zOrder = _layer }
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

local function setEntryLayer(entry, n)
    n = math.max(0, math.min(10, n))
    entry.zOrder = n
    local s = entry.entity:getSprite()
    if s then s.zOrder = n end
    _saved = false
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Sauvegarde JSON
-- ═════════════════════════════════════════════════════════════════════════════

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
    end
    Log.error("[Editor] Sauvegarde échouée : " .. tostring(err))
    return false
end

-- ═════════════════════════════════════════════════════════════════════════════
-- API publique
-- ═════════════════════════════════════════════════════════════════════════════

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

function Editor.toggle() if _enabled then Editor.disable() else Editor.enable() end end
function Editor.isEnabled() return _enabled end

-- ═════════════════════════════════════════════════════════════════════════════
-- Helpers UI
-- ═════════════════════════════════════════════════════════════════════════════

-- Largeur écran d'un texte en pixels (police monospace ≈ 16 px par caractère).
local function textW(s) return #s * 16 end

local function truncate(s, maxPx)
    local maxC = math.max(1, math.floor(maxPx / 16))
    if #s <= maxC then return s end
    return s:sub(1, maxC - 1) .. "…"
end

-- Rect avec bordure 1 px supérieure pour un effet "panneau".
local function panel(x, y, w, h, bg)
    Debug.fillRect(x, y, w, h, bg)
    Debug.fillRect(x, y, w, 1, C_BORDER)
end

-- Bandeau accent vertical (4 px) à gauche pour signaler une ligne active.
local function accentBar(x, y, h)
    Debug.fillRect(x, y, 4, h, C_ACCENT)
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Géométrie des zones
-- ═════════════════════════════════════════════════════════════════════════════

local function paletteListRect(ws)
    local x = 0
    local y = TOOLBAR_H + SECTION_H
    local w = PALETTE_W
    local h = ws.y - STATUS_H - y
    return x, y, w, h
end

local function visibleItemCount(ws)
    local _, _, _, h = paletteListRect(ws)
    return math.max(1, math.floor(h / ITEM_H))
end

local function propertiesRect(ws)
    -- Flottant en haut-droit, sous le toolbar
    return ws.x - PROP_W - PAD, TOOLBAR_H + PAD, PROP_W, PROP_H
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Input
-- ═════════════════════════════════════════════════════════════════════════════

function Editor.onMouseDown(button, sx, sy)
    if not _enabled then return end

    if button == "right" then
        _panning = true
        local mp = Input.getMousePosition()
        _panStartMouse.x, _panStartMouse.y = mp.x, mp.y
        local c = Camera.getPosition()
        _panStartCam.x, _panStartCam.y = c.x, c.y
        return
    end
    if button ~= "left" then return end

    local ws = Viewport.getWindowSize()

    -- Toolbar : non-cliquable pour l'instant
    if sy < TOOLBAR_H then return end

    -- Palette
    if sx < PALETTE_W then
        local lx, ly, _, lh = paletteListRect(ws)
        if pointInRect(sx, sy, lx, ly, PALETTE_W, lh) then
            local idx = _paletteScroll + math.floor((sy - ly) / ITEM_H) + 1
            if idx >= 1 and idx <= #_spriteList then
                _paletteIdx = idx
                _placing    = true
                _selected   = nil
            end
        end
        return
    end

    -- Properties (clics ignorés — purement informatif)
    if _selected then
        local px, py, pw, ph = propertiesRect(ws)
        if pointInRect(sx, sy, px, py, pw, ph) then return end
    end

    -- Status bar : pas de clic
    if sy >= ws.y - STATUS_H then return end

    -- Zone monde
    local wx, wy = screenToWorld(sx, sy)
    wx, wy = snap(wx), snap(wy)

    if _placing then
        spawnSprite(_spriteList[_paletteIdx], wx, wy)
    else
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
        if _placing       then _placing  = false
        elseif _selected  then _selected = nil
        else                   Editor.disable() end
        return
    end

    if key == "Delete" and _selected then
        removeEntry(_selected); _selected = nil; return
    end

    if key == "G" then
        _snapGrid = (_snapGrid > 0) and 0 or 32; return
    end

    if key == "S" and (Input.isKeyPressed("LControl") or Input.isKeyPressed("RControl")) then
        saveScene(); return
    end

    -- Navigation palette (sans entité sélectionnée)
    if key == "PageUp" or (key == "Up" and not _selected) then
        if _paletteIdx > 1 then
            _paletteIdx = _paletteIdx - 1
            _placing    = true
            if _paletteIdx <= _paletteScroll then
                _paletteScroll = math.max(0, _paletteScroll - 1)
            end
        end
        return
    end
    if key == "PageDown" or (key == "Down" and not _selected) then
        if _paletteIdx < #_spriteList then
            _paletteIdx = _paletteIdx + 1
            _placing    = true
            local vis   = visibleItemCount(Viewport.getWindowSize())
            if _paletteIdx > _paletteScroll + vis then
                _paletteScroll = _paletteScroll + 1
            end
        end
        return
    end

    -- Layer (zOrder)
    if key == "LBracket" or key == "LParen" then
        if _selected then setEntryLayer(_selected, _selected.zOrder - 1)
        else _layer = math.max(0, _layer - 1) end
        return
    end
    if key == "RBracket" or key == "RParen" then
        if _selected then setEntryLayer(_selected, _selected.zOrder + 1)
        else _layer = math.min(10, _layer + 1) end
        return
    end
    local num = key:match("^Num(%d)$") or key:match("^Numpad(%d)$")
    if num then
        local n = tonumber(num)
        if _selected then setEntryLayer(_selected, n) else _layer = n end
        return
    end

    -- Zoom
    if key == "Add"      then Camera.setZoom(math.min(Camera.getZoom() * 1.25, 8.0)); return end
    if key == "Subtract" then Camera.setZoom(math.max(Camera.getZoom() / 1.25, 0.1)); return end

    -- Déplacement entité sélectionnée
    if _selected then
        local step = math.max(_snapGrid, 1)
        local t    = _selected.entity:getTransform()
        if not t then return end
        if     key == "A" or key == "Left"  then t.position.x = t.position.x - step
        elseif key == "D" or key == "Right" then t.position.x = t.position.x + step
        elseif key == "W" or key == "Up"    then t.position.y = t.position.y - step
        elseif key == "S" or key == "Down"  then t.position.y = t.position.y + step
        else return end
        _saved = false
    end
end

function Editor.update(dt)
    if not _enabled or not _panning then return end
    local mp = Input.getMousePosition()
    local z  = Camera.getZoom()
    Camera.setPosition(
        _panStartCam.x - (mp.x - _panStartMouse.x) / z,
        _panStartCam.y - (mp.y - _panStartMouse.y) / z
    )
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Rendu
-- ═════════════════════════════════════════════════════════════════════════════

local function drawGrid(ws)
    if _snapGrid <= 0 then return end
    local z  = Camera.getZoom()
    local c  = Camera.getPosition()
    local gs = _snapGrid * z
    if gs < 6 then return end

    local ox = (ws.x * 0.5 - c.x * z) % gs
    local oy = (ws.y * 0.5 - c.y * z) % gs

    local worldX1, worldY1 = PALETTE_W, TOOLBAR_H
    local worldX2          = ws.x
    local worldY2          = ws.y - STATUS_H

    local x = worldX1 + ox
    while x < worldX2 do
        Debug.fillRect(x, worldY1, 1, worldY2 - worldY1, C_GRID)
        x = x + gs
    end
    local y = worldY1 + oy
    while y < worldY2 do
        Debug.fillRect(worldX1, y, worldX2 - worldX1, 1, C_GRID)
        y = y + gs
    end
end

local function drawToolbar(ws)
    -- Fond + bordure basse
    Debug.fillRect(0, 0, ws.x, TOOLBAR_H, C_BG_DEEPER)
    Debug.fillRect(0, TOOLBAR_H - 1, ws.x, 1, C_BORDER)

    local y = math.floor((TOOLBAR_H - FONT_H) / 2)

    -- Titre + état "modifié"
    local title = "EDITEUR"
    Debug.label(PAD, y, title, C_TEXT)

    -- Indicateur "*" (modifications non sauvées)
    if not _saved then
        Debug.label(PAD + textW(title) + 8, y, "●", C_WARN)
    end

    -- Champ Layer (au centre-gauche, mis en valeur)
    local sectX = PAD + 200
    Debug.label(sectX, y, "Layer", C_TEXT_DIM)
    local layerStr = string.format(" %d ", _layer)
    local boxX = sectX + textW("Layer") + 6
    local boxW = textW(layerStr) + 8
    Debug.fillRect(boxX, y - 4, boxW, FONT_H + 8, C_BG_ITEM)
    Debug.fillRect(boxX, y - 4, 4, FONT_H + 8, C_ACCENT)
    Debug.label(boxX + 4, y, layerStr, C_TEXT)
    Debug.label(boxX + boxW + 8, y, "[ ]", C_TEXT_MUTE)

    -- Snap + Zoom à droite
    local snapTxt = (_snapGrid > 0) and ("Snap " .. _snapGrid) or "Snap OFF"
    local zoomTxt = string.format("Zoom %.1fx", Camera.getZoom())
    local right   = snapTxt .. "    " .. zoomTxt
    Debug.label(ws.x - textW(right) - PAD, y, right, C_TEXT_DIM)
end

local function drawPalette(ws)
    local lx, ly, lw, lh = paletteListRect(ws)
    local paletteFullH   = ws.y - STATUS_H

    -- Fond palette + bordure droite
    Debug.fillRect(0, TOOLBAR_H, PALETTE_W, paletteFullH - TOOLBAR_H, C_BG)
    Debug.fillRect(PALETTE_W - 1, TOOLBAR_H, 1, paletteFullH - TOOLBAR_H, C_BORDER)

    -- En-tête de section "SPRITES"
    local sectY = TOOLBAR_H
    Debug.fillRect(0, sectY, PALETTE_W, SECTION_H, C_BG_DEEPER)
    Debug.fillRect(0, sectY + SECTION_H - 1, PALETTE_W, 1, C_BORDER)
    local stY = sectY + math.floor((SECTION_H - FONT_H) / 2)
    Debug.label(PAD, stY, "SPRITES", C_TEXT_DIM)
    local count = string.format("%d", #_spriteList)
    Debug.label(PALETTE_W - PAD - textW(count), stY, count, C_TEXT_MUTE)

    -- Bornage du scroll (en cas de redimensionnement)
    local vis = visibleItemCount(ws)
    if _paletteScroll + vis > #_spriteList then
        _paletteScroll = math.max(0, #_spriteList - vis)
    end

    -- Items
    local mp = Input.getMousePosition()
    for i = 1, vis do
        local idx = _paletteScroll + i
        if idx > #_spriteList then break end
        local id = _spriteList[idx]
        local y  = ly + (i - 1) * ITEM_H

        local isActive = (idx == _paletteIdx and _placing)
        local isHover  = pointInRect(mp.x, mp.y, lx, y, lw, ITEM_H) and not isActive
        local bg       = isActive and C_BG_ACTIVE or (isHover and C_BG_HOVER or C_BG_ITEM)

        -- Ligne : fond plein puis 1 px de séparation au bas
        Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
        if isActive then accentBar(lx + 4, y + 2, ITEM_H - 4) end

        local labelCol = isActive and C_TEXT or (isHover and C_TEXT or C_TEXT_DIM)
        Debug.label(lx + PAD + (isActive and 6 or 0), y + TXT_PAD_Y, id, labelCol)

        -- Indicateur layer du sprite (à droite)
        local def = _spriteDefs[id]
        if def and def.zOrder then
            local zs = "z" .. def.zOrder
            Debug.label(lx + lw - textW(zs) - PAD, y + TXT_PAD_Y, zs, C_TEXT_MUTE)
        end
    end

    -- Compteur scroll si liste plus longue que visible
    if #_spriteList > vis then
        local nav = string.format("%d–%d / %d", _paletteScroll + 1,
                                  math.min(_paletteScroll + vis, #_spriteList),
                                  #_spriteList)
        Debug.label(PAD, ly + lh - FONT_H - 2, nav, C_TEXT_MUTE)
    end
end

local function drawProperties(ws)
    if not _selected then return end
    local px, py, pw, ph = propertiesRect(ws)
    local t = _selected.entity:getTransform()
    if not t then return end

    -- Ombre + panneau
    Debug.fillRect(px + 4, py + 4, pw, ph, Color.new(0, 0, 0, 80))
    Debug.fillRect(px,     py,     pw, ph, C_BG)
    Debug.fillRect(px,     py,     pw, 1,  C_BORDER_LITE)
    Debug.fillRect(px,     py + ph - 1, pw, 1, C_BORDER_LITE)
    Debug.fillRect(px,     py,     1,  ph, C_BORDER_LITE)
    Debug.fillRect(px + pw - 1, py, 1, ph, C_BORDER_LITE)

    -- En-tête
    Debug.fillRect(px, py, pw, SECTION_H, C_BG_DEEPER)
    Debug.fillRect(px, py + SECTION_H - 1, pw, 1, C_BORDER)
    Debug.label(px + PAD, py + math.floor((SECTION_H - FONT_H) / 2),
                "PROPERTIES", C_TEXT_DIM)

    -- Lignes (40 px chacune)
    local row = py + SECTION_H + 6
    local function line(label, value, valueCol)
        Debug.label(px + PAD,       row, label, C_TEXT_MUTE)
        Debug.label(px + 110,       row, value, valueCol or C_TEXT)
        row = row + ITEM_H
    end

    line("Sprite", _selected.spriteID, C_ACCENT)
    line("Position", string.format("%d, %d", math.floor(t.position.x), math.floor(t.position.y)))
    line("Layer", string.format("%d  [/]", _selected.zOrder))

    -- Indication de raccourcis en bas
    local hint = "WASD déplace · Suppr efface · Esc désélec."
    Debug.label(px + PAD, py + ph - FONT_H - 6, truncate(hint, pw - 2 * PAD), C_TEXT_MUTE)
end

local function drawStatus(ws)
    local barY = ws.y - STATUS_H
    Debug.fillRect(0, barY, ws.x, STATUS_H, C_BG_DEEPER)
    Debug.fillRect(0, barY, ws.x, 1, C_BORDER)

    local y = barY + math.floor((STATUS_H - FONT_H) / 2)

    -- Texte gauche : mode courant
    local mode, modeCol
    if _placing then
        mode    = "PLACE  " .. (_spriteList[_paletteIdx] or "?")
        modeCol = C_ACCENT
    elseif _selected then
        mode    = "SELECT  " .. _selected.spriteID
        modeCol = C_HIGHLIGHT
    else
        mode    = "IDLE"
        modeCol = C_TEXT_DIM
    end
    Debug.label(PAD, y, mode, modeCol)

    -- Coords souris monde
    local mp = Input.getMousePosition()
    local wx, wy = screenToWorld(mp.x, mp.y)
    local coords = string.format("(%d, %d)", math.floor(snap(wx)), math.floor(snap(wy)))
    Debug.label(PAD + 280, y, coords, C_TEXT_DIM)

    -- Texte droit : compte + raccourcis
    local right = string.format("%d entités  •  Ctrl+S enregistrer  •  G grille  •  Esc",
                                #_placed)
    Debug.label(ws.x - textW(right) - PAD, y, right, C_TEXT_MUTE)
end

local function drawSelectionHighlight()
    if not _selected then return end
    local t = _selected.entity:getTransform()
    local s = _selected.entity:getSprite()
    if not t or not s then return end
    local z  = Camera.getZoom()
    local hw = s.size.x * (t.scale.x or 1) * 0.5 * z
    local hh = s.size.y * (t.scale.y or 1) * 0.5 * z
    local sx, sy = worldToScreen(t.position.x, t.position.y)
    local th = 2

    Debug.fillRect(sx - hw - th, sy - hh - th, hw * 2 + th * 2, th, C_HIGHLIGHT)
    Debug.fillRect(sx - hw - th, sy + hh,       hw * 2 + th * 2, th, C_HIGHLIGHT)
    Debug.fillRect(sx - hw - th, sy - hh - th, th, hh * 2 + th * 2, C_HIGHLIGHT)
    Debug.fillRect(sx + hw,      sy - hh - th, th, hh * 2 + th * 2, C_HIGHLIGHT)
end

local function drawGhostSprite()
    if not _placing then return end
    local mp = Input.getMousePosition()
    if mp.x < PALETTE_W or mp.y < TOOLBAR_H then return end

    local wx, wy = screenToWorld(mp.x, mp.y)
    wx, wy = snap(wx), snap(wy)
    local sx, sy = worldToScreen(wx, wy)

    local def = _spriteDefs[_spriteList[_paletteIdx]]
    local z   = Camera.getZoom()
    local sc  = (def and def.scale) or 1.0
    local hw  = ((def and def.width)  or 64) * sc * 0.5 * z
    local hh  = ((def and def.height) or 64) * sc * 0.5 * z

    Debug.fillRect(sx - hw, sy - hh, hw * 2, hh * 2, C_GHOST)
    Debug.fillRect(sx - hw, sy - hh, hw * 2, 1,      C_GHOST_B)
    Debug.fillRect(sx - hw, sy + hh, hw * 2, 1,      C_GHOST_B)
    Debug.fillRect(sx - hw, sy - hh, 1, hh * 2,      C_GHOST_B)
    Debug.fillRect(sx + hw, sy - hh, 1, hh * 2,      C_GHOST_B)
    Debug.fillRect(sx - 8, sy, 16, 1, C_GHOST_B)
    Debug.fillRect(sx, sy - 8, 1, 16, C_GHOST_B)
end

function Editor.draw()
    if not _enabled then return end
    local ws = Viewport.getWindowSize()
    drawGrid(ws)
    drawSelectionHighlight()
    drawGhostSprite()
    drawToolbar(ws)
    drawPalette(ws)
    drawProperties(ws)
    drawStatus(ws)
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Branchement EventBus
-- ═════════════════════════════════════════════════════════════════════════════

EventBus.on("mouse_down", function(data) Editor.onMouseDown(data.button, data.x, data.y) end)
EventBus.on("mouse_up",   function(data) Editor.onMouseUp(data.button) end)

EventBus.on("key_down_E", function()
    if Input.isKeyPressed("LControl") or Input.isKeyPressed("RControl") then
        Editor.toggle()
    end
end)

EventBus.on("key_down", function(data)
    if _enabled then Editor.onKeyDown(data.key) end
end)

return Editor
