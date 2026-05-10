-- editor/editor.lua
-- Éditeur de niveau in-game — Ctrl+E pour activer/désactiver.
--
-- Modes d'outils (toolbar) :
--   SELECT              — clic monde sélectionne une entité
--   PLACE               — clic monde dépose le sprite courant (palette)
--   Choisir un sprite dans la palette bascule automatiquement en PLACE.
--
-- Contrôles globaux :
--   Ctrl+E              — toggle éditeur
--   Ctrl+S              — sauvegarder la scène courante
--   Ctrl                — sortir du mode PLACE (retour SELECT)
--   Ctrl+D              — désélectionner / sortir PLACE
--   Ctrl+G              — bascule grille
--   Ctrl+1 / Ctrl+2     — bascule onglet SPRITES / LAYERS
--   Echap               — désélec. / fermer / annuler une édition
--   G                   — bascule grille (alias)
--   + / -               — zoom caméra
--   Clic droit + drag   — pan caméra
--
-- Panneau gauche (tabs) :
--   SPRITES             — palette de placement
--   LAYERS              — inspector des entités groupées par zOrder
--                         Clic en-tête      → bascule layer actif + plie/déplie
--                         Clic entité       → sélectionne + recentre la caméra
--
-- Panneau droit (Properties) :
--   Clic ligne          — édite la valeur (Entrée valide, Echap annule)
--   ↑/↓ ou [/]          — cycle dans la liste sprite (édition spriteID)
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
local _spriteDefs    = {}          -- id  → def
local _spriteDefsByPath = {}       -- texture path → def (pour résoudre les entités scène)
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

local _leftTab       = "sprites"   -- "sprites" | "layers"
local _layerExpanded = {}          -- [n] = bool
local _editing       = nil         -- { field = "x"|"y"|"zOrder"|"spriteID", buffer = "" }

-- ═════════════════════════════════════════════════════════════════════════════
-- Thème (inspiré VS Code / Tiled / LDtk)
-- ═════════════════════════════════════════════════════════════════════════════

local C_BG          = Color.new( 24,  24,  32, 240)
local C_BG_DEEPER   = Color.new( 18,  18,  24, 250)
local C_BG_ITEM     = Color.new( 36,  38,  50, 255)
local C_BG_HOVER    = Color.new( 50,  54,  72, 255)
local C_BG_ACTIVE   = Color.new( 56,  96, 168, 255)
local C_BG_SELECT   = Color.new( 80,  60, 140, 255)
local C_BG_SUB      = Color.new( 28,  30,  40, 255)
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
-- Constantes layout (police 28 px)
-- ═════════════════════════════════════════════════════════════════════════════

