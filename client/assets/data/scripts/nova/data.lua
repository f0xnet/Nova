-- nova/data.lua
-- Chargement et accès aux fichiers de données JSON du jeu.
-- Disponible sans require : Data est un global auto-chargé.
--
-- Sépare les données de jeu (items, stats, dialogues…) du code.
-- Les fichiers JSON sont chargés une seule fois et mis en cache.
--
-- API :
--   Data.load(id, path)            -- charge un fichier JSON et le met en cache
--   Data.get(id, key)              -- accède à une entrée (clé optionnelle)
--   Data.set(id, key, value)       -- modifie une valeur en mémoire (pas sur disque)
--   Data.reload(id)                -- recharge le fichier depuis le disque
--   Data.unload(id)                -- libère le cache
--   Data.isLoaded(id)              -- vrai si l'id est en cache
--   Data.getAll(id)                -- retourne la table entière
--   Data.query(id, fn)             -- filtre les entrées d'une liste : fn(entry) → bool
--
-- Utilise Resources.loadJSON(path) (binding C++, nlohmann::json → table Lua).
--
-- Exemple :
--   -- Chargement au démarrage
--   Data.load("items",    "data/definitions/items.json")
--   Data.load("enemies",  "data/definitions/enemies.json")
--   Data.load("dialogue", "data/definitions/dialogue.json")
--
--   -- Lecture
--   local sword  = Data.get("items", "sword_iron")
--   local damage = sword.damage  -- 15
--
--   -- Requête filtrée
--   local rares = Data.query("items", function(item)
--       return item.rarity == "rare"
--   end)
--
--   -- Définitions PNJ
--   local goblinDef = Data.get("enemies", "goblin")
--   Stats.init(entity.id, goblinDef.stats)

local Data = {}

local _cache = {}     -- id → table chargée
local _paths = {}     -- id → path (pour reload)

function Data.load(id, path)
    local t = Resources.loadJSON(path)
    if t == nil then
        Log.warn("[Data.load] Echec chargement '" .. tostring(path) .. "' (id=" .. id .. ")")
        return false
    end
    _cache[id] = t
    _paths[id] = path
    return true
end

function Data.reload(id)
    local path = _paths[id]
    if not path then
        Log.warn("[Data.reload] id '" .. id .. "' jamais chargé, rien à recharger")
        return false
    end
    return Data.load(id, path)
end

function Data.unload(id)
    _cache[id] = nil
    _paths[id] = nil
end

function Data.isLoaded(id)
    return _cache[id] ~= nil
end

function Data.getAll(id)
    return _cache[id]
end

-- Accès direct à un sous-élément ; key peut être nil (retourne la table entière)
function Data.get(id, key)
    local t = _cache[id]
    if t == nil then
        Log.warn("[Data.get] id '" .. id .. "' non chargé")
        return nil
    end
    if key == nil then return t end
    return t[key]
end

-- Modifie une valeur en mémoire (ne touche pas au fichier)
function Data.set(id, key, value)
    if not _cache[id] then
        Log.warn("[Data.set] id '" .. id .. "' non chargé")
        return
    end
    _cache[id][key] = value
end

-- Filtre une table-liste : retourne les entrées où fn(entry) == true
-- Fonctionne aussi sur les dicts (itère les valeurs)
function Data.query(id, fn)
    local t = _cache[id]
    if not t then return {} end
    local result = {}
    for k, v in pairs(t) do
        if type(v) == "table" and fn(v) then
            result[#result + 1] = v
        end
    end
    return result
end

return Data
