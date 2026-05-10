-- nova/persist.lua
-- Données persistantes — survit aux changements de scène
-- Sauvegarde automatique dans data/saves/persist.lua
-- Disponible sans require : Persist est un global auto-chargé.
--
-- API :
--   Persist.set(key, value)              -- définit (types: string, number, boolean, table)
--   Persist.setBatch(tbl)                -- définit plusieurs clés en une seule écriture
--   Persist.get(key, default)            -- lit (default si absent)
--   Persist.delete(key)                  -- supprime une clé
--   Persist.deleteBatch(keys)            -- supprime plusieurs clés en une seule écriture
--   Persist.getAll()                     -- retourne une copie de toutes les données
--   Persist.clear()                      -- efface tout
--   Persist.save()                       -- force l'écriture sur disque
--   Persist.load()                       -- force le rechargement depuis le disque
--
-- Versioning et migration :
--   Persist.setVersion(v)                -- déclare la version courante du schéma (défaut : 1)
--   Persist.onMigrate(fromV, toV, fn)    -- enregistre une migration fn(data) → data
--
-- La migration s'exécute automatiquement lors de Persist.load() si la version
-- sauvegardée est inférieure à la version courante. Les migrations sont
-- appliquées dans l'ordre (de fromV+1 jusqu'à toV).
--
-- Exemple de migration :
--   Persist.setVersion(2)
--   Persist.onMigrate(1, 2, function(data)
--       data["health"] = data["old_hp"]  -- renommage de clé
--       data["old_hp"] = nil
--       return data
--   end)
--
-- Exemple :
--   Persist.set("playerGold",  100)
--   Persist.set("bossDefeated", true)
--   Persist.set("playerName", "Héros")
--
--   local gold = Persist.get("playerGold",  0)
--   local name = Persist.get("playerName", "Inconnu")

local Persist = {}
local data      = {}
local SAVE_PATH = "data/saves/persist.lua"

-- Clé de version interne — jamais exposée dans getAll()
local VERSION_KEY = "__persist_version"
local _version    = 1                   -- version courante du schéma
local _migrations = {}                  -- _migrations[toVersion] = fn(data) → data

-- ─── Sérialisation ────────────────────────────────────────────────────────────

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

-- ─── Versioning / Migration ───────────────────────────────────────────────────

--- Déclare la version courante du schéma. À appeler avant Persist.load().
function Persist.setVersion(v)
    _version = v
end

--- Enregistre une migration de fromVersion vers toVersion.
--- fn(data) reçoit la table de données brute et doit la retourner (ou nil pour no-op).
function Persist.onMigrate(fromVersion, toVersion, fn)
    _migrations[toVersion] = fn
end

local function _runMigrations(raw, savedVersion)
    for v = savedVersion + 1, _version do
        local fn = _migrations[v]
        if fn then
            local ok, result = pcall(fn, raw)
            if ok then
                raw = result or raw
                Log.info(string.format("[Persist] Migration → v%d OK", v))
            else
                Log.error(string.format("[Persist] Migration → v%d FAIL : %s", v, tostring(result)))
            end
        else
            Log.warn(string.format("[Persist] Aucune migration enregistrée pour v%d", v))
        end
    end
    return raw
end

-- ─── Sauvegarde / Chargement ──────────────────────────────────────────────────

function Persist.save()
    local ok, err = pcall(function()
        local f = io.open(SAVE_PATH, "w")
        if not f then
            Log.warn("[Persist] impossible d'ouvrir " .. SAVE_PATH)
            return
        end
        -- Inclure la version dans le snapshot (jamais dans `data` lui-même)
        local snapshot = {}
        for k, v in pairs(data) do snapshot[k] = v end
        snapshot[VERSION_KEY] = _version
        f:write("return " .. serialize(snapshot) .. "\n")
        f:close()
    end)
    if not ok then Log.error("[Persist] save : " .. tostring(err)) end
end

function Persist.load()
    local ok, err = pcall(function()
        local f = io.open(SAVE_PATH, "r")
        if not f then return end   -- pas encore de sauvegarde
        local content = f:read("*a")
        f:close()
        local chunk, loadErr = load(content)
        if not chunk then
            Log.warn("[Persist] load : " .. tostring(loadErr))
            return
        end
        local loaded     = chunk() or {}
        local savedVer   = loaded[VERSION_KEY] or 0
        loaded[VERSION_KEY] = nil   -- retirer le marqueur avant migration / usage

        if savedVer < _version then
            loaded = _runMigrations(loaded, savedVer)
            data   = loaded
            Persist.save()   -- réécrire avec la nouvelle version
        else
            data = loaded
        end
    end)
    if not ok then Log.error("[Persist] load : " .. tostring(err)) end
end

-- ─── API publique ─────────────────────────────────────────────────────────────

function Persist.set(key, value)
    data[key] = value
    Persist.save()
end

--- Définit plusieurs clés en une seule écriture sur disque.
function Persist.setBatch(tbl)
    for k, v in pairs(tbl) do data[k] = v end
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

--- Supprime plusieurs clés en une seule écriture sur disque.
function Persist.deleteBatch(keys)
    for _, k in ipairs(keys) do data[k] = nil end
    Persist.save()
end

function Persist.clear()
    data = {}
    Persist.save()
end

--- Retourne une copie de toutes les données (sans la clé de version interne).
function Persist.getAll()
    local copy = {}
    for k, v in pairs(data) do copy[k] = v end
    return copy
end

-- Auto-chargement au premier require
Persist.load()

return Persist
