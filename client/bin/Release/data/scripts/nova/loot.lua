-- nova/loot.lua
-- Tables de butin pondérées avec quantités, conditions et bonus.
-- Disponible sans require : Loot est un global auto-chargé.
--
-- API :
--   Loot.define(id, table)            -- enregistre une loot table
--   Loot.roll(id, count, ctx)         -- tire <count> fois (défaut 1), retourne { item, quantity }[]
--   Loot.rollOne(id, ctx)             -- tire une seule fois, retourne { item, quantity } ou nil
--   Loot.dropFor(entity, tableId)     -- roll + Inventory.addItem pour chaque drop
--
-- Format d'une loot table :
--   {
--     guaranteed = {                  -- toujours inclus
--       { item = "gold_coin", quantity = { min=5, max=20 } },
--     },
--     pool = {                        -- tirage pondéré
--       { item = "potion_hp",  weight = 40, quantity = 1 },
--       { item = "sword_iron", weight = 10, quantity = 1,
--         cond = function(ctx) return ctx and ctx.level >= 5 end },
--       { item = nil,          weight = 50 },   -- rien
--     },
--     rolls = 2,                      -- nombre de tirages dans pool (défaut 1)
--   }
--
-- Exemple :
--   Loot.define("goblin", {
--       guaranteed = {
--           { item = "gold_coin", quantity = { min = 1, max = 5 } },
--       },
--       pool = {
--           { item = "potion_hp",   weight = 30 },
--           { item = "leather_cap", weight = 10,
--             cond = function(ctx) return ctx and ctx.difficulty == "hard" end },
--           { item = nil,           weight = 60 },
--       },
--       rolls = 1,
--   })
--
--   local drops = Loot.roll("goblin", 1, { difficulty = "hard" })
--   for _, drop in ipairs(drops) do
--       if drop.item then
--           print("Dropped: " .. drop.item .. " x" .. drop.quantity)
--       end
--   end

local Loot = {}
local _tables = {}

local function _resolveQty(q)
    if type(q) == "number" then return q end
    if type(q) == "table" then
        return math.random(q.min or 1, q.max or 1)
    end
    return 1
end

local function _rollPool(pool, ctx)
    local eligible = {}
    local total    = 0
    for _, entry in ipairs(pool) do
        if not entry.cond or entry.cond(ctx) then
            eligible[#eligible + 1] = entry
            total = total + (entry.weight or 1)
        end
    end
    if #eligible == 0 then return nil end

    local r, cum = math.random() * total, 0
    for _, entry in ipairs(eligible) do
        cum = cum + (entry.weight or 1)
        if r <= cum then
            if entry.item then
                return { item = entry.item, quantity = _resolveQty(entry.quantity or 1) }
            end
            return nil  -- "rien"
        end
    end
    return nil
end

function Loot.define(id, t)
    _tables[id] = t
end

function Loot.roll(id, count, ctx)
    local def = _tables[id]
    if not def then return {} end
    count = count or 1

    local results = {}

    -- Guaranteed drops
    if def.guaranteed then
        for _, g in ipairs(def.guaranteed) do
            for _ = 1, count do
                results[#results + 1] = {
                    item     = g.item,
                    quantity = _resolveQty(g.quantity or 1),
                }
            end
        end
    end

    -- Pool drops
    if def.pool then
        local rolls = (def.rolls or 1) * count
        for _ = 1, rolls do
            local drop = _rollPool(def.pool, ctx)
            if drop then
                results[#results + 1] = drop
            end
        end
    end

    return results
end

function Loot.rollOne(id, ctx)
    local results = Loot.roll(id, 1, ctx)
    return results[1] or nil
end

function Loot.dropFor(entity, tableId, ctx)
    local drops = Loot.roll(tableId, 1, ctx)
    local eid   = (type(entity) == "userdata") and entity.id or entity
    for _, drop in ipairs(drops) do
        if drop.item then
            Inventory.add(eid, drop.item, drop.quantity)
        end
    end
end

return Loot
