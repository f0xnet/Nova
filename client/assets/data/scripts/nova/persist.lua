-- nova/persist.lua
-- Données persistantes — survit aux changements de scène
-- Sauvegarde automatique dans data/saves/persist.lua
-- Disponible sans require : Persist est un global auto-chargé.
--
-- API :
--   Persist.set(key, value)         -- définit (types: string, number, boolean)
--   Persist.get(key, default)       -- lit (default si absent)
--   Persist.delete(key)             -- supprime une clé
--   Persist.clear()                 -- efface tout
--   Persist.getAll()                -- retourne une copie de toutes les données
--   Persist.save()                  -- force l'écriture sur disque
--   Persist.load()                  -- force le rechargement depuis le disque
--
-- Exemple :
--   Persist.set("playerGold",  100)
--   Persist.set("bossDefeated", true)
--   Persist.set("playerName", "Héros")
--
--   local gold = Persist.get("playerGold",  0)
--   local name = Persist.get("playerName", "Inconnu")

local Persist = {}
local data     = {}
local SAVE_PATH = "data/saves/persist.lua"

-- Sérialise une valeur en code Lua (string, number, boolean, table flat)
local function serialize(val, indent)
    indent = indent or ""
    local t = type(val)
    if t == "string" then
        return string.format("%q", val)
    elseif t == "number" or t == "boolean" then
        return tostring(val)
    elseif t == "table" then
        local parts = {}
        local ni    = indent .. "  "
        for k, v in pairs(val) do
            local key = (type(k) == "string")
                and ("[" .. string.format("%q", k) .. "]")
                or  ("[" .. tostring(k) .. "]")
            table.insert(parts, ni .. key .. " = " .. serialize(v, ni))
        end
        if #parts == 0 then return "{}" end
        return "{\n" .. table.concat(parts, ",\n") .. "\n" .. indent .. "}"
    end
    return "nil"
end

function Persist.save()
    local ok, err = pcall(function()
        local f = io.open(SAVE_PATH, "w")
        if not f then
            Log.warn("Persist.save: impossible d'ouvrir " .. SAVE_PATH)
            return
        end
        f:write("return " .. serialize(data) .. "\n")
        f:close()
    end)
    if not ok then Log.error("Persist.save: " .. tostring(err)) end
end

function Persist.load()
    local ok, err = pcall(function()
        local f = io.open(SAVE_PATH, "r")
        if not f then return end   -- pas encore de sauvegarde
        local content = f:read("*a")
        f:close()
        local chunk, loadErr = load(content)
        if chunk then
            data = chunk() or {}
        else
            Log.warn("Persist.load: " .. tostring(loadErr))
        end
    end)
    if not ok then Log.error("Persist.load: " .. tostring(err)) end
end

function Persist.set(key, value)
    data[key] = value
    Persist.save()
end

function Persist.get(key, default)
    local v = data[key]
    return (v ~= nil) and v or default
end

function Persist.delete(key)
    data[key] = nil
    Persist.save()
end

function Persist.clear()
    data = {}
    Persist.save()
end

function Persist.getAll()
    local copy = {}
    for k, v in pairs(data) do copy[k] = v end
    return copy
end

-- Auto-chargement au premier require
Persist.load()

return Persist
