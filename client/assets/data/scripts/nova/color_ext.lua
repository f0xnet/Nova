-- nova/color_ext.lua
-- Étend le type Color C++ avec des utilitaires haut niveau.
-- Ajoute Color.lerp, Color.fromHex, Color.fromHSV, Color.withAlpha…
-- Disponible sans require : les méthodes sont ajoutées au global Color.
--
-- API ajoutée à Color :
--   Color.lerp(a, b, t)         -- interpolation linéaire entre deux couleurs
--   Color.fromHex("#ff8800")    -- depuis une chaîne hexadécimale (#rrggbb ou #rrggbbaa)
--   Color.fromHSV(h, s, v)      -- depuis teinte (0-360), saturation (0-1), valeur (0-1)
--   Color.toHex(c)              -- → "#rrggbb"
--   Color.withAlpha(c, a)       -- copie avec alpha modifié (0.0–1.0)
--   Color.multiply(c, factor)   -- assombrit/éclaircit (factor 0..2)
--
-- Exemple :
--   local fire = Color.fromHSV(20, 1, 1)    -- orange vif
--   local fade = Color.lerp(fire, Color.Black, 0.5)
--   local sprite = entity:getSprite()
--   if sprite then sprite.tint = fade end

-- Interpolation linéaire (t = 0..1)
Color.lerp = function(a, b, t)
    t = math.max(0.0, math.min(1.0, t))
    return Color.new(
        math.floor(a.r + (b.r - a.r) * t + 0.5),
        math.floor(a.g + (b.g - a.g) * t + 0.5),
        math.floor(a.b + (b.b - a.b) * t + 0.5),
        math.floor(a.a + (b.a - a.a) * t + 0.5)
    )
end

-- Depuis une chaîne hex : "#ff8800", "#ff8800cc", "ff8800"
Color.fromHex = function(hex)
    hex = tostring(hex):gsub("^#", "")
    local r = tonumber(hex:sub(1,2), 16) or 0
    local g = tonumber(hex:sub(3,4), 16) or 0
    local b = tonumber(hex:sub(5,6), 16) or 0
    local a = (#hex >= 8) and (tonumber(hex:sub(7,8), 16) or 255) or 255
    return Color.new(r, g, b, a)
end

-- Depuis HSV : h=0-360 (teinte), s=0-1 (saturation), v=0-1 (valeur)
Color.fromHSV = function(h, s, v, a)
    h = h % 360
    local c  = v * s
    local x  = c * (1 - math.abs((h / 60) % 2 - 1))
    local m  = v - c
    local r, g, b
    if     h < 60  then r, g, b = c, x, 0
    elseif h < 120 then r, g, b = x, c, 0
    elseif h < 180 then r, g, b = 0, c, x
    elseif h < 240 then r, g, b = 0, x, c
    elseif h < 300 then r, g, b = x, 0, c
    else                r, g, b = c, 0, x
    end
    return Color.new(
        math.floor((r + m) * 255 + 0.5),
        math.floor((g + m) * 255 + 0.5),
        math.floor((b + m) * 255 + 0.5),
        math.floor((a or 1.0) * 255 + 0.5)
    )
end

-- Vers chaîne hex
Color.toHex = function(c)
    return string.format("#%02x%02x%02x", c.r, c.g, c.b)
end

-- Copie avec alpha modifié (a = 0.0–1.0)
Color.withAlpha = function(c, a)
    return Color.new(c.r, c.g, c.b, math.floor(a * 255 + 0.5))
end

-- Multiplie les composantes RGB (0.0 = noir, 1.0 = identique, 2.0 = double)
Color.multiply = function(c, factor)
    return Color.new(
        math.min(255, math.floor(c.r * factor + 0.5)),
        math.min(255, math.floor(c.g * factor + 0.5)),
        math.min(255, math.floor(c.b * factor + 0.5)),
        c.a
    )
end

-- Ce module étend Color en place ; le retourner permet de le tracer via "ColorExt = require(...)"
return Color
