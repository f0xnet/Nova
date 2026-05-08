-- nova/i18n.lua
-- Module de localisation / traduction.
-- Disponible sans require : I18n est un global auto-chargé.
--
-- Les fichiers de traduction sont des modules Lua retournant une table :
--   data/i18n/fr.lua → return { ["ui.start"] = "Commencer", ["npc.greet"] = "Bonjour !" }
--
-- API :
--   I18n.load(lang, modulePath)   -- charge les traductions via require(modulePath), met en cache
--   I18n.loadTable(lang, t)       -- charge depuis une table Lua directement (sans fichier)
--   I18n.set(lang)                -- change la langue active (doit être chargée au préalable)
--   I18n.get()                    -- retourne le code de la langue active
--   I18n.t(key, ...)              -- traduit une clé ; repli sur la clé si absente ;
--                                 --   les arguments supplémentaires sont passés à string.format
--   I18n.has(key)                 -- true si la clé existe dans la langue active
--   I18n.fallback(lang)           -- définit une langue de repli quand la clé est absente
--
-- Événements émis via EventBus :
--   "language_changed"  { lang }  -- après chaque appel à I18n.set()
--
-- Exemple :
--   I18n.load("fr", "data/i18n/fr")
--   I18n.load("en", "data/i18n/en")
--   I18n.fallback("en")
--   I18n.set("fr")
--
--   print(I18n.t("ui.start"))              -- "Commencer"
--   print(I18n.t("ui.score", 42))          -- string.format(translations["ui.score"], 42)
--   print(I18n.t("ui.missing_key"))        -- "ui.missing_key"  (repli sur la clé)
--
--   EventBus.on("language_changed", function(data)
--       print("Nouvelle langue : " .. data.lang)
--   end)

local I18n = {}

local _translations  = {}   -- { [lang] = { [key] = string } }
local _currentLang   = nil
local _fallbackLang  = nil

-- Charge un module Lua et met les traductions en cache.
function I18n.load(lang, modulePath)
    if type(lang) ~= "string" or type(modulePath) ~= "string" then
        Log.warn("[I18n] load: lang et modulePath doivent être des chaînes")
        return
    end
    local ok, result = pcall(require, modulePath)
    if not ok then
        Log.error("[I18n] load: impossible de charger '" .. modulePath .. "' : " .. tostring(result))
        return
    end
    if type(result) ~= "table" then
        Log.warn("[I18n] load: '" .. modulePath .. "' ne retourne pas une table")
        return
    end
    _translations[lang] = result
end

-- Charge les traductions depuis une table Lua existante (pas de fichier requis).
function I18n.loadTable(lang, t)
    if type(lang) ~= "string" then
        Log.warn("[I18n] loadTable: lang doit être une chaîne")
        return
    end
    if type(t) ~= "table" then
        Log.warn("[I18n] loadTable: t doit être une table")
        return
    end
    _translations[lang] = t
end

-- Change la langue active. La langue doit avoir été chargée au préalable.
function I18n.set(lang)
    if type(lang) ~= "string" then
        Log.warn("[I18n] set: lang doit être une chaîne")
        return
    end
    if not _translations[lang] then
        Log.warn("[I18n] set: la langue '" .. lang .. "' n'a pas été chargée")
        return
    end
    _currentLang = lang
    EventBus.emit("language_changed", { lang = lang })
end

-- Retourne le code de la langue active.
function I18n.get()
    return _currentLang
end

-- Définit la langue de repli utilisée quand une clé est absente de la langue active.
function I18n.fallback(lang)
    if type(lang) ~= "string" then
        Log.warn("[I18n] fallback: lang doit être une chaîne")
        return
    end
    _fallbackLang = lang
end

-- Traduit une clé dans la langue active.
-- Si la clé est absente, tente la langue de repli, puis retourne la clé brute.
-- Les arguments supplémentaires sont passés à string.format sur la valeur trouvée.
function I18n.t(key, ...)
    local value

    -- Cherche dans la langue active
    if _currentLang and _translations[_currentLang] then
        value = _translations[_currentLang][key]
    end

    -- Tente la langue de repli
    if value == nil and _fallbackLang and _translations[_fallbackLang] then
        value = _translations[_fallbackLang][key]
    end

    -- Repli final sur la clé brute
    if value == nil then
        return key
    end

    -- Formatage si des arguments supplémentaires sont fournis
    local args = { ... }
    if #args > 0 then
        local ok, result = pcall(string.format, value, ...)
        if ok then
            return result
        else
            Log.warn("[I18n] t: erreur de formatage pour la clé '" .. key .. "' : " .. tostring(result))
            return value
        end
    end

    return value
end

-- Retourne true si la clé existe dans la langue active.
function I18n.has(key)
    if not _currentLang or not _translations[_currentLang] then
        return false
    end
    return _translations[_currentLang][key] ~= nil
end

return I18n
