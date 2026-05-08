-- nova/input_bind.lua
-- Couche d'indirection pour les contrôles remappables.
-- Disponible sans require : InputBind est un global auto-chargé.
--
-- API :
--   InputBind.register(action, key)        -- lie une action à une touche
--   InputBind.remap(action, key)           -- remplace le binding
--   InputBind.isPressed(action)            -- maintenu enfoncé
--   InputBind.isJustPressed(action)        -- vrai uniquement au premier frame
--   InputBind.isJustReleased(action)       -- vrai uniquement au frame de relâchement
--   InputBind.getKey(action)               -- retourne le nom de la touche liée
--   InputBind.getAll()                     -- { action = key, ... }
--   InputBind.save()                       -- retourne table sérialisable (sauvegarde)
--   InputBind.load(t)                      -- restaure depuis table
--
-- S'appuie sur Input (C++) et Input.isKeyJustPressed / isKeyJustReleased de input_ext.lua.
--
-- Exemple :
--   InputBind.register("attack",    "Space")
--   InputBind.register("interact",  "E")
--   InputBind.register("sprint",    "LShift")
--
--   -- Dans un script d'entité :
--   function OnUpdate(dt)
--       if InputBind.isJustPressed("attack") then
--           entity:doAttack()
--       end
--       if InputBind.isPressed("sprint") then
--           entity.speed = 200
--       end
--   end

local InputBind = {}

local _binds = {}  -- action → key string

function InputBind.register(action, key)
    if not _binds[action] then
        _binds[action] = key
    end
end

function InputBind.remap(action, key)
    _binds[action] = key
    EventBus.emit("input_remapped", { action = action, key = key })
end

function InputBind.getKey(action)
    return _binds[action]
end

function InputBind.isPressed(action)
    local key = _binds[action]
    if not key then return false end
    return Input.isKeyPressed(key)
end

function InputBind.isJustPressed(action)
    local key = _binds[action]
    if not key then return false end
    return Input.isKeyJustPressed(key)
end

function InputBind.isJustReleased(action)
    local key = _binds[action]
    if not key then return false end
    return Input.isKeyJustReleased(key)
end

function InputBind.getAll()
    local copy = {}
    for k, v in pairs(_binds) do copy[k] = v end
    return copy
end

function InputBind.save()
    return InputBind.getAll()
end

function InputBind.load(t)
    for action, key in pairs(t) do
        _binds[action] = key
    end
end

return InputBind
