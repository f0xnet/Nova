-- editor/editor.lua
-- Éditeur de niveau in-game — Ctrl+E pour activer/désactiver.
--
-- Modes d'outils (toolbar) :
--   SELECT              — clic monde sélectionne une entité
--   PLACE               — clic monde dépose l'élément courant de la palette
--   Choisir un élément dans la palette bascule automatiquement en PLACE.
--
-- Contrôles globaux :
--   Ctrl+E              — toggle éditeur
--   Ctrl+S              — sauvegarder la scène courante
--   Ctrl                — sortir du mode PLACE (retour SELECT)
--   Ctrl+D              — désélectionner / sortir PLACE
--   Ctrl+G              — bascule grille
--   Ctrl+1 / Ctrl+2     — bascule onglet PALETTE / LAYERS
--   Ctrl+Up / Ctrl+Down — layer +1 / -1 (entité sélectionnée ou layer actif)
--   Echap               — désélec. / fermer / annuler une édition
--   G                   — bascule grille (alias)
--   Clic droit + drag   — pan caméra
--
-- Panneau gauche (tabs) :
--   PALETTE             — sections « Sprites » + « Entities »
--   LAYERS              — inspector des entités groupées par zOrder
--
-- Panneau droit (Properties) :
--   Clic ligne          — édite la valeur (Entrée valide, Echap annule)
--   Up/Down             — cycle dans la liste sprite (édition spriteID)
--   Clic ligne bool     — toggle on/off
--
-- Sélection :
--   WASD / Flèches      — déplace l'entité (pas = grille ou 1 px)
--   Backspace           — supprime l'entité sélectionnée
--   0–9                 — fixe le layer directement
--
-- Note rendu : toutes les primitives passent par Debug.* (font 28 px).

local Editor = {}

-- ═════════════════════════════════════════════════════════════════════════════
-- État
-- ═════════════════════════════════════════════════════════════════════════════

local _enabled          = false
local _spriteDefs       = {}     -- id  → def
local _spriteDefsByPath = {}     -- texture path → list of defs
local _spriteList       = {}     -- sprite IDs (ordered)
local _paletteFlat      = {}     -- liste plate de la palette (sections + items)

local _paletteIdx       = 1      -- index dans _paletteFlat (item courant)
local _paletteScroll    = 0
local _placing          = false
local _placed           = {}     -- entries { entity, kind, zOrder, spriteID? }
local _selected         = nil
local _snapGrid         = 32
local _layer            = 3
local _scenePath        = nil
local _sceneName        = "Untitled"
local _saved            = true

local _panning          = false
local _panStartMouse    = { x = 0, y = 0 }
local _panStartCam      = { x = 0, y = 0 }

local _leftTab          = "palette"   -- "palette" | "layers"
local _layerExpanded    = {}          -- [n] = bool
local _editing          = nil         -- { field, buffer }

-- ═════════════════════════════════════════════════════════════════════════════
-- Thème
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
local C_LIGHT_GHOST = Color.new(255, 220, 100,  90)
local C_LIGHT_RING  = Color.new(255, 220, 100, 200)
local C_HIGHLIGHT   = Color.new(255, 200,  60, 230)
local C_GRID        = Color.new( 80,  85, 110,  55)

-- ═════════════════════════════════════════════════════════════════════════════
-- Constantes layout
-- ═════════════════════════════════════════════════════════════════════════════

local FONT_H    = 28
local TOOLBAR_H = 48
local STATUS_H  = 44
local SECTION_H = 36
local ITEM_H    = 40
local HEAD_H    = 32
local PALETTE_W = 280
local PROP_W    = 340
local PROP_H    = 460       -- assez pour les lumières (8+ rows)
local PAD       = 10
local TXT_PAD_Y = math.floor((ITEM_H - FONT_H) / 2)

-- ═════════════════════════════════════════════════════════════════════════════
-- Helpers généraux
-- ═════════════════════════════════════════════════════════════════════════════

local function pointInRect(px, py, rx, ry, rw, rh)
    return px >= rx and px < rx + rw and py >= ry and py < ry + rh
end

local function snap(v)
    if _snapGrid <= 0 then return v end
    return math.floor(v / _snapGrid + 0.5) * _snapGrid
end

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

local function isCtrl()
    return Input.isKeyPressed("LControl") or Input.isKeyPressed("RControl")
end

local function textW(s) return #s * 16 end

local function truncate(s, maxPx)
    local maxC = math.max(1, math.floor(maxPx / 16))
    if #s <= maxC then return s end
    return s:sub(1, maxC - 1) .. "…"
end

local function clamp(v, lo, hi) return math.max(lo, math.min(hi, v)) end

local function fmtNum(v)
    if not v then return "0" end
    if math.floor(v) == v then return tostring(math.floor(v)) end
    return string.format("%.3g", v)
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Définitions sprite + palette
-- ═════════════════════════════════════════════════════════════════════════════

