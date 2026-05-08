-- nova/random.lua
-- Générateur aléatoire ergonomique avec seed, dés, poids, shuffle.
-- Disponible sans require : Random est un global auto-chargé.
--
-- API :
--   Random.seed(n)                   -- initialise le générateur
--   Random.int(min, max)             -- entier dans [min, max]
--   Random.float(min, max)           -- flottant dans [min, max] (défaut 0..1)
--   Random.chance(p)                 -- true avec probabilité p (0.0–1.0)
--   Random.pick(table)               -- élément aléatoire d'une liste
--   Random.shuffle(table)            -- copie mélangée (Fisher-Yates)
--   Random.dice(faces)               -- 1dN (ex: Random.dice(6))
--   Random.dice(count, faces)        -- NdN (ex: Random.dice(2, 6) = 2d6)
--   Random.weighted(items)           -- tirage pondéré { value, weight }
--   Random.gaussian(mean, std)       -- distribution normale (Box-Muller)
--
-- Exemple :
--   Random.seed(os.time())
--   local dir = Random.pick({"north","south","east","west"})
--   if Random.chance(0.1) then triggerRareEvent() end
--   local damage = Random.int(8, 15) + Random.dice(2, 6)
--   local drop = Random.weighted({
--       { value = "gold",  weight = 60 },
--       { value = "sword", weight = 10 },
--       { value = nil,     weight = 30 },   -- rien
--   })

local Random = {}

function Random.seed(n)
    math.randomseed(n)
end

function Random.int(min, max)
    return math.random(min, max)
end

function Random.float(min, max)
    min = min or 0.0
    max = max or 1.0
    return min + math.random() * (max - min)
end

function Random.chance(p)
    return math.random() < p
end

function Random.pick(t)
    if not t or #t == 0 then return nil end
    return t[math.random(#t)]
end

-- Retourne une COPIE mélangée (Fisher-Yates) — n'altère pas l'original
function Random.shuffle(t)
    local copy = {}
    for i, v in ipairs(t) do copy[i] = v end
    for i = #copy, 2, -1 do
        local j = math.random(i)
        copy[i], copy[j] = copy[j], copy[i]
    end
    return copy
end

-- Random.dice(6) = 1d6, Random.dice(3, 6) = 3d6
function Random.dice(countOrFaces, faces)
    local count, n
    if faces then
        count, n = countOrFaces, faces
    else
        count, n = 1, countOrFaces
    end
    local total = 0
    for _ = 1, count do total = total + math.random(n) end
    return total
end

-- Tirage pondéré : items = { { value=…, weight=… }, … }
-- Le champ value peut être n'importe quoi (string, table, nil pour "rien")
function Random.weighted(items)
    if not items or #items == 0 then return nil end
    local total = 0
    for _, item in ipairs(items) do total = total + (item.weight or 1) end
    local r, cum = math.random() * total, 0
    for _, item in ipairs(items) do
        cum = cum + (item.weight or 1)
        if r <= cum then return item.value end
    end
    return items[#items].value
end

-- Distribution normale approchée (Box-Muller)
-- mean = 0, std = 1 par défaut
function Random.gaussian(mean, std)
    mean = mean or 0.0
    std  = std  or 1.0
    local u, v
    repeat u = math.random() until u > 0
    v = math.random()
    local z = math.sqrt(-2.0 * math.log(u)) * math.cos(2.0 * math.pi * v)
    return mean + z * std
end

-- Initialisation automatique avec le temps système
Random.seed(os.time())

return Random
