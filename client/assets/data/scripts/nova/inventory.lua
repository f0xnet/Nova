-- nova/inventory.lua
-- Inventaire par entité — inspiré de l'inventaire Papyrus / Skyrim.
-- Disponible sans require : Inventory est un global auto-chargé.
--
-- API :
--   Inventory.add(entityId, item, count)       -- ajoute des items (count défaut: 1)
--   Inventory.remove(entityId, item, count)    -- retire, retourne la qté réellement retirée
--   Inventory.count(entityId, item)            -- quantité possédée
--   Inventory.has(entityId, item, count)       -- possède au moins N ? (count défaut: 1)
--   Inventory.clear(entityId)                  -- vide l'inventaire de l'entité
--   Inventory.clearItem(entityId, item)        -- retire toutes les instances d'un item
--   Inventory.getAll(entityId)                 -- { item = count, ... }
--   Inventory.transfer(from, to, item, count)  -- transfert entre deux entités
--   Inventory.getWeight(entityId)              -- poids total (si items définis avec weight)
--   Inventory.defineItem(item, def)            -- enregistre les propriétés d'un item
--
-- EventBus events émis :
--   "item_added"   { entityId, item, count, total }
--   "item_removed" { entityId, item, count, total }
--
-- Exemple :
--   Inventory.add(entity.id, "gold_coin",  50)
--   Inventory.add(entity.id, "health_pot",  3)
--
--   if Inventory.has(entity.id, "dungeon_key") then
--       Inventory.remove(entity.id, "dungeon_key", 1)
--       entity:sendMessage("openDoor")
--   end
--
--   -- Loot d'un coffre
--   Inventory.transfer(chest.id, entity.id, "sword", 1)
--   Inventory.transfer(chest.id, entity.id, "gold_coin", 30)

local Inventory = {}

-- bags[entityId][item] = count
local bags = {}

-- Définitions d'items (poids, maxStack, displayName...)
local itemDefs = {}

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

-- Enregistre les propriétés d'un type d'item (optionnel)
-- Inventory.defineItem("health_pot", { weight=0.5, maxStack=99, displayName="Potion de soin" })
function Inventory.defineItem(item, def)
    itemDefs[item] = def
end

function Inventory.add(entityId, item, count)
    entityId = toId(entityId)
    count = math.max(count or 1, 1)
    if not bags[entityId] then bags[entityId] = {} end
    local def = itemDefs[item]
    local maxStack = def and def.maxStack
    local current = bags[entityId][item] or 0
    local added = maxStack and math.min(count, maxStack - current) or count
    if added <= 0 then return 0 end
    bags[entityId][item] = current + added
    EventBus.emit("item_added", {
        entityId = entityId, item = item,
        count = added, total = bags[entityId][item],
    })
    return added
end

-- Retourne la quantité réellement retirée (peut être < count si insuffisant)
function Inventory.remove(entityId, item, count)
    entityId = toId(entityId)
    count = math.max(count or 1, 1)
    if not bags[entityId] or not bags[entityId][item] then return 0 end
    local current = bags[entityId][item]
    local removed = math.min(current, count)
    bags[entityId][item] = current - removed
    if bags[entityId][item] <= 0 then bags[entityId][item] = nil end
    local total = bags[entityId][item] or 0
    EventBus.emit("item_removed", {
        entityId = entityId, item = item,
        count = removed, total = total,
    })
    return removed
end

function Inventory.count(entityId, item)
    entityId = toId(entityId)
    return (bags[entityId] and bags[entityId][item]) or 0
end

function Inventory.has(entityId, item, count)
    return Inventory.count(entityId, item) >= (count or 1)
end

function Inventory.clearItem(entityId, item)
    entityId = toId(entityId)
    if bags[entityId] then bags[entityId][item] = nil end
end

function Inventory.clear(entityId)
    entityId = toId(entityId)
    bags[entityId] = nil
end

function Inventory.getAll(entityId)
    entityId = toId(entityId)
    if not bags[entityId] then return {} end
    local copy = {}
    for item, count in pairs(bags[entityId]) do copy[item] = count end
    return copy
end

EventBus.on("entity_destroyed", function(data)
    Inventory.clear(data.entityId)
end)

-- Transfert atomique : retire de `from`, ajoute à `to`
function Inventory.transfer(fromEntity, toEntity, item, count)
    local removed = Inventory.remove(fromEntity, item, count)
    if removed > 0 then Inventory.add(toEntity, item, removed) end
    return removed
end

-- Poids total (nécessite Inventory.defineItem avec champ weight)
function Inventory.getWeight(entityId)
    entityId = toId(entityId)
    if not bags[entityId] then return 0 end
    local total = 0
    for item, count in pairs(bags[entityId]) do
        local def = itemDefs[item]
        if def and def.weight then total = total + def.weight * count end
    end
    return total
end

return Inventory