-- Liste statique des entités placeables (hors sprites).
local _entityPaletteList = {
    { kind = "player",      label = "Player",        defaultSprite = "player" },
    { kind = "light_point", label = "Light (point)" },
    { kind = "light_spot",  label = "Light (spot)"  },
}

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
        local list = _spriteDefsByPath[s.texture]
        if not list then list = {}; _spriteDefsByPath[s.texture] = list end
        table.insert(list, s)
        table.insert(_spriteList, s.id)
    end
    Log.info("[Editor] " .. #_spriteList .. " sprites chargés")
end

-- Construit la palette plate avec sections et items.
-- Chaque entrée est :
--   { isSection = true, label = "..." }
--   { kind = "sprite",      spriteID = "...", label = "..." }
--   { kind = "player",      spriteID = "player", label = "Player" }
--   { kind = "light_point", label = "Light (point)" }
--   { kind = "light_spot",  label = "Light (spot)" }
local function buildPalette()
    _paletteFlat = {}
    table.insert(_paletteFlat, { isSection = true, label = "Sprites" })
    for _, id in ipairs(_spriteList) do
        local def = _spriteDefs[id]
        table.insert(_paletteFlat, {
            kind     = "sprite",
            spriteID = id,
            label    = id,
            zHint    = def and def.zOrder,
        })
    end
    table.insert(_paletteFlat, { isSection = true, label = "Entities" })
    for _, e in ipairs(_entityPaletteList) do
        table.insert(_paletteFlat, {
            kind          = e.kind,
            spriteID      = e.defaultSprite,
            label         = e.label,
        })
    end
end

-- Retourne le premier index "item" (non section) — utilisé à l'init.
local function firstItemIndex()
    for i, it in ipairs(_paletteFlat) do
        if not it.isSection then return i end
    end
    return 1
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Hit-test AABB (selection)
-- ═════════════════════════════════════════════════════════════════════════════

local function hitTestSprite(entry, sx, sy)
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

-- Pour les lumières (pas de sprite) : disque autour de la position.
local function hitTestLight(entry, sx, sy)
    local t = entry.entity:getTransform()
    if not t then return false end
    local cx, cy = worldToScreen(t.position.x, t.position.y)
    local d = math.sqrt((sx - cx)^2 + (sy - cy)^2)
    return d <= 28
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Kinds — chaque type d'entité éditable
-- ═════════════════════════════════════════════════════════════════════════════

local Kinds = {}

-- ──────────────────── Sprite ────────────────────

Kinds.sprite = {
    label = "Sprite",

    spawn = function(wx, wy, spriteID)
        local def = _spriteDefs[spriteID]
        if not def then return nil end

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

        return { entity = e, kind = "sprite", spriteID = spriteID, zOrder = _layer }
    end,

    rows = function(entry)
        local t = entry.entity:getTransform()
        return {
            { "Sprite", "spriteID", entry.spriteID,            C_ACCENT },
            { "X",      "x",        math.floor(t.position.x),  C_TEXT },
            { "Y",      "y",        math.floor(t.position.y),  C_TEXT },
            { "Layer",  "zOrder",   entry.zOrder,              C_TEXT },
            { "Scale",  "scale",    fmtNum(t.scale.x or 1.0),  C_TEXT },
        }
    end,

    apply = function(entry, field, buf)
        local t = entry.entity:getTransform()
        local s = entry.entity:getSprite()
        if not (t and s) then return end
        if field == "x" then
            local n = tonumber(buf); if n then t.position.x = n; _saved = false end
        elseif field == "y" then
            local n = tonumber(buf); if n then t.position.y = n; _saved = false end
        elseif field == "zOrder" then
            local n = tonumber(buf)
            if n then
                n = clamp(math.floor(n), 0, 10)
                entry.zOrder = n
                s.zOrder = n
                _saved = false
            end
        elseif field == "scale" then
            local n = tonumber(buf)
            if n and n > 0 then t.scale.x, t.scale.y = n, n; _saved = false end
        elseif field == "spriteID" then
            local def = _spriteDefs[buf]
            if def then
                entry.spriteID  = buf
                s.textureID     = buf
                s.textureHandle = Resources.loadTexture(def.texture)
                s.size.x        = def.width
                s.size.y        = def.height
                t.origin.x      = def.width  * 0.5
                t.origin.y      = def.height * 0.5
                local sc        = def.scale or 1.0
                t.scale.x, t.scale.y = sc, sc
                _saved = false
            end
        end
    end,

    serialize = function(entry, indent)
        local t = entry.entity:getTransform()
        local p = indent or "    "
        return {
            p .. '{',
            p .. '  "type": "sprite",',
            p .. '  "spriteID": "' .. entry.spriteID .. '",',
            p .. string.format('  "x": %d,', math.floor(t.position.x)),
            p .. string.format('  "y": %d,', math.floor(t.position.y)),
            p .. string.format('  "zOrder": %d', entry.zOrder),
            p .. '}',
        }
    end,

    hitTest = hitTestSprite,
}

-- ──────────────────── Player ────────────────────

Kinds.player = {
    label = "Player",

    spawn = function(wx, wy, spriteID)
        spriteID = spriteID or "player"
        local def = _spriteDefs[spriteID]
        if not def then return nil end

        local e = Registry:createEntity()
        e:addComponent("TransformComponent")
        e:addComponent("SpriteComponent")
        e:addComponent("TagComponent")

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

        local tag = e:getTag()
        if tag then tag.tag = "player" end

        return { entity = e, kind = "player", spriteID = spriteID, zOrder = _layer }
    end,

    rows = Kinds.sprite and Kinds.sprite.rows,
    apply = Kinds.sprite and Kinds.sprite.apply,

    serialize = function(entry, indent)
        local t = entry.entity:getTransform()
        local p = indent or "    "
        return {
            p .. '{',
            p .. '  "type": "player",',
            p .. '  "spriteID": "' .. entry.spriteID .. '",',
            p .. string.format('  "x": %d,', math.floor(t.position.x)),
            p .. string.format('  "y": %d,', math.floor(t.position.y)),
            p .. string.format('  "zOrder": %d', entry.zOrder),
            p .. '}',
        }
    end,

    hitTest = hitTestSprite,
}
-- Player partage les mêmes lignes/apply que sprite
Kinds.player.rows  = Kinds.sprite.rows
Kinds.player.apply = Kinds.sprite.apply

-- ──────────────────── Light (point/spot) ────────────────────

local function spawnLight(wx, wy, lightType)
    local e = Registry:createEntity()
    e:addComponent("TransformComponent")
    e:addComponent("LightComponent")

    local t = e:getTransform()
    t.position.x = wx
    t.position.y = wy

    local l = e:getLight()
    if l then
        l.type        = lightType
        l.color       = Color.new(255, 240, 200, 255)
        l.radius      = 400.0
        l.intensity   = 0.8
        l.enabled     = true
        l.castShadows = false
        if lightType == "spot" then
            l.direction = Vec2f.new(0.0, 1.0)
            l.angle     = 90.0
        end
    end

    return { entity = e, kind = "light_" .. lightType, zOrder = _layer }
end

local function lightRows(entry, isSpot)
    local t = entry.entity:getTransform()
    local l = entry.entity:getLight()
    if not (t and l) then return {} end
    local rows = {
        { "Type",      "_kind",   isSpot and "spot" or "point",  C_ACCENT },
        { "X",         "x",       math.floor(t.position.x),       C_TEXT },
        { "Y",         "y",       math.floor(t.position.y),       C_TEXT },
        { "Layer",     "zOrder",  entry.zOrder,                   C_TEXT },
        { "Color R",   "color_r", l.color.r,                      C_TEXT },
        { "Color G",   "color_g", l.color.g,                      C_TEXT },
        { "Color B",   "color_b", l.color.b,                      C_TEXT },
        { "Radius",    "radius",  math.floor(l.radius),           C_TEXT },
        { "Intensity", "intensity_100", math.floor((l.intensity or 0) * 100), C_TEXT },
        { "Enabled",   "enabled", l.enabled and "ON" or "OFF",    l.enabled and C_OK or C_TEXT_MUTE },
        { "Shadows",   "castShadows", l.castShadows and "ON" or "OFF",
                                                                  l.castShadows and C_OK or C_TEXT_MUTE },
    }
    if isSpot then
        table.insert(rows, { "Angle",  "angle", math.floor(l.angle or 0), C_TEXT })
        table.insert(rows, { "Dir X10","dir_x_10",
                             math.floor((l.direction.x or 0) * 10), C_TEXT })
        table.insert(rows, { "Dir Y10","dir_y_10",
                             math.floor((l.direction.y or 0) * 10), C_TEXT })
    end
    return rows
end

local function lightApply(entry, field, buf)
    local t = entry.entity:getTransform()
    local l = entry.entity:getLight()
    if not (t and l) then return end
    if field == "x" then
        local n = tonumber(buf); if n then t.position.x = n; _saved = false end
    elseif field == "y" then
        local n = tonumber(buf); if n then t.position.y = n; _saved = false end
    elseif field == "zOrder" then
        local n = tonumber(buf)
        if n then entry.zOrder = clamp(math.floor(n), 0, 10); _saved = false end
    elseif field == "color_r" then
        local n = tonumber(buf); if n then
            local c = l.color
            l.color = Color.new(clamp(math.floor(n),0,255), c.g, c.b, c.a)
            _saved = false
        end
    elseif field == "color_g" then
        local n = tonumber(buf); if n then
            local c = l.color
            l.color = Color.new(c.r, clamp(math.floor(n),0,255), c.b, c.a)
            _saved = false
        end
    elseif field == "color_b" then
        local n = tonumber(buf); if n then
            local c = l.color
            l.color = Color.new(c.r, c.g, clamp(math.floor(n),0,255), c.a)
            _saved = false
        end
    elseif field == "radius" then
        local n = tonumber(buf); if n then l.radius = n; _saved = false end
    elseif field == "intensity_100" then
        local n = tonumber(buf); if n then l.intensity = clamp(n,0,200) / 100.0; _saved = false end
    elseif field == "angle" then
        local n = tonumber(buf); if n then l.angle = n; _saved = false end
    elseif field == "dir_x_10" then
        local n = tonumber(buf); if n then
            l.direction = Vec2f.new(n / 10.0, l.direction.y)
            _saved = false
        end
    elseif field == "dir_y_10" then
        local n = tonumber(buf); if n then
            l.direction = Vec2f.new(l.direction.x, n / 10.0)
            _saved = false
        end
    elseif field == "enabled" then
        l.enabled = not l.enabled; _saved = false
    elseif field == "castShadows" then
        l.castShadows = not l.castShadows; _saved = false
    end
end

local function lightSerialize(entry, indent, isSpot)
    local t = entry.entity:getTransform()
    local l = entry.entity:getLight()
    local p = indent or "    "
    local lines = {
        p .. '{',
        p .. '  "components": {',
        p .. '    "transform": {',
        p .. string.format('      "position": [%d, %d]',
                           math.floor(t.position.x), math.floor(t.position.y)),
        p .. '    },',
        p .. '    "light": {',
        p .. string.format('      "type": "%s",', isSpot and "spot" or "point"),
        p .. string.format('      "color": [%d, %d, %d, %d],',
                           l.color.r, l.color.g, l.color.b, l.color.a),
        p .. string.format('      "radius": %s,',    fmtNum(l.radius)),
        p .. string.format('      "intensity": %s,', fmtNum(l.intensity)),
        p .. string.format('      "castShadows": %s,', tostring(l.castShadows)),
        p .. string.format('      "enabled": %s%s',  tostring(l.enabled), isSpot and "," or ""),
    }
    if isSpot then
        table.insert(lines, p .. string.format('      "direction": [%s, %s],',
                                               fmtNum(l.direction.x), fmtNum(l.direction.y)))
        table.insert(lines, p .. string.format('      "angle": %s', fmtNum(l.angle)))
    end
    table.insert(lines, p .. '    }')
    table.insert(lines, p .. '  }')
    table.insert(lines, p .. '}')
    return lines
end

Kinds.light_point = {
    label   = "Light (point)",
    spawn   = function(wx, wy) return spawnLight(wx, wy, "point") end,
    rows    = function(entry) return lightRows(entry, false) end,
    apply   = lightApply,
    serialize = function(entry, indent) return lightSerialize(entry, indent, false) end,
    hitTest = hitTestLight,
}

Kinds.light_spot = {
    label   = "Light (spot)",
    spawn   = function(wx, wy) return spawnLight(wx, wy, "spot") end,
    rows    = function(entry) return lightRows(entry, true) end,
    apply   = lightApply,
    serialize = function(entry, indent) return lightSerialize(entry, indent, true) end,
    hitTest = hitTestLight,
}

-- ═════════════════════════════════════════════════════════════════════════════
-- Identification d'entités au chargement de scène
-- ═════════════════════════════════════════════════════════════════════════════

local function resolveSpriteDef(s)
    local def = _spriteDefs[s.textureID]
    if def then return def end
    local candidates = _spriteDefsByPath[s.textureID]
    if not candidates then return nil end
    if #candidates == 1 then return candidates[1] end
    for _, c in ipairs(candidates) do
        if c.width == s.size.x and c.height == s.size.y then return c end
    end
    return candidates[1]
end

local function identifyEntity(e)
    local hasLight = e:hasComponent("LightComponent")
    if hasLight then
        local l = e:getLight()
        if l then
            local k = (l.type == "spot") and "light_spot" or "light_point"
            return { kind = k, zOrder = _layer }
        end
    end

    local hasSprite = e:hasComponent("SpriteComponent")
    if hasSprite then
        local s = e:getSprite()
        if not s or s.textureID == "" then return nil end
        local def = resolveSpriteDef(s)
        if not def then return nil end

        local tag = e:getTag()
        local isPlayer = (tag and tag.tag == "player")
                      or e:hasComponent("ScriptComponent")
        if isPlayer then
            return { kind = "player", spriteID = def.id, zOrder = s.zOrder }
        end
        return { kind = "sprite", spriteID = def.id, zOrder = s.zOrder }
    end
    return nil
end

local function importSceneEntities()
    _placed = {}
    local total, accepted = 0, 0
    for _, e in ipairs(Registry:getAllEntities()) do
        total = total + 1
        local info = identifyEntity(e)
        if info then
            info.entity = e
            table.insert(_placed, info)
            accepted = accepted + 1
        end
    end
    Log.info(string.format("[Editor] Import : total=%d accepted=%d", total, accepted))
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Spawn / suppression / layer
-- ═════════════════════════════════════════════════════════════════════════════

local function spawnFromPaletteItem(item, wx, wy)
    local kind = Kinds[item.kind]
    if not kind or not kind.spawn then return nil end
    local entry
    if item.kind == "sprite" or item.kind == "player" then
        entry = kind.spawn(wx, wy, item.spriteID)
    else
        entry = kind.spawn(wx, wy)
    end
    if entry then
        table.insert(_placed, entry)
        _saved = false
    end
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
    n = clamp(n, 0, 10)
    entry.zOrder = n
    local s = entry.entity:getSprite()
    if s then s.zOrder = n end
    _saved = false
end

-- ═════════════════════════════════════════════════════════════════════════════
-- Édition de propriétés
-- ═════════════════════════════════════════════════════════════════════════════

local _BOOL_FIELDS = { enabled = true, castShadows = true }

local function startEdit(entry, field)
    if not entry then return end
    local kind = Kinds[entry.kind]
    if not kind or not kind.rows then return end

    if _BOOL_FIELDS[field] then
        -- Bool : toggle direct sans buffer
        if kind.apply then kind.apply(entry, field, nil) end
        return
    end

    -- Récupérer la valeur courante depuis rows()
    local rows = kind.rows(entry)
    for _, r in ipairs(rows) do
        if r[2] == field then
            _editing = { field = field, buffer = tostring(r[3]) }
            return
        end
    end
end

local function applyEdit()
    if not _editing then return end
    if not _selected then _editing = nil; return end
    local kind = Kinds[_selected.kind]
    if kind and kind.apply then
        kind.apply(_selected, _editing.field, _editing.buffer)
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
    if key == "Return" or key == "Enter" then applyEdit(); return end
    if key == "Escape" then cancelEdit(); return end
    if key == "Backspace" then
        _editing.buffer = _editing.buffer:sub(1, -2); return
    end
    if _editing.field == "spriteID" then
        if key == "Up"   then cycleSprite(-1); return end
        if key == "Down" then cycleSprite( 1); return end
        return
    end
    local n = key:match("^(%d)$")
    if n then _editing.buffer = _editing.buffer .. n; return end
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
        local kind = Kinds[entry.kind]
        if kind and kind.serialize then
            local block = kind.serialize(entry, "    ")
            -- Ajoute la virgule en fin de bloc (sauf dernier)
            local closing = block[#block]
            if i < #_placed then block[#block] = closing .. "," end
            for _, l in ipairs(block) do table.insert(lines, l) end
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
    buildPalette()
    _paletteIdx = firstItemIndex()
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
-- UI helpers + géométrie
-- ═════════════════════════════════════════════════════════════════════════════

local function accentBar(x, y, h)
    Debug.fillRect(x, y, 4, h, C_ACCENT)
end

local function toolbarLayout()
    local textY = math.floor((TOOLBAR_H - FONT_H) / 2)
    local boxH  = FONT_H + 8
    local boxY  = textY - 4
    local x = PAD + textW("EDITEUR") + 8
    if not _saved then x = x + 18 end
    x = x + 16
    local selW = textW("SELECT") + 24
    local plaW = textW("PLACE")  + 24
    local selectRect = { x = x,                y = boxY, w = selW, h = boxH }
    x = x + selW + 6
    local placeRect  = { x = x,                y = boxY, w = plaW, h = boxH }
    x = x + plaW + 24
    return { textY = textY, select = selectRect, place = placeRect, nextX = x }
end

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

-- Calcule la hauteur de la properties panel selon le nombre de lignes.
local function propertiesContentH(rowCount)
    return SECTION_H + 6 + rowCount * ITEM_H + 12 + FONT_H + 6
end

local function propertiesRect(ws)
    local h = PROP_H
    if _selected then
        local kind = Kinds[_selected.kind]
        local rows = (kind and kind.rows) and kind.rows(_selected) or nil
        if rows then h = propertiesContentH(#rows) end
    end
    return ws.x - PROP_W - PAD, TOOLBAR_H + PAD, PROP_W, h
end

local function buildLayersFlat()
    local groups = {}
    for n = 0, 10 do groups[n] = {} end
    for _, e in ipairs(_placed) do
        local n = clamp(e.zOrder or 0, 0, 10)
        table.insert(groups[n], e)
    end
    local flat = {}
    for n = 0, 10 do
        table.insert(flat, { kind = "layer", n = n, count = #groups[n] })
        if _layerExpanded[n] then
            for _, entry in ipairs(groups[n]) do
                table.insert(flat, { kind = "entity", entry = entry })
            end
        end
    end
    return flat
end

local function propertyRowAt(ws, sx, sy)
    if not _selected then return nil end
    local kind = Kinds[_selected.kind]
    if not kind or not kind.rows then return nil end
    local rows = kind.rows(_selected)
    local px, py, pw, ph = propertiesRect(ws)
    if not pointInRect(sx, sy, px, py, pw, ph) then return nil end
    local startY = py + SECTION_H + 6
    if sy < startY then return nil end
    local idx = math.floor((sy - startY) / ITEM_H) + 1
    local r = rows[idx]
    return r and r[2] or nil
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

    -- Toolbar : boutons SELECT / PLACE
    if sy < TOOLBAR_H then
        local lay = toolbarLayout()
        if pointInRect(sx, sy, lay.select.x, lay.select.y, lay.select.w, lay.select.h) then
            _placing = false; return
        end
        if pointInRect(sx, sy, lay.place.x, lay.place.y, lay.place.w, lay.place.h) then
            if _paletteFlat[_paletteIdx] and not _paletteFlat[_paletteIdx].isSection then
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
            local newTab = (sx < halfW) and "palette" or "layers"
            if newTab ~= _leftTab then _leftTab = newTab; _paletteScroll = 0 end
            return
        end
    end

    -- Liste palette
    if sx < PALETTE_W then
        local lx, ly, _, lh = paletteListRect(ws)
        if not pointInRect(sx, sy, lx, ly, PALETTE_W, lh) then return end
        local i   = math.floor((sy - ly) / ITEM_H) + 1
        local idx = _paletteScroll + i

        if _leftTab == "palette" then
            local item = _paletteFlat[idx]
            if item and not item.isSection then
                _paletteIdx = idx
                _placing    = true
                _selected   = nil
            end
        else
            local flat = buildLayersFlat()
            local f    = flat[idx]
            if f then
                if f.kind == "layer" then
                    _layer = f.n
                    _layerExpanded[f.n] = not _layerExpanded[f.n]
                else
                    _selected = f.entry
                    _placing  = false
                    local t = f.entry.entity:getTransform()
                    if t then Camera.setPosition(t.position.x, t.position.y) end
                end
            end
        end
        return
    end

    -- Properties : clic sur une ligne
    if _selected then
        local px, py, pw, ph = propertiesRect(ws)
        if pointInRect(sx, sy, px, py, pw, ph) then
            local field = propertyRowAt(ws, sx, sy)
            if field and field ~= "_kind" then
                startEdit(_selected, field)
            end
            return
        end
    end

    if sy >= ws.y - STATUS_H then return end

    -- Zone monde
    local wx, wy = screenToWorld(sx, sy)
    wx, wy = snap(wx), snap(wy)

    if _placing then
        local item = _paletteFlat[_paletteIdx]
        if item and not item.isSection then
            spawnFromPaletteItem(item, wx, wy)
        end
    else
        local best, bestZ = nil, -1
        for _, entry in ipairs(_placed) do
            local kind = Kinds[entry.kind]
            local ht = kind and kind.hitTest
            if ht and ht(entry, sx, sy) and (entry.zOrder or 0) > bestZ then
                best, bestZ = entry, entry.zOrder or 0
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
    if _editing then handleEditKey(key); return end

    if key == "LControl" or key == "RControl" then
        if _placing then _placing = false end
        return
    end

    if isCtrl() then
        if key == "S" then saveScene(); return end
        if key == "G" then _snapGrid = (_snapGrid > 0) and 0 or 32; return end
        if key == "D" then _placing = false; _selected = nil; return end
        if key == "1" then _leftTab = "palette"; _paletteScroll = 0; return end
        if key == "2" then _leftTab = "layers";  _paletteScroll = 0; return end
        if key == "Up" then
            if _selected then setEntryLayer(_selected, (_selected.zOrder or 0) + 1)
            else _layer = math.min(10, _layer + 1) end
            return
        end
        if key == "Down" then
            if _selected then setEntryLayer(_selected, (_selected.zOrder or 0) - 1)
            else _layer = math.max(0, _layer - 1) end
            return
        end
        return
    end

    if key == "Escape" then
        if _placing      then _placing  = false
        elseif _selected then _selected = nil
        else                  Editor.disable() end
        return
    end

    if key == "Backspace" and _selected then
        removeEntry(_selected); _selected = nil; return
    end

    if key == "G" then
        _snapGrid = (_snapGrid > 0) and 0 or 32; return
    end

    -- Navigation palette (saute les sections)
    local function paletteStep(dir)
        local n = #_paletteFlat
        local i = _paletteIdx
        for _ = 1, n do
            i = i + dir
            if i < 1 then i = n elseif i > n then i = 1 end
            if not _paletteFlat[i].isSection then
                _paletteIdx = i
                local vis = visibleItemCount(Viewport.getWindowSize())
                if i - 1 < _paletteScroll then _paletteScroll = math.max(0, i - 2) end
                if i - 1 >= _paletteScroll + vis then _paletteScroll = i - vis end
                _placing = true
                return
            end
        end
    end

    if key == "Up" and not _selected then paletteStep(-1); return end
    if key == "Down" and not _selected then paletteStep( 1); return end

    -- Layer direct
    local num = key:match("^(%d)$")
    if num then
        local n = tonumber(num)
        if _selected then setEntryLayer(_selected, n) else _layer = n end
        return
    end

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

    local mp = Input.getMousePosition()
    local function modeBtn(rect, label, active)
        local hover = pointInRect(mp.x, mp.y, rect.x, rect.y, rect.w, rect.h)
                      and not active
        local bg = active and C_BG_ACTIVE or (hover and C_BG_HOVER or C_BG_ITEM)
        Debug.fillRect(rect.x, rect.y, rect.w, rect.h, bg)
        if active then Debug.fillRect(rect.x, rect.y, 4, rect.h, C_ACCENT) end
        Debug.label(rect.x + 12, y, label, active and C_TEXT or C_TEXT_DIM)
    end
    modeBtn(lay.select, "SELECT", not _placing)
    modeBtn(lay.place,  "PLACE",  _placing)

    local sectX = lay.nextX
    Debug.label(sectX, y, "Layer", C_TEXT_DIM)
    local layerStr = string.format(" %d ", _layer)
    local boxX     = sectX + textW("Layer") + 6
    local boxW     = textW(layerStr) + 8
    Debug.fillRect(boxX, y - 4, boxW, FONT_H + 8, C_BG_ITEM)
    Debug.fillRect(boxX, y - 4, 4, FONT_H + 8, C_ACCENT)
    Debug.label(boxX + 4, y, layerStr, C_TEXT)

    local snapTxt = (_snapGrid > 0) and ("Snap " .. _snapGrid) or "Snap OFF"
    local zoomTxt = string.format("Zoom %.1fx", Camera.getZoom())
    local right   = snapTxt .. "    " .. zoomTxt
    Debug.label(ws.x - textW(right) - PAD, y, right, C_TEXT_DIM)
end

local function drawTabBar()
    local x, y, w, h = tabBarRect()
    local halfW = math.floor(w / 2)
    Debug.fillRect(x, y, w, h, C_BG_DEEPER)
    Debug.fillRect(x, y + h - 1, w, 1, C_BORDER)
    local stY = y + math.floor((h - FONT_H) / 2)
    local palActive = (_leftTab == "palette")
    local layActive = (_leftTab == "layers")
    if palActive then
        Debug.fillRect(x, y, halfW, h, C_BG)
        Debug.fillRect(x, y + h - 2, halfW, 2, C_ACCENT)
    end
    Debug.label(x + PAD, stY, "PALETTE", palActive and C_TEXT or C_TEXT_DIM)
    if layActive then
        Debug.fillRect(x + halfW, y, halfW, h, C_BG)
        Debug.fillRect(x + halfW, y + h - 2, halfW, 2, C_ACCENT)
    end
    Debug.label(x + halfW + PAD, stY, "LAYERS", layActive and C_TEXT or C_TEXT_DIM)
    Debug.fillRect(x + halfW, y + 6, 1, h - 12, C_BORDER)
end

local function drawPaletteTab(ws)
    local lx, ly, lw, lh = paletteListRect(ws)
    local n = #_paletteFlat
    local vis = visibleItemCount(ws)
    if _paletteScroll + vis > n then
        _paletteScroll = math.max(0, n - vis)
    end
    local mp = Input.getMousePosition()
    for i = 1, vis do
        local idx = _paletteScroll + i
        if idx > n then break end
        local item = _paletteFlat[idx]
        local y    = ly + (i - 1) * ITEM_H

        if item.isSection then
            Debug.fillRect(lx, y, lw, ITEM_H, C_BG_DEEPER)
            Debug.label(lx + PAD, y + TXT_PAD_Y, item.label, C_TEXT_MUTE)
            Debug.fillRect(lx + PAD, y + ITEM_H - 2, lw - 2 * PAD, 1, C_BORDER)
        else
            local isActive = (idx == _paletteIdx and _placing)
            local isHover  = pointInRect(mp.x, mp.y, lx, y, lw, ITEM_H) and not isActive
            local bg = isActive and C_BG_ACTIVE
                       or (isHover and C_BG_HOVER or C_BG_ITEM)
            Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
            if isActive then accentBar(lx + 4, y + 2, ITEM_H - 4) end
            local col = (isActive or isHover) and C_TEXT or C_TEXT_DIM
            Debug.label(lx + PAD + (isActive and 6 or 0), y + TXT_PAD_Y,
                        truncate(item.label, lw - 50), col)
            if item.zHint then
                local zs = "z" .. item.zHint
                Debug.label(lx + lw - textW(zs) - PAD, y + TXT_PAD_Y, zs, C_TEXT_MUTE)
            end
        end
    end
end

local function drawLayersTab(ws)
    local lx, ly, lw, lh = paletteListRect(ws)
    local flat = buildLayersFlat()
    local vis  = visibleItemCount(ws)
    if _paletteScroll + vis > #flat then
        _paletteScroll = math.max(0, #flat - vis)
    end
    local mp = Input.getMousePosition()
    for i = 1, vis do
        local idx = _paletteScroll + i
        if idx > #flat then break end
        local item = flat[idx]
        local y = ly + (i - 1) * ITEM_H
        local isHover = pointInRect(mp.x, mp.y, lx, y, lw, ITEM_H)

        if item.kind == "layer" then
            local isActive = (item.n == _layer)
            local bg = isActive and C_BG_ACTIVE
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
            local bg = isSelected and C_BG_SELECT
                       or (isHover and C_BG_HOVER or C_BG_SUB)
            Debug.fillRect(lx + 4, y + 2, lw - 8, ITEM_H - 4, bg)
            if isSelected then accentBar(lx + 4, y + 2, ITEM_H - 4) end
            local kind = Kinds[item.entry.kind]
            local label = item.entry.spriteID or (kind and kind.label) or item.entry.kind
            Debug.label(lx + PAD + 26, y + TXT_PAD_Y,
                        truncate(label, lw - 40),
                        isSelected and C_TEXT or C_TEXT_DIM)
        end
    end
end

local function drawPalette(ws)
    local paletteFullH = ws.y - STATUS_H
    Debug.fillRect(0, TOOLBAR_H, PALETTE_W, paletteFullH - TOOLBAR_H, C_BG)
    Debug.fillRect(PALETTE_W - 1, TOOLBAR_H, 1, paletteFullH - TOOLBAR_H, C_BORDER)
    drawTabBar()
    if _leftTab == "palette" then drawPaletteTab(ws) else drawLayersTab(ws) end
end

local function drawProperties(ws)
    if not _selected then return end
    local kind = Kinds[_selected.kind]
    if not kind or not kind.rows then return end
    local rows = kind.rows(_selected)
    local px, py, pw, ph = propertiesRect(ws)

    Debug.fillRect(px + 4, py + 4, pw, ph, Color.new(0, 0, 0, 80))
    Debug.fillRect(px,     py,     pw, ph, C_BG)
    Debug.fillRect(px,     py,     pw, 1,  C_BORDER_LITE)
    Debug.fillRect(px,     py + ph - 1, pw, 1, C_BORDER_LITE)
    Debug.fillRect(px,     py,     1,  ph, C_BORDER_LITE)
    Debug.fillRect(px + pw - 1, py, 1, ph, C_BORDER_LITE)

    Debug.fillRect(px, py, pw, SECTION_H, C_BG_DEEPER)
    Debug.fillRect(px, py + SECTION_H - 1, pw, 1, C_BORDER)
    Debug.label(px + PAD, py + math.floor((SECTION_H - FONT_H) / 2),
                kind.label or "PROPERTIES", C_TEXT_DIM)

    local rowY = py + SECTION_H + 6
    local rh   = ITEM_H - 4
    local mp   = Input.getMousePosition()

    for _, r in ipairs(rows) do
        local label, fkey, value, valueCol = r[1], r[2], r[3], r[4]
        local isEdit  = (_editing and _editing.field == fkey)
        local isHover = pointInRect(mp.x, mp.y, px + PAD, rowY, pw - 2 * PAD, rh)
                        and not isEdit
        local bg = isEdit and C_BG_ACTIVE
                   or (isHover and C_BG_HOVER or C_BG_ITEM)
        Debug.fillRect(px + PAD, rowY, pw - 2 * PAD, rh, bg)
        if isEdit then accentBar(px + PAD, rowY, rh) end
        local txtY = rowY + math.floor((rh - FONT_H) / 2)
        Debug.label(px + PAD + (isEdit and 8 or 4), txtY, label,
                    isEdit and C_TEXT or C_TEXT_MUTE)
        local valStr = isEdit and (_editing.buffer .. "_") or tostring(value)
        Debug.label(px + 130, txtY, truncate(valStr, pw - 150), valueCol)
        rowY = rowY + ITEM_H
    end

    local hint = _editing and "Entree valide  -  Echap annule"
                            or "Clic ligne pour editer"
    if _editing and _editing.field == "spriteID" then
        hint = "Up/Down cycler  -  Entree valide"
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
        local item = _paletteFlat[_paletteIdx]
        mode    = "PLACE  " .. ((item and item.label) or "?")
        modeCol = C_ACCENT
    elseif _selected then
        local k = Kinds[_selected.kind]
        mode    = "SELECT  " .. ((k and k.label) or _selected.kind)
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
    if not t then return end
    local z = Camera.getZoom()
    local cx, cy = worldToScreen(t.position.x, t.position.y)
    local th = 2

    local s = _selected.entity:getSprite()
    if s and (_selected.kind == "sprite" or _selected.kind == "player") then
        local hw = s.size.x * (t.scale.x or 1) * 0.5 * z
        local hh = s.size.y * (t.scale.y or 1) * 0.5 * z
        Debug.fillRect(cx - hw - th, cy - hh - th, hw * 2 + th * 2, th, C_HIGHLIGHT)
        Debug.fillRect(cx - hw - th, cy + hh,       hw * 2 + th * 2, th, C_HIGHLIGHT)
        Debug.fillRect(cx - hw - th, cy - hh - th, th, hh * 2 + th * 2, C_HIGHLIGHT)
        Debug.fillRect(cx + hw,      cy - hh - th, th, hh * 2 + th * 2, C_HIGHLIGHT)
    else
        local r = 28
        Debug.fillRect(cx - r - th, cy - r - th, r * 2 + th * 2, th, C_HIGHLIGHT)
        Debug.fillRect(cx - r - th, cy + r,       r * 2 + th * 2, th, C_HIGHLIGHT)
        Debug.fillRect(cx - r - th, cy - r - th, th, r * 2 + th * 2, C_HIGHLIGHT)
        Debug.fillRect(cx + r,      cy - r - th, th, r * 2 + th * 2, C_HIGHLIGHT)
    end
end

local function drawLightMarkers()
    -- Marqueurs visuels pour toutes les lumières (cercle jaune)
    for _, entry in ipairs(_placed) do
        if entry.kind == "light_point" or entry.kind == "light_spot" then
            local t = entry.entity:getTransform()
            if t then
                local cx, cy = worldToScreen(t.position.x, t.position.y)
                local r = 22
                Debug.fillRect(cx - r, cy - r, r * 2, r * 2, C_LIGHT_GHOST)
                Debug.fillRect(cx - r, cy - r, r * 2, 1, C_LIGHT_RING)
                Debug.fillRect(cx - r, cy + r - 1, r * 2, 1, C_LIGHT_RING)
                Debug.fillRect(cx - r, cy - r, 1, r * 2, C_LIGHT_RING)
                Debug.fillRect(cx + r - 1, cy - r, 1, r * 2, C_LIGHT_RING)
                local letter = (entry.kind == "light_spot") and "S" or "P"
                Debug.label(cx - 8, cy - 14, letter, C_LIGHT_RING)
            end
        end
    end
end

local function drawGhost()
    if not _placing then return end
    local item = _paletteFlat[_paletteIdx]
    if not item or item.isSection then return end
    local mp = Input.getMousePosition()
    if mp.x < PALETTE_W or mp.y < TOOLBAR_H then return end

    local wx, wy = screenToWorld(mp.x, mp.y)
    wx, wy = snap(wx), snap(wy)
    local sx, sy = worldToScreen(wx, wy)

    if item.kind == "sprite" or item.kind == "player" then
        local def = _spriteDefs[item.spriteID]
        local z   = Camera.getZoom()
        local sc  = (def and def.scale)  or 1.0
        local hw  = ((def and def.width)  or 64) * sc * 0.5 * z
        local hh  = ((def and def.height) or 64) * sc * 0.5 * z
        Debug.fillRect(sx - hw, sy - hh, hw * 2, hh * 2, C_GHOST)
        Debug.fillRect(sx - hw, sy - hh, hw * 2, 1,      C_GHOST_B)
        Debug.fillRect(sx - hw, sy + hh, hw * 2, 1,      C_GHOST_B)
        Debug.fillRect(sx - hw, sy - hh, 1, hh * 2,      C_GHOST_B)
        Debug.fillRect(sx + hw, sy - hh, 1, hh * 2,      C_GHOST_B)
    else
        local r = 22
        Debug.fillRect(sx - r, sy - r, r * 2, r * 2, C_LIGHT_GHOST)
        Debug.fillRect(sx - r, sy - r, r * 2, 1, C_LIGHT_RING)
        Debug.fillRect(sx - r, sy + r - 1, r * 2, 1, C_LIGHT_RING)
        Debug.fillRect(sx - r, sy - r, 1, r * 2, C_LIGHT_RING)
        Debug.fillRect(sx + r - 1, sy - r, 1, r * 2, C_LIGHT_RING)
    end
    Debug.fillRect(sx - 8, sy, 16, 1, C_GHOST_B)
    Debug.fillRect(sx, sy - 8, 1, 16, C_GHOST_B)
end

function Editor.draw()
    if not _enabled then return end
    local ws = Viewport.getWindowSize()
    drawGrid(ws)
    drawLightMarkers()
    drawSelectionHighlight()
    drawGhost()
    drawToolbar(ws)
    drawPalette(ws)
    drawProperties(ws)
    drawStatus(ws)
end

-- ═════════════════════════════════════════════════════════════════════════════
-- EventBus
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
