-- nova/vec2.lua
-- Utilitaires mathématiques 2D complétant le type Vec2f natif.
-- Disponible sans require : Vec2 est un global auto-chargé.

local Vec2 = {}

function Vec2.length(v)
    return math.sqrt(v.x * v.x + v.y * v.y)
end

function Vec2.normalize(v)
    local len = Vec2.length(v)
    if len == 0 then return Vec2f.new(0, 0) end
    return Vec2f.new(v.x / len, v.y / len)
end

function Vec2.distance(a, b)
    local dx = b.x - a.x
    local dy = b.y - a.y
    return math.sqrt(dx * dx + dy * dy)
end

function Vec2.lerp(a, b, t)
    return Vec2f.new(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t)
end

function Vec2.dot(a, b)
    return a.x * b.x + a.y * b.y
end

function Vec2.angle(v)
    return math.atan(v.y, v.x)
end

function Vec2.fromAngle(angle, length)
    length = length or 1
    return Vec2f.new(math.cos(angle) * length, math.sin(angle) * length)
end

function Vec2.new(x, y)
    return Vec2f.new(x or 0, y or 0)
end

return Vec2
