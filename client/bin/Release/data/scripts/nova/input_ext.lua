-- nova/input_ext.lua
-- Extension de l'Input pour détecter "vient d'être pressé/relâché ce frame"
-- Complément à Input.isKeyPressed (état maintenu) et aux OnKeyDown named handlers.
-- Disponible sans require : InputEx est un global auto-chargé.
--
-- API :
--   InputEx.isKeyJustPressed(key)    -- vrai UNE SEULE FRAME quand la touche s'enfonce
--   InputEx.isKeyJustReleased(key)   -- vrai UNE SEULE FRAME quand la touche se relâche
--   InputEx.isMouseJustPressed(btn)  -- pareil pour la souris ("left","right","middle")
--   InputEx.isMouseJustReleased(btn)
--   InputEx.getJustPressedKeys()     -- table des touches enfoncées ce frame
--
-- DIFFÉRENCES avec les autres APIs :
--   Input.isKeyPressed("E")      → vrai TANT QUE la touche est tenue
--   InputEx.isKeyJustPressed("E") → vrai UNIQUEMENT le premier frame de l'appui
--   EventBus.on("key_down_E")    → callback asynchrone (sans accès au polling)
--   function OnKeyDown(key)      → named handler automatique (même chose)
--
-- Exemple :
--   function update(entity, dt)
--       -- Attaque uniquement au moment du clic, pas chaque frame
--       if InputEx.isKeyJustPressed("Space") then
--           attackNearby()
--       end
--   end

local InputEx = {}

local justPressed   = {}
local justReleased  = {}
local mousePressed  = {}
local mouseReleased = {}

-- Abonnements aux événements génériques émis par ScriptSystem
EventBus.on("key_down", function(data)
    justPressed[data.key] = true
end)

EventBus.on("key_up", function(data)
    justReleased[data.key] = true
end)

EventBus.on("mouse_down", function(data)
    mousePressed[data.button] = true
end)

EventBus.on("mouse_up", function(data)
    mouseReleased[data.button] = true
end)

-- Appelé par ScriptSystem au DÉBUT de chaque frame (avant updateInputBridge)
function InputEx._clear()
    justPressed   = {}
    justReleased  = {}
    mousePressed  = {}
    mouseReleased = {}
end

function InputEx.isKeyJustPressed(key)
    return justPressed[key] == true
end

function InputEx.isKeyJustReleased(key)
    return justReleased[key] == true
end

function InputEx.isMouseJustPressed(btn)
    return mousePressed[btn] == true
end

function InputEx.isMouseJustReleased(btn)
    return mouseReleased[btn] == true
end

-- Retourne la liste des touches enfoncées ce frame (utile pour debug)
function InputEx.getJustPressedKeys()
    local list = {}
    for k in pairs(justPressed) do table.insert(list, k) end
    return list
end

-- Étend le global Input (C++) avec les helpers "just" pour une API unifiée.
-- Après ce module : Input.isKeyJustPressed("E") fonctionne comme InputEx.isKeyJustPressed("E")
if Input then
    Input.isKeyJustPressed    = function(k)   return justPressed[k]   == true end
    Input.isKeyJustReleased   = function(k)   return justReleased[k]  == true end
    Input.isMouseJustPressed  = function(btn) return mousePressed[btn] == true end
    Input.isMouseJustReleased = function(btn) return mouseReleased[btn]== true end
    Input.getJustPressedKeys  = InputEx.getJustPressedKeys
end

return InputEx
