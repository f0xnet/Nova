-- nova/table_ext.lua
-- Utilitaires fonctionnels sur les tables Lua.
-- Disponible sans require : Table est un global auto-chargé.
-- (Complémente la stdlib 'table' sans la remplacer.)
--
-- API :
--   Table.contains(t, value)       -- vrai si value est dans t
--   Table.find(t, fn)              -- premier élément satisfaisant fn
--   Table.filter(t, fn)            -- sous-table des éléments satisfaisant fn
--   Table.map(t, fn)               -- table transformée par fn
--   Table.reduce(t, fn, initial)   -- réduction
--   Table.keys(t)                  -- liste des clés
--   Table.values(t)                -- liste des valeurs
--   Table.shuffle(t)               -- copie mélangée
--   Table.pick(t)                  -- élément aléatoire
--   Table.merge(a, b)              -- fusion shallow (b écrase a)
--   Table.count(t)                 -- nombre de paires (works on dict)
--   Table.reverse(t)               -- copie inversée
--   Table.unique(t)                -- supprime les doublons
--   Table.flatten(t)               -- aplatit les tables imbriquées

local Table = {}

function Table.contains(t, value)
    for _, v in ipairs(t) do
        if v == value then return true end
    end
    return false
end

-- Retourne (valeur, index) ou (nil, nil)
function Table.find(t, fn)
    for i, v in ipairs(t) do
        if fn(v) then return v, i end
    end
    return nil, nil
end

function Table.filter(t, fn)
    local result = {}
    for _, v in ipairs(t) do
        if fn(v) then table.insert(result, v) end
    end
    return result
end

function Table.map(t, fn)
    local result = {}
    for i, v in ipairs(t) do result[i] = fn(v, i) end
    return result
end

function Table.reduce(t, fn, initial)
    local acc = initial
    for _, v in ipairs(t) do acc = fn(acc, v) end
    return acc
end

function Table.keys(t)
    local keys = {}
    for k in pairs(t) do table.insert(keys, k) end
    return keys
end

function Table.values(t)
    local vals = {}
    for _, v in pairs(t) do table.insert(vals, v) end
    return vals
end

-- Retourne une COPIE mélangée (Fisher-Yates)
function Table.shuffle(t)
    local copy = {}
    for i, v in ipairs(t) do copy[i] = v end
    for i = #copy, 2, -1 do
        local j = math.random(i)
        copy[i], copy[j] = copy[j], copy[i]
    end
    return copy
end

function Table.pick(t)
    if #t == 0 then return nil end
    return t[math.random(#t)]
end

-- Fusion shallow : les clés de b écrasent celles de a
function Table.merge(a, b)
    local result = {}
    for k, v in pairs(a) do result[k] = v end
    for k, v in pairs(b) do result[k] = v end
    return result
end

-- Fonctionne sur les dicts (contrairement à #t)
function Table.count(t)
    local n = 0
    for _ in pairs(t) do n = n + 1 end
    return n
end

function Table.reverse(t)
    local result = {}
    for i = #t, 1, -1 do table.insert(result, t[i]) end
    return result
end

function Table.unique(t)
    local seen, result = {}, {}
    for _, v in ipairs(t) do
        if not seen[v] then
            seen[v] = true
            table.insert(result, v)
        end
    end
    return result
end

-- Aplatit récursivement les tables imbriquées en une liste plate
function Table.flatten(t)
    local result = {}
    for _, v in ipairs(t) do
        if type(v) == "table" then
            for _, inner in ipairs(Table.flatten(v)) do
                table.insert(result, inner)
            end
        else
            table.insert(result, v)
        end
    end
    return result
end

-- Regroupe les éléments par la clé retournée par fn
-- Table.groupBy({"a","bb","cc","d"}, function(s) return #s end)
-- → { [1]={"a","d"}, [2]={"bb","cc"} }
function Table.groupBy(t, fn)
    local groups = {}
    for _, v in ipairs(t) do
        local key = fn(v)
        if not groups[key] then groups[key] = {} end
        table.insert(groups[key], v)
    end
    return groups
end

-- Copie profonde (shallow pour les valeurs non-table)
function Table.copy(t)
    local copy = {}
    for k, v in pairs(t) do
        copy[k] = (type(v) == "table") and Table.copy(v) or v
    end
    return copy
end

return Table
