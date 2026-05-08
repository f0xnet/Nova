-- nova/class.lua
-- Système OOP léger avec héritage prototype pour Lua
-- Disponible sans require : Class est un global auto-chargé.
--
-- API :
--   local MyClass = Class.new()           -- classe sans parent
--   local Child   = Class.new(MyClass)    -- héritage (alias Class.extend)
--   local obj     = MyClass.new(...)      -- instanciation (appelle :init si défini)
--   obj:is(MyClass)                       -- vérifie l'héritage
--
-- Exemple :
--   local Entity = Class.new()
--   function Entity:init(name, hp)
--       self.name = name
--       self.hp   = hp
--   end
--   function Entity:takeDamage(amount)
--       self.hp = self.hp - amount
--       Log.info(self.name .. " a " .. self.hp .. " HP restants")
--   end
--
--   local Goblin = Class.new(Entity)
--   function Goblin:init(name)
--       Entity.init(self, name, 30)    -- appel super
--       self.loot = "pièces"
--   end
--   function Goblin:attack()
--       return self.name .. " attaque !"
--   end
--
--   local g = Goblin.new("Gob")
--   g:takeDamage(10)
--   print(g:attack())
--   print(g:is(Goblin))   -- true
--   print(g:is(Entity))   -- true

local Class = {}

function Class.new(base)
    local cls      = {}
    cls.__index    = cls
    cls.__class    = cls

    if base then
        setmetatable(cls, { __index = base })
        cls.super = base
    end

    -- Constructeur : Class.new(...)
    function cls.new(...)
        local inst = setmetatable({}, cls)
        if inst.init then inst:init(...) end
        return inst
    end

    -- Vérifie si l'instance hérite de klass
    function cls:is(klass)
        local mt = getmetatable(self)
        while mt do
            if mt == klass then return true end
            local next = getmetatable(mt)
            mt = next and next.__index
        end
        return false
    end

    return cls
end

-- Alias sémantique
Class.extend = Class.new

return Class
