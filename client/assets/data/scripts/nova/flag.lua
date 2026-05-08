-- nova/flag.lua
-- Drapeaux booléens nommés — équivalent des GlobalVariables de Papyrus.
-- Utilisés pour les états de jeu persistants simples qui ne nécessitent
-- ni une stat ni une quête entière.
-- Disponible sans require : Flag est un global auto-chargé.
--
-- API :
--   Flag.set(name)        -- active le drapeau
--   Flag.unset(name)      -- désactive le drapeau
--   Flag.toggle(name)     -- inverse l'état
--   Flag.get(name)        -- retourne true si actif, false sinon
--   Flag.clear(name)      -- alias de unset
--   Flag.clearAll()       -- réinitialise tous les drapeaux
--   Flag.getAll()         -- retourne une copie { name = true, ... }
--   Flag.require(name)    -- alias get — lecture sémantique ("nécessite le flag X")
--
-- EventBus events émis :
--   "flag_set"      { name }
--   "flag_unset"    { name }
--   "flags_cleared" {}
--
-- Exemple :
--   Flag.set("boss_defeated")
--   Flag.set("bridge_repaired")
--
--   if Flag.get("boss_defeated") and Flag.get("bridge_repaired") then
--       Scene.transition("city_liberated")
--   end
--
--   -- Named handler qui réagit aux quêtes complétées
--   function OnQuestComplete(questId)
--       if questId == "main_quest" then
--           Flag.set("credits_unlocked")
--       end
--   end
--
--   -- Porte qui se souvient d'avoir été ouverte
--   function OnActivate()
--       if not Flag.get("throne_door_open") then
--           Flag.set("throne_door_open")
--           entity:enable()
--       end
--   end

local Flag = {}

local _flags = {}

function Flag.set(name)
    if not _flags[name] then
        _flags[name] = true
        EventBus.emit("flag_set", { name = name })
    end
end

function Flag.unset(name)
    if _flags[name] then
        _flags[name] = nil
        EventBus.emit("flag_unset", { name = name })
    end
end

function Flag.toggle(name)
    if _flags[name] then Flag.unset(name) else Flag.set(name) end
end

function Flag.get(name)
    return _flags[name] == true
end

function Flag.clearAll()
    _flags = {}
    EventBus.emit("flags_cleared", {})
end

function Flag.getAll()
    local copy = {}
    for k in pairs(_flags) do copy[k] = true end
    return copy
end

-- Alias sémantiques
Flag.clear   = Flag.unset
Flag.require = Flag.get

return Flag
