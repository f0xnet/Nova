-- nova/stat_names.lua
-- Constantes canoniques des noms de stats — évite les typos silencieuses.
-- Disponible sans require : Stat est un global auto-chargé.
--
-- Utilisation :
--   Stats.set(entity.id, Stat.Health, 100)    -- au lieu de Stats.set(entity.id, "Health", 100)
--   Stats.get(entity.id, Stat.Mana, 0)
--   Stats.mod(entity.id, Stat.Stamina, -10)
--
-- Ajouter de nouvelles stats projet :
--   Stat.MyCustomStat = "MyCustomStat"
--
-- Les valeurs sont identiques aux clés (string) pour rester compatibles avec
-- toute API qui compare les noms : EventBus stat_changed, Debug.watch, etc.

local Stat = {}

-- Stats de base RPG
Stat.Health  = "Health"
Stat.Mana    = "Mana"
Stat.Stamina = "Stamina"
Stat.Shield  = "Shield"

-- Progression
Stat.Level = "Level"
Stat.Exp   = "Exp"

-- Combat
Stat.Attack  = "Attack"
Stat.Defense = "Defense"
Stat.Speed   = "Speed"
Stat.Armor   = "Armor"
Stat.CritChance = "CritChance"

-- Résistances
Stat.FireRes    = "FireRes"
Stat.IceRes     = "IceRes"
Stat.PoisonRes  = "PoisonRes"

return Stat
