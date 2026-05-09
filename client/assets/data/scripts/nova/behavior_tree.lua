-- nova/behavior_tree.lua
-- Behavior Trees pour l'IA des entités.
-- Disponible sans require : BT est un global auto-chargé.
--
-- API :
--   BT.action(fn)           -- feuille : fn(entity, dt) → SUCCESS|FAILURE|RUNNING
--   BT.condition(fn)        -- feuille : fn(entity, dt) → SUCCESS si truthy, FAILURE sinon
--   BT.sequence(...)        -- tous les enfants doivent réussir (AND)
--   BT.selector(...)        -- le premier qui réussit suffit (OR)
--   BT.parallel(...)        -- tous les enfants s'exécutent ; FAILURE si l'un échoue
--   BT.invert(child)        -- déco : SUCCESS↔FAILURE, RUNNING inchangé
--   BT.succeed(child)       -- déco : toujours SUCCESS (ignore le résultat)
--   BT.fail(child)          -- déco : toujours FAILURE (ignore le résultat)
--   BT.loop(child, n)       -- déco : répète n fois (-1 = infini)
--   BT.run(tree, entity, dt) -- exécute l'arbre, retourne le statut
--
-- Statuts :
--   BT.SUCCESS  (1)   l'action s'est terminée avec succès
--   BT.FAILURE  (0)   l'action a échoué
--   BT.RUNNING  (2)   l'action est en cours (continue au prochain frame)
--
-- Notes :
--   • Les nœuds sont des fonctions pures (closures) — créez l'arbre dans init()
--     pour que chaque entité possède sa propre instance (important pour BT.loop).
--   • Sequence et Selector sont « stateless » : si un enfant retourne RUNNING,
--     la traversée reprend depuis le début au prochain tick. Les conditions sont
--     donc ré-évaluées chaque frame, ce qui est le comportement standard.
--   • L'état multi-frame (ex. « je marche vers X ») doit être stocké dans
--     l'entité ou via StateMachine, pas dans l'arbre lui-même.
--
-- Exemple :
--   local tree
--   function init(entity)
--       tree = BT.selector(
--           BT.sequence(
--               BT.condition(function(e) return canSeePlayer(e) end),
--               BT.action(function(e, dt) return chase(e, dt) end)
--           ),
--           BT.action(function(e, dt) return patrol(e, dt) end)
--       )
--   end
--
--   function update(entity, dt)
--       BT.run(tree, entity, dt)
--   end

local BT = {}

BT.SUCCESS = 1
BT.FAILURE = 0
BT.RUNNING  = 2

-- ─── Feuilles ─────────────────────────────────────────────────────────────────

-- Enveloppe une fonction d'action.
-- fn(entity, dt) doit retourner BT.SUCCESS, BT.FAILURE ou BT.RUNNING.
function BT.action(fn)
    return fn
end

-- Enveloppe une fonction booléenne en nœud condition.
-- Retourne SUCCESS si fn() est truthy, FAILURE sinon.
function BT.condition(fn)
    return function(entity, dt)
        return fn(entity, dt) and BT.SUCCESS or BT.FAILURE
    end
end

-- ─── Composites ───────────────────────────────────────────────────────────────

-- Exécute les enfants en ordre. S'arrête au premier FAILURE ou RUNNING.
-- Retourne SUCCESS seulement si tous les enfants réussissent.
function BT.sequence(...)
    local children = { ... }
    return function(entity, dt)
        for _, child in ipairs(children) do
            local s = child(entity, dt)
            if s ~= BT.SUCCESS then return s end
        end
        return BT.SUCCESS
    end
end

-- Exécute les enfants en ordre. S'arrête au premier SUCCESS ou RUNNING.
-- Retourne FAILURE seulement si tous les enfants échouent.
function BT.selector(...)
    local children = { ... }
    return function(entity, dt)
        for _, child in ipairs(children) do
            local s = child(entity, dt)
            if s ~= BT.FAILURE then return s end
        end
        return BT.FAILURE
    end
end

-- Exécute TOUS les enfants chaque tick.
-- Retourne FAILURE si l'un échoue, RUNNING si l'un est en cours, SUCCESS sinon.
function BT.parallel(...)
    local children = { ... }
    return function(entity, dt)
        local anyRunning = false
        for _, child in ipairs(children) do
            local s = child(entity, dt)
            if s == BT.FAILURE then return BT.FAILURE end
            if s == BT.RUNNING  then anyRunning = true end
        end
        return anyRunning and BT.RUNNING or BT.SUCCESS
    end
end

-- ─── Décorateurs ──────────────────────────────────────────────────────────────

-- Inverse SUCCESS et FAILURE. RUNNING est transmis tel quel.
function BT.invert(child)
    return function(entity, dt)
        local s = child(entity, dt)
        if s == BT.SUCCESS then return BT.FAILURE
        elseif s == BT.FAILURE then return BT.SUCCESS
        else return s end
    end
end

-- Toujours SUCCESS, quel que soit le résultat de l'enfant.
function BT.succeed(child)
    return function(entity, dt)
        child(entity, dt)
        return BT.SUCCESS
    end
end

-- Toujours FAILURE, quel que soit le résultat de l'enfant.
function BT.fail(child)
    return function(entity, dt)
        child(entity, dt)
        return BT.FAILURE
    end
end

-- Répète l'enfant n fois (n = -1 : boucle infinie).
-- Retourne FAILURE si l'enfant échoue, RUNNING tant que la répétition continue,
-- SUCCESS quand le compteur est atteint.
-- Crée l'arbre dans init() : le compteur est par-instance (closure).
function BT.loop(child, n)
    local count = 0
    return function(entity, dt)
        if n >= 0 and count >= n then return BT.SUCCESS end
        local s = child(entity, dt)
        if s == BT.FAILURE then return BT.FAILURE end
        if s == BT.SUCCESS then
            count = count + 1
            if n >= 0 and count >= n then return BT.SUCCESS end
        end
        return BT.RUNNING
    end
end

-- ─── Exécution ────────────────────────────────────────────────────────────────

-- Point d'entrée pour exécuter un arbre depuis update().
function BT.run(tree, entity, dt)
    return tree(entity, dt)
end

return BT
