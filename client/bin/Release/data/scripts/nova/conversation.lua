-- nova/conversation.lua
-- Arbre de dialogue avec nœuds, choix et conditions.
-- Disponible sans require : Conversation est un global auto-chargé.
--
-- API :
--   Conversation.define(id, tree)          -- enregistre un arbre de dialogue
--   Conversation.start(id, ctx)            -- démarre la conversation
--   Conversation.choose(index)             -- sélectionne le choix courant
--   Conversation.advance()                 -- avance au nœud suivant (sans choix)
--   Conversation.stop()                    -- termine la conversation en cours
--   Conversation.getCurrent()              -- retourne { node, choices } ou nil
--   Conversation.isRunning()               -- true si une conversation est active
--
-- Format d'un arbre (tree) :
--   {
--     start = "intro",
--     nodes = {
--       intro = {
--         speaker = "guard",
--         text    = "Halt! Who goes there?",
--         onEnter = function(ctx) end,        -- optionnel
--         choices = {
--           { text = "A friend.",  next = "friend",  cond = function(ctx) return true end },
--           { text = "None of your business.", next = "hostile" },
--         }
--         -- OU: next = "nextNodeId"  (pas de choix)
--       },
--       friend = {
--         speaker = "guard",
--         text    = "Pass, friend.",
--         next    = nil,   -- nil = fin
--       },
--       ...
--     }
--   }
--
-- EventBus events émis :
--   "conversation_node"   { id, node, speaker, text }
--   "conversation_choice" { index, text }
--   "conversation_chosen" { index, text, next }
--   "conversation_end"    { id }
--
-- Exemple :
--   Conversation.define("gate_guard", {
--       start = "greet",
--       nodes = {
--           greet = {
--               speaker = "guard",
--               text    = "State your name.",
--               choices = {
--                   { text = "Aiden.", next = "ok" },
--                   { text = "(Attack)", next = "fight",
--                     cond = function() return Flag.get("player_armed") end },
--               }
--           },
--           ok    = { speaker = "guard", text = "Go ahead.", next = nil },
--           fight = { speaker = "guard", text = "Guards!", next = nil,
--                     onEnter = function() Flag.set("combat_triggered") end },
--       }
--   })
--
--   Conversation.start("gate_guard", { player = player })

local Conversation = {}

local _trees   = {}    -- id → tree
local _session = nil   -- session active

local function _enterNode(nodeId)
    local tree = _trees[_session.treeId]
    local node = tree.nodes[nodeId]
    if not node then
        Conversation.stop()
        return
    end

    _session.currentNodeId = nodeId

    if node.onEnter then node.onEnter(_session.ctx) end

    -- Filtre les choix selon les conditions
    local visible = {}
    if node.choices then
        for i, choice in ipairs(node.choices) do
            if not choice.cond or choice.cond(_session.ctx) then
                visible[#visible + 1] = { index = i, text = choice.text, next = choice.next }
            end
        end
    end
    _session.visibleChoices = visible

    EventBus.emit("conversation_node", {
        id      = _session.treeId,
        nodeId  = nodeId,
        speaker = node.speaker,
        text    = node.text,
    })

    for _, c in ipairs(visible) do
        EventBus.emit("conversation_choice", { index = c.index, text = c.text })
    end
end

function Conversation.define(id, tree)
    _trees[id] = tree
end

function Conversation.start(id, ctx)
    local tree = _trees[id]
    if not tree then return false end
    if _session then Conversation.stop() end

    _session = {
        treeId         = id,
        ctx            = ctx or {},
        currentNodeId  = nil,
        visibleChoices = {},
    }

    _enterNode(tree.start)
    return true
end

function Conversation.choose(index)
    if not _session then return end
    local node = _trees[_session.treeId].nodes[_session.currentNodeId]
    if not node or not node.choices then return end

    local choice = node.choices[index]
    if not choice then return end

    EventBus.emit("conversation_chosen", { index = index, text = choice.text, next = choice.next })

    if choice.next then
        _enterNode(choice.next)
    else
        Conversation.stop()
    end
end

function Conversation.advance()
    if not _session then return end
    local node = _trees[_session.treeId].nodes[_session.currentNodeId]
    if not node then return end

    if node.next then
        _enterNode(node.next)
    else
        Conversation.stop()
    end
end

function Conversation.stop()
    if not _session then return end
    local id = _session.treeId
    _session = nil
    EventBus.emit("conversation_end", { id = id })
end

function Conversation.getCurrent()
    if not _session then return nil end
    local node = _trees[_session.treeId].nodes[_session.currentNodeId]
    if not node then return nil end
    return {
        node    = node,
        choices = _session.visibleChoices,
        text    = node.text,
        speaker = node.speaker,
    }
end

function Conversation.isRunning()
    return _session ~= nil
end

return Conversation
