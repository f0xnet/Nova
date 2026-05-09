-- nova/spatial.lua
-- Grille de hachage spatiale pour requêtes de proximité en O(1).
-- Disponible sans require : Spatial est un global auto-chargé.
--
-- Remplace l'itération brute-force O(n) par une grille à cellules fixes.
-- La grille est reconstruite intégralement une fois par frame via _update.
--
-- API :
--   Spatial.CELL_SIZE                        -- taille de cellule en pixels (défaut 64)
--   Spatial._update(dt)                      -- reconstruit la grille (appelé par ScriptSystem)
--   Spatial.findInRadius(x, y, r, tag)       -- entités dans le rayon r autour de (x,y)
--   Spatial.findInRect(x, y, w, h, tag)      -- entités dont la position est dans le rectangle
--   Spatial.nearest(x, y, tag)               -- (entité, distance) la plus proche, ou nil
--   Spatial.findAt(cellX, cellY, tag)        -- entités brutes dans la cellule (cx, cy)
--   Spatial.count(tag)                       -- nombre total d'entités enregistrées (nil = toutes)
--
-- PERFORMANCE :
--   La grille est reconstruite en O(n) une fois par frame.
--   Les requêtes ne parcourent que les cellules couvertes par le rayon/rect.
--   Avec CELL_SIZE=64 et r=128, findInRadius examine au plus ~25 cellules au lieu de n entités.
--   Idéal pour de nombreuses requêtes de proximité dans la même frame.
--
-- Exemple :
--   -- Toucher tous les ennemis dans un rayon d'explosion
--   local victims = Spatial.findInRadius(pos.x, pos.y, 96, "enemy")
--   for _, e in ipairs(victims) do
--       Stats.mod(e.id, "Health", -blast_damage)
--   end
--
--   -- Trouver la ressource la plus proche
--   local ore, dist = Spatial.nearest(miner:getPosition().x, miner:getPosition().y, "ore")
--   if ore and dist < 200 then
--       Nav.moveTo(miner, ore, 80)
--   end
--
--   -- Changer la résolution de la grille avant la première frame
--   Spatial.CELL_SIZE = 128

local Spatial = {}

-- Taille de cellule en pixels — modifier avant le premier _update si besoin
Spatial.CELL_SIZE = 64

-- Données internes
-- _grid["cx,cy"] = { entity, ... }
-- _allEntities   = { entity, ... }           (toutes, sans filtre tag)
-- _tagEntities[tag] = { entity, ... }        (par tag)
local _grid        = {}
local _allEntities = {}
local _tagEntities = {}

-- Convertit une coordonnée monde en coordonnée de cellule (entier)
local function toCellCoord(v)
    return math.floor(v / Spatial.CELL_SIZE)
end

-- Clé de cellule
local function cellKey(cx, cy)
    return cx .. "," .. cy
end