local FONT_H        = 28
local TOOLBAR_H     = 48
local STATUS_H      = 44
local SECTION_H     = 36
local ITEM_H        = 40
local PALETTE_W     = 280
local PROP_W        = 320
local PROP_H        = 320
local PAD           = 10
local TXT_PAD_Y     = math.floor((ITEM_H - FONT_H) / 2)

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
    _spriteDefs        = {}
    _spriteDefsByPath  = {}
    _spriteList        = {}
    for _, s in ipairs(data.sprites) do
        _spriteDefs[s.id] = s
        -- Plusieurs sprites peuvent partager le même chemin texture (ex. wall + wall_brick) :
        -- on stocke une liste, désambiguïsée par width/height à la résolution.
        local list = _spriteDefsByPath[s.texture]
        if not list then list = {}; _spriteDefsByPath[s.texture] = list end
        table.insert(list, s)
        table.insert(_spriteList, s.id)
    end
    Log.info("[Editor] " .. #_spriteList .. " sprites chargés")
end

local function importSceneEntities()
    _placed = {}
    local total, accepted = 0, 0
    local rNoSprite, rEmptyTex, rScript, rNoDef = 0, 0, 0, 0
    for _, e in ipairs(Registry:getAllEntities()) do
        total = total + 1
        local s = e:getSprite()
        if not s then
            rNoSprite = rNoSprite + 1
        elseif s.textureID == "" then
            rEmptyTex = rEmptyTex + 1
        elseif e:hasComponent("ScriptComponent") then
            rScript = rScript + 1
        else
            -- Le moteur stocke le chemin texture dans textureID pour les entités scène ;
            -- l'éditeur y stocke directement le spriteID. Résoudre les deux cas.
            local def = _spriteDefs[s.textureID]
            if not def then
                local candidates = _spriteDefsByPath[s.textureID]
                if candidates then
                    if #candidates == 1 then
                        def = candidates[1]
                    else
                        for _, c in ipairs(candidates) do
                            if c.width == s.size.x and c.height == s.size.y then
                                def = c; break
                            end
                        end
                        def = def or candidates[1]
                    end
                end
            end
            if def then
                table.insert(_placed, { entity = e, spriteID = def.id, zOrder = s.zOrder })
                accepted = accepted + 1
            else
                rNoDef = rNoDef + 1
                Log.info("[Editor] textureID inconnu : '" .. tostring(s.textureID) .. "'")
            end
        end
    end
    Log.info(string.format(
        "[Editor] Import : total=%d accepted=%d (rejets : no_sprite=%d empty_tex=%d script=%d no_def=%d)",
        total, accepted, rNoSprite, rEmptyTex, rScript, rNoDef))
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

-- Retourne vrai si le point écran (sx,sy) est dans la AABB de l'entité.
local function hitTest(entry, sx, sy)
    local t = entry.entity:getTransform()
    local s = entry.entity:getSprite()
    if not (t and s) then return false end
    local z   = Camera.getZoom()
    local scX = t.scale.x or 1.0
    local scY = t.scale.y or 1.0
    local hw  = s.size.x * scX * 0.5 * z
    local hh  = s.size.y * scY * 0.5 * z
    local cx, cy = worldToScreen(t.position.x, t.position.y)
    return sx >= cx - hw and sx <= cx + hw
       and sy >= cy - hh and sy <= cy + hh
end

local function setEntryLayer(entry, n)
    n = math.max(0, math.min(10, n))
    entry.zOrder = n
    local s = entry.entity:getSprite()
    if s then s.zOrder = n end
    _saved = false
end

local function changeEntrySprite(entry, newID)
    local def = _spriteDefs[newID]
    if not def then return false end
    local t = entry.entity:getTransform()
    local s = entry.entity:getSprite()
    if not (t and s) then return false end

    entry.spriteID  = newID
    s.textureID     = newID
    s.textureHandle = Resources.loadTexture(def.texture)
    s.size.x        = def.width
    s.size.y        = def.height
    t.origin.x      = def.width  * 0.5
    t.origin.y      = def.height * 0.5
    local sc        = def.scale or 1.0
    t.scale.x, t.scale.y = sc, sc
    _saved = false
    return true
end

-- Regroupe les entités par zOrder (clamp 0..10).
local function entitiesByLayer()
    local groups = {}
    for n = 0, 10 do groups[n] = {} end
    for _, entry in ipairs(_placed) do
        local n = math.max(0, math.min(10, entry.zOrder or 0))
        table.insert(groups[n], entry)
    end
    return groups
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Édition de propriétés
-- ═════════════════════════════════════════════════════════════════════════════

local function startEdit(field, currentValue)
    _editing = { field = field, buffer = tostring(currentValue) }
end

local function applyEdit()
    if not _editing then return end
    if not _selected then _editing = nil; return end
    local entry = _selected
    local t     = entry.entity:getTransform()
    local field = _editing.field
    local buf   = _editing.buffer

    if t then
        if field == "x" then
            local n = tonumber(buf); if n then t.position.x = n; _saved = false end
        elseif field == "y" then
            local n = tonumber(buf); if n then t.position.y = n; _saved = false end
        elseif field == "zOrder" then
            local n = tonumber(buf); if n then setEntryLayer(entry, math.floor(n)) end
        elseif field == "scale" then
            local n = tonumber(buf)
            if n and n > 0 then t.scale.x, t.scale.y = n, n; _saved = false end
        elseif field == "spriteID" then
            changeEntrySprite(entry, buf)
        end
    end
    _editing = nil
end

local function cancelEdit() _editing = nil end

local function cycleSprite(dir)
    if not _editing then return end
    local cur = _editing.buffer
    local idx = 1
    for i, id in ipairs(_spriteList) do
        if id == cur then idx = i; break end
    end
    idx = ((idx - 1 + dir) % #_spriteList) + 1
    _editing.buffer = _spriteList[idx]
end

local function handleEditKey(key)
    if key == "Return" or key == "Enter" or key == "KP_Enter" or key == "NumpadEnter" then
        applyEdit(); return
    end
    if key == "Escape"   then cancelEdit(); return end
    if key == "Backspace" then
        _editing.buffer = _editing.buffer:sub(1, -2); return
    end

    -- spriteID : cycle dans la liste, pas de saisie libre
    if _editing.field == "spriteID" then
        if key == "Up"   or key == "LBracket" or key == "LParen" then cycleSprite(-1); return end
        if key == "Down" or key == "RBracket" or key == "RParen" then cycleSprite( 1); return end
        return
    end

    -- Champs numériques : digits / signe / virgule
    local n = key:match("^Num(%d)$") or key:match("^Numpad(%d)$")
    if n then _editing.buffer = _editing.buffer .. n; return end
    if key == "Hyphen" or key == "Subtract" then
        if _editing.buffer == "" then _editing.buffer = "-" end
        return
    end
    if key == "Period" or key == "Decimal" then
        if not _editing.buffer:find(".", 1, true) then
            _editing.buffer = _editing.buffer .. "."
        end
        return
    end
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
    _editing  = nil
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
    _editing  = nil
    Game.setTimescale(1)
    Log.info("[Editor] Désactivé")
end

function Editor.toggle() if _enabled then Editor.disable() else Editor.enable() end end
function Editor.isEnabled() return _enabled end

-- ═════════════════════════════════════════════════════════════════════════════
-- Helpers UI
-- ═════════════════════════════════════════════════════════════════════════════

local function textW(s) return #s * 16 end

local function truncate(s, maxPx)
    local maxC = math.max(1, math.floor(maxPx / 16))
    if #s <= maxC then return s end
    return s:sub(1, maxC - 1) .. "…"
end

local function panel(x, y, w, h, bg)
    Debug.fillRect(x, y, w, h, bg)
    Debug.fillRect(x, y, w, 1, C_BORDER)
end

local function accentBar(x, y, h)
    Debug.fillRect(x, y, 4, h, C_ACCENT)
end

local function isCtrl()
    return Input.isKeyPressed("LControl") or Input.isKeyPressed("RControl")
end

-- Calcule la disposition du toolbar (titre, boutons SELECT/PLACE, suite).
-- Permet à drawToolbar et onMouseDown de partager les mêmes rectangles.
local function toolbarLayout()
    local textY  = math.floor((TOOLBAR_H - FONT_H) / 2)
    local boxH   = FONT_H + 8
    local boxY   = textY - 4

    local x = PAD + textW("EDITEUR") + 8
    if not _saved then x = x + 18 end
    x = x + 16

    local selW = textW("SELECT") + 24
    local plaW = textW("PLACE")  + 24

    local selectRect = { x = x,                y = boxY, w = selW, h = boxH }
    x = x + selW + 6
    local placeRect  = { x = x,                y = boxY, w = plaW, h = boxH }
    x = x + plaW + 24

    return {
        textY  = textY,
        select = selectRect,
        place  = placeRect,
        nextX  = x,
    }
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Géométrie des zones
-- ═════════════════════════════════════════════════════════════════════════════

local function tabBarRect()
    return 0, TOOLBAR_H, PALETTE_W, SECTION_H
end

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
    return ws.x - PROP_W - PAD, TOOLBAR_H + PAD, PROP_W, PROP_H
end

-- Construit la liste à plat des items affichés dans l'onglet LAYERS.
-- Retourne { { kind="layer", n, count } | { kind="entity", n, entry }, ... }
local function buildLayersFlat()
    local groups = entitiesByLayer()
    local flat   = {}
    for n = 0, 10 do
        table.insert(flat, { kind = "layer", n = n, count = #groups[n] })
        if _layerExpanded[n] then
            for _, entry in ipairs(groups[n]) do
                table.insert(flat, { kind = "entity", n = n, entry = entry })
            end
        end
    end
    return flat
end

-- Détermine le champ Properties cliqué (ou nil).
local function propertyRowAt(ws, sx, sy)
    local px, py, pw, ph = propertiesRect(ws)
    if not pointInRect(sx, sy, px, py, pw, ph) then return nil end
    local startY = py + SECTION_H + 6
    if sy < startY then return nil end
    local idx    = math.floor((sy - startY) / ITEM_H) + 1
    local fields = { "spriteID", "x", "y", "zOrder", "scale" }
    return fields[idx]
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

    -- Edit en cours : tout clic hors panneau Properties commit
    if _editing then
        local px, py, pw, ph = propertiesRect(ws)
        if not pointInRect(sx, sy, px, py, pw, ph) then applyEdit() end
    end

    -- Toolbar : boutons de mode SELECT / PLACE
    if sy < TOOLBAR_H then
        local lay = toolbarLayout()
        if pointInRect(sx, sy, lay.select.x, lay.select.y, lay.select.w, lay.select.h) then
            _placing = false
            return
        end
        if pointInRect(sx, sy, lay.place.x, lay.place.y, lay.place.w, lay.place.h) then
            if _spriteList[_paletteIdx] then
                _placing  = true
                _selected = nil
            end
            return
        end
        return
    end

    -- Tab bar palette
    do
        local tx, ty, tw, th = tabBarRect()
        if pointInRect(sx, sy, tx, ty, tw, th) then
            local halfW  = math.floor(PALETTE_W / 2)
            local newTab = (sx < halfW) and "sprites" or "layers"
            if newTab ~= _leftTab then
                _leftTab       = newTab
                _paletteScroll = 0
            end
            return
        end
    end

    -- Liste palette
    if sx < PALETTE_W then
        local lx, ly, _, lh = paletteListRect(ws)
        if not pointInRect(sx, sy, lx, ly, PALETTE_W, lh) then return end
        local i   = math.floor((sy - ly) / ITEM_H) + 1
        local idx = _paletteScroll + i

        if _leftTab == "sprites" then
            if idx >= 1 and idx <= #_spriteList then
                _paletteIdx = idx
                _placing    = true
                _selected   = nil
            end
        else
            local flat = buildLayersFlat()
            if idx >= 1 and idx <= #flat then
                local item = flat[idx]
                if item.kind == "layer" then
                    _layer                  = item.n
                    _layerExpanded[item.n]  = not _layerExpanded[item.n]
                else
                    _selected = item.entry
                    _placing  = false
                    local t = item.entry.entity:getTransform()
                    if t then Camera.setPosition(t.position.x, t.position.y) end
                end
            end
        end
        return
    end

    -- Properties : clic sur une ligne pour éditer
    if _selected then
        local px, py, pw, ph = propertiesRect(ws)
        if pointInRect(sx, sy, px, py, pw, ph) then
            local field = propertyRowAt(ws, sx, sy)
            if field then
                local t = _selected.entity:getTransform()
                local val
                if     field == "spriteID" then val = _selected.spriteID
                elseif field == "x"        then val = math.floor(t.position.x)
                elseif field == "y"        then val = math.floor(t.position.y)
                elseif field == "zOrder"   then val = _selected.zOrder
                elseif field == "scale"    then val = string.format("%.3g", t.scale.x or 1.0)
                end
                startEdit(field, val)
            end
            return
        end
    end

    -- Status bar
    if sy >= ws.y - STATUS_H then return end

    -- Zone monde
    local wx, wy = screenToWorld(sx, sy)
    wx, wy = snap(wx), snap(wy)

    if _placing then
        spawnSprite(_spriteList[_paletteIdx], wx, wy)
    else
        -- Sélectionne l'entité la plus haute (zOrder max) sous le curseur.
        local best, bestZ = nil, -1
        for _, entry in ipairs(_placed) do
            if hitTest(entry, sx, sy) and entry.zOrder > bestZ then
                best, bestZ = entry, entry.zOrder
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

    -- Édition de propriété : capture toute la saisie
    if _editing then handleEditKey(key); return end

    -- Ctrl seul (sans autre touche) : sortir du mode PLACE
    if key == "LControl" or key == "RControl" then
        if _placing then _placing = false end
        return
    end

    -- Raccourcis Ctrl+... (exclusifs : Ctrl+autre = ne rien déclencher)
    if isCtrl() then
        if key == "S" then saveScene(); return end
        if key == "G" then _snapGrid = (_snapGrid > 0) and 0 or 32; return end
        if key == "D" then _placing = false; _selected = nil; return end
        if key == "Num1" or key == "Numpad1" then
            _leftTab = "sprites"; _paletteScroll = 0; return
        end
        if key == "Num2" or key == "Numpad2" then
            _leftTab = "layers";  _paletteScroll = 0; return
        end
        return
    end

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

    -- Navigation palette
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
    Debug.fillRect(0, 0, ws.x, TOOLBAR_H, C_BG_DEEPER)
    Debug.fillRect(0, TOOLBAR_H - 1, ws.x, 1, C_BORDER)

    local lay = toolbarLayout()
    local y   = lay.textY

    Debug.label(PAD, y, "EDITEUR", C_TEXT)
    if not _saved then
        Debug.label(PAD + textW("EDITEUR") + 8, y, "*", C_WARN)
    end

    -- Boutons de mode (SELECT / PLACE)
    local mp = Input.getMousePosition()
    local function modeBtn(rect, label, active)
        local hover = pointInRect(mp.x, mp.y, rect.x, rect.y, rect.w, rect.h)
                      and not active
        local bg    = active and C_BG_ACTIVE
                      or (hover and C_BG_HOVER or C_BG_ITEM)
        Debug.fillRect(rect.x, rect.y, rect.w, rect.h, bg)
        if active then Debug.fillRect(rect.x, rect.y, 4, rect.h, C_ACCENT) end
        Debug.label(rect.x + 12, y, label, active and C_TEXT or C_TEXT_DIM)
    end
    modeBtn(lay.select, "SELECT", not _placing)
    modeBtn(lay.place,  "PLACE",  _placing)

    -- Champ Layer (indicateur global)
    local sectX = lay.nextX
    Debug.label(sectX, y, "Layer", C_TEXT_DIM)
    local layerStr = string.format(" %d ", _layer)
    local boxX     = sectX + textW("Layer") + 6
    local boxW     = textW(layerStr) + 8
    Debug.fillRect(boxX, y - 4, boxW, FONT_H + 8, C_BG_ITEM)
    Debug.fillRect(boxX, y - 4, 4, FONT_H + 8, C_ACCENT)
    Debug.label(boxX + 4, y, layerStr, C_TEXT)
    Debug.label(boxX + boxW + 8, y, "[ ]", C_TEXT_MUTE)

    local snapTxt = (_snapGrid > 0) and ("Snap " .. _snapGrid) or "Snap OFF"
    local zoomTxt = string.format("Zoom %.1fx", Camera.getZoom())
    local right   = snapTxt .. "    " .. zoomTxt
    Debug.label(ws.x - textW(right) - PAD, y, right, C_TEXT_DIM)
end

local function drawTabBar()
    local x, y, w, h = tabBarRect()
    local halfW      = math.floor(w / 2)

    Debug.fillRect(x, y, w, h, C_BG_DEEPER)
    Debug.fillRect(x, y + h - 1, w, 1, C_BORDER)

    local stY       = y + math.floor((h - FONT_H) / 2)
    local sprActive = (_leftTab == "sprites")
    local layActive = (_leftTab == "layers")

    if sprActive then
        Debug.fillRect(x, y, halfW, h, C_BG)
        Debug.fillRect(x, y + h - 2, halfW, 2, C_ACCENT)
    end
    Debug.label(x + PAD, stY, "SPRITES", sprActive and C_TEXT or C_TEXT_DIM)

    if layActive then
        Debug.fillRect(x + halfW, y, halfW, h, C_BG)
        Debug.fillRect(x + halfW, y + h - 2, halfW, 2, C_ACCENT)
    end
    Debug.label(x + halfW + PAD, stY, "LAYERS", layActive and C_TEXT or C_TEXT_DIM)

    Debug.fillRect(x + halfW, y + 6, 1, h - 12, C_BORDER)
end

local function drawSpritesTab(ws)
    local lx, ly, lw, lh = paletteListRect(ws)

    local vis = visibleItemCount(ws)
    if _paletteScroll + vis > #_spriteList then
        _paletteScroll = math.max(0, #_spriteList - vis)
    end

    local mp = Input.getMousePosition()
    for i = 1, vis do
        local idx = _paletteScroll + i
        if idx > #_spriteList then break end
        local id = _spriteList[idx]
        local y  = ly + (i - 1) * ITEM_H

        local isActive = (idx == _paletteIdx and _placing)
        local isHover  = pointInRect(mp.x, mp.y, lx, y, lw, ITEM_H) and not isActive
        local bg       = isActive and C_BG_ACTIVE or (isHover and C_BG_HOVER or C_BG_ITEM)

        Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
        if isActive then accentBar(lx + 4, y + 2, ITEM_H - 4) end

        local labelCol = (isActive or isHover) and C_TEXT or C_TEXT_DIM
        Debug.label(lx + PAD + (isActive and 6 or 0), y + TXT_PAD_Y, id, labelCol)

        local def = _spriteDefs[id]
        if def and def.zOrder then
            local zs = "z" .. def.zOrder
            Debug.label(lx + lw - textW(zs) - PAD, y + TXT_PAD_Y, zs, C_TEXT_MUTE)
        end
    end

    if #_spriteList > vis then
        local nav = string.format("%d-%d / %d", _paletteScroll + 1,
                                  math.min(_paletteScroll + vis, #_spriteList),
                                  #_spriteList)
        Debug.label(PAD, ly + lh - FONT_H - 2, nav, C_TEXT_MUTE)
    end
end

local function drawLayersTab(ws)
    local lx, ly, lw, lh = paletteListRect(ws)
    local flat           = buildLayersFlat()

    local vis = visibleItemCount(ws)
    if _paletteScroll + vis > #flat then
        _paletteScroll = math.max(0, #flat - vis)
    end

    local mp = Input.getMousePosition()
    for i = 1, vis do
        local idx = _paletteScroll + i
        if idx > #flat then break end
        local item    = flat[idx]
        local y       = ly + (i - 1) * ITEM_H
        local isHover = pointInRect(mp.x, mp.y, lx, y, lw, ITEM_H)

        if item.kind == "layer" then
            local isActive = (item.n == _layer)
            local bg       = isActive and C_BG_ACTIVE
                             or (isHover and C_BG_HOVER or C_BG_ITEM)

            Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
            if isActive then accentBar(lx + 4, y + 2, ITEM_H - 4) end

            local arrow = _layerExpanded[item.n] and "v" or ">"
            local txt   = arrow .. "  Layer " .. item.n
            Debug.label(lx + PAD + (isActive and 6 or 0), y + TXT_PAD_Y, txt,
                        isActive and C_TEXT or C_TEXT_DIM)

            local cnt = "(" .. item.count .. ")"
            Debug.label(lx + lw - textW(cnt) - PAD, y + TXT_PAD_Y, cnt,
                        item.count > 0 and C_ACCENT or C_TEXT_MUTE)
        else
            local isSelected = (_selected == item.entry)
            local bg         = isSelected and C_BG_SELECT
                              or (isHover and C_BG_HOVER or C_BG_SUB)

            Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
            if isSelected then accentBar(lx + 4, y + 2, ITEM_H - 4) end

            local txt = item.entry.spriteID
            Debug.label(lx + PAD + 26, y + TXT_PAD_Y,
                        truncate(txt, lw - 140),
                        isSelected and C_TEXT or C_TEXT_DIM)

            local t = item.entry.entity:getTransform()
            if t then
                local pos = string.format("%d,%d",
                                          math.floor(t.position.x),
                                          math.floor(t.position.y))
                Debug.label(lx + lw - textW(pos) - PAD, y + TXT_PAD_Y,
                            pos, C_TEXT_MUTE)
            end
        end
    end

    if #flat > vis then
        local nav = string.format("%d-%d / %d", _paletteScroll + 1,
                                  math.min(_paletteScroll + vis, #flat),
                                  #flat)
        Debug.label(PAD, ly + lh - FONT_H - 2, nav, C_TEXT_MUTE)
    end
end

local function drawPalette(ws)
    local paletteFullH = ws.y - STATUS_H

    Debug.fillRect(0, TOOLBAR_H, PALETTE_W, paletteFullH - TOOLBAR_H, C_BG)
    Debug.fillRect(PALETTE_W - 1, TOOLBAR_H, 1, paletteFullH - TOOLBAR_H, C_BORDER)

    drawTabBar()
    if _leftTab == "sprites" then drawSpritesTab(ws)
    else                          drawLayersTab(ws) end
end

local function drawProperties(ws)
    if not _selected then return end
    local px, py, pw, ph = propertiesRect(ws)
    local t = _selected.entity:getTransform()
    local s = _selected.entity:getSprite()
    if not (t and s) then return end

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

    -- Lignes éditables : { label, fieldKey, value, valueCol }
    local rows = {
        { "Sprite", "spriteID", _selected.spriteID,                         C_ACCENT },
        { "X",      "x",        math.floor(t.position.x),                   C_TEXT   },
        { "Y",      "y",        math.floor(t.position.y),                   C_TEXT   },
        { "Layer",  "zOrder",   _selected.zOrder,                           C_TEXT   },
        { "Scale",  "scale",    string.format("%.3g", t.scale.x or 1.0),    C_TEXT   },
    }

    local rowY = py + SECTION_H + 6
    local rh   = ITEM_H - 4
    local mp   = Input.getMousePosition()

    for _, r in ipairs(rows) do
        local label, fkey, value, valueCol = r[1], r[2], r[3], r[4]
        local isEdit  = (_editing and _editing.field == fkey)
        local isHover = pointInRect(mp.x, mp.y, px + PAD, rowY, pw - 2 * PAD, rh)
                        and not isEdit
        local bg      = isEdit  and C_BG_ACTIVE
                       or (isHover and C_BG_HOVER or C_BG_ITEM)

        Debug.fillRect(px + PAD, rowY, pw - 2 * PAD, rh, bg)
        if isEdit then accentBar(px + PAD, rowY, rh) end

        local txtY = rowY + math.floor((rh - FONT_H) / 2)
        Debug.label(px + PAD + (isEdit and 8 or 4), txtY, label,
                    isEdit and C_TEXT or C_TEXT_MUTE)

        local valStr = isEdit and (_editing.buffer .. "_") or tostring(value)
        Debug.label(px + 110, txtY, truncate(valStr, pw - 130), valueCol)

        rowY = rowY + ITEM_H
    end

    -- Hint en bas
    local hint = _editing and "Entree valide  -  Echap annule"
                            or "Clic ligne pour editer"
    if _editing and _editing.field == "spriteID" then
        hint = "Up/Down ou [ ] pour cycler  -  Entree valide"
    end
    Debug.label(px + PAD, py + ph - FONT_H - 6,
                truncate(hint, pw - 2 * PAD), C_TEXT_MUTE)
end

local function drawStatus(ws)
    local barY = ws.y - STATUS_H
    Debug.fillRect(0, barY, ws.x, STATUS_H, C_BG_DEEPER)
    Debug.fillRect(0, barY, ws.x, 1, C_BORDER)

    local y = barY + math.floor((STATUS_H - FONT_H) / 2)

    local mode, modeCol
    if _editing then
        mode    = "EDIT  " .. _editing.field
        modeCol = C_WARN
    elseif _placing then
        mode    = "PLACE  " .. (_spriteList[_paletteIdx] or "?")
        modeCol = C_ACCENT
    elseif _selected then
        mode    = "SELECT  " .. _selected.spriteID
        modeCol = C_HIGHLIGHT
    else
        mode    = "SELECT"
        modeCol = C_TEXT_DIM
    end
    Debug.label(PAD, y, mode, modeCol)

    local mp     = Input.getMousePosition()
    local wx, wy = screenToWorld(mp.x, mp.y)
    local coords = string.format("(%d, %d)", math.floor(snap(wx)), math.floor(snap(wy)))
    Debug.label(PAD + 280, y, coords, C_TEXT_DIM)

    local right = string.format("%d entites  -  Ctrl+S save  -  Ctrl exit place  -  Esc",
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
    local sc  = (def and def.scale)  or 1.0
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