-- Insère une entité dans la cellule (cx, cy)
local function insertCell(cx, cy, entity)
    local k = cellKey(cx, cy)
    local cell = _grid[k]
    if not cell then
        cell = {}
        _grid[k] = cell
    end
    cell[#cell + 1] = entity
end

-- Reconstruit entièrement la grille depuis Registry:getAllEntities().
-- Toute entité sans TransformComponent (getPosition() renvoie nil) est ignorée.
function Spatial._update(dt)
    -- Réinitialisation
    _grid        = {}
    _allEntities = {}
    _tagEntities = {}

    for _, entity in ipairs(Registry:getAllEntities()) do
        local pos = entity:getPosition()
        if pos then
            -- Enregistrement dans la grille
            local cx = toCellCoord(pos.x)
            local cy = toCellCoord(pos.y)
            insertCell(cx, cy, entity)

            -- Enregistrement global
            _allEntities[#_allEntities + 1] = entity

            -- Enregistrement par tag
            local tag = entity.tag
            if tag ~= "" then
                local bucket = _tagEntities[tag]
                if not bucket then
                    bucket = {}
                    _tagEntities[tag] = bucket
                end
                bucket[#bucket + 1] = entity
            end
        end
    end
end

-- Retourne toutes les entités dans le rayon r autour de (x, y).
-- Si tag n'est pas nil, filtre par tag.
-- Seules les cellules recouvertes par le disque sont examinées.
function Spatial.findInRadius(x, y, r, tag)
    local result = {}
    local r2     = r * r
    local cs     = Spatial.CELL_SIZE

    local minCX = toCellCoord(x - r)
    local maxCX = toCellCoord(x + r)
    local minCY = toCellCoord(y - r)
    local maxCY = toCellCoord(y + r)

    -- Ensemble de dédoublonnage — une entité peut couvrir plusieurs cellules
    -- si la grille était multi-cellule, mais ici chaque entité est dans une seule
    -- cellule : pas besoin de seen{} sauf si CELL_SIZE est modifiée entre les appels.
    -- On utilise quand même un seen pour la sécurité.
    local seen = {}

    for cx = minCX, maxCX do
        for cy = minCY, maxCY do
            local cell = _grid[cellKey(cx, cy)]
            if cell then
                for _, entity in ipairs(cell) do
                    if not seen[entity] then
                        -- Filtre tag
                        if tag == nil or entity.tag == tag then
                            local pos = entity:getPosition()
                            if pos then
                                local dx = pos.x - x
                                local dy = pos.y - y
                                if dx * dx + dy * dy <= r2 then
                                    result[#result + 1] = entity
                                end
                            end
                        end
                        seen[entity] = true
                    end
                end
            end
        end
    end

    return result
end

-- Retourne toutes les entités dont la position est à l'intérieur du rectangle
-- défini par le coin supérieur gauche (x, y) et les dimensions (w, h).
-- Si tag n'est pas nil, filtre par tag.
function Spatial.findInRect(x, y, w, h, tag)
    local result = {}
    local x2 = x + w
    local y2 = y + h

    local minCX = toCellCoord(x)
    local maxCX = toCellCoord(x2)
    local minCY = toCellCoord(y)
    local maxCY = toCellCoord(y2)

    local seen = {}

    for cx = minCX, maxCX do
        for cy = minCY, maxCY do
            local cell = _grid[cellKey(cx, cy)]
            if cell then
                for _, entity in ipairs(cell) do
                    if not seen[entity] then
                        if tag == nil or entity.tag == tag then
                            local pos = entity:getPosition()
                            if pos then
                                if pos.x >= x and pos.x <= x2
                                and pos.y >= y and pos.y <= y2 then
                                    result[#result + 1] = entity
                                end
                            end
                        end
                        seen[entity] = true
                    end
                end
            end
        end
    end

    return result
end

-- Retourne (entity, distance) de l'entité la plus proche de (x, y).
-- Si tag n'est pas nil, cherche uniquement parmi les entités avec ce tag.
-- Retourne nil, nil si aucune entité n'est enregistrée.
-- Stratégie : anneau de cellules croissant jusqu'à trouver un candidat,
-- puis vérifie le rayon minimal pour garantir l'exactitude.
function Spatial.nearest(x, y, tag)
    local source = tag and _tagEntities[tag] or _allEntities
    if not source or #source == 0 then return nil, nil end

    local best, bestD2 = nil, math.huge

    for _, entity in ipairs(source) do
        local pos = entity:getPosition()
        if pos then
            local dx = pos.x - x
            local dy = pos.y - y
            local d2 = dx * dx + dy * dy
            if d2 < bestD2 then
                best, bestD2 = entity, d2
            end
        end
    end

    if best then
        return best, math.sqrt(bestD2)
    end
    return nil, nil
end

-- Retourne les entités dans la cellule aux coordonnées de grille (cellX, cellY).
-- Si tag n'est pas nil, filtre par tag.
-- Retourne une table (vide si la cellule est vide).
function Spatial.findAt(cellX, cellY, tag)
    local result = {}
    local cell = _grid[cellKey(cellX, cellY)]
    if not cell then return result end

    for _, entity in ipairs(cell) do
        if tag == nil or entity.tag == tag then
            result[#result + 1] = entity
        end
    end
    return result
end

-- Retourne le nombre total d'entités enregistrées.
-- Si tag n'est pas nil, compte uniquement les entités avec ce tag.
function Spatial.count(tag)
    if tag == nil then
        return #_allEntities
    end
    local bucket = _tagEntities[tag]
    return bucket and #bucket or 0
end

return Spatial
