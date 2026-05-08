-- nova/anim.lua
-- Contrôleur d'animation haut niveau sur AnimationComponent C++.
-- Disponible sans require : Anim est un global auto-chargé.
--
-- API :
--   Anim.play(entity, id, loop)      -- joue une animation (loop = true par défaut)
--   Anim.stop(entity)                -- arrête l'animation en cours
--   Anim.setSpeed(entity, speed)     -- vitesse de lecture (1.0 = normale)
--   Anim.current(entity)             -- nom de l'animation courante (animationID)
--   Anim.onFrame(entity, frame, fn)  -- callback au frame N (one-shot)
--   Anim.onFinish(entity, fn)        -- callback quand une animation non-loop se termine
--   Anim.clearCallbacks(entity)      -- supprime tous les callbacks de l'entité
--   Anim._update(dt)                 -- appelé par ScriptSystem chaque frame
--
-- Note : Anim.play() change animationID, loop, currentFrame et playing sur l'AnimationComponent.
-- Les frames (rectangles sprite) doivent déjà être configurées dans le composant.
--
-- Exemple :
--   Anim.play(entity, "run", true)
--
--   Anim.onFinish(entity, function()
--       Anim.play(entity, "idle", true)
--   end)
--
--   Anim.onFrame(entity, 5, function()
--       Sound.play("sounds/sword_swing.ogg")
--   end)

local Anim = {}

local _callbacks    = {}  -- entityId → { onFinish=fn|nil, onFrame={ [frame]=fn, ... } }
local _prevFrame    = {}  -- entityId → last seen frame index
local _prevPlaying  = {}  -- entityId → last seen playing state (for onFinish detection)
local _baseFrameDur = {}  -- entityId → original frameDuration (for setSpeed)

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

local function getEntity(v)
    if type(v) == "userdata" then return v end
    return Registry:getEntity(v)
end

local function getAnimComp(entity)
    local ent = getEntity(entity)
    return ent and ent:getAnimation()
end

local function ensureCb(entityId)
    if not _callbacks[entityId] then
        _callbacks[entityId] = { onFinish = nil, onFrame = {} }
    end
    return _callbacks[entityId]
end

function Anim.play(entity, id, loop)
    local comp = getAnimComp(entity)
    if not comp then return end
    if loop == nil then loop = true end
    comp.animationID  = id
    comp.loop         = loop
    comp.playing      = true
    comp.currentFrame = 0
    -- Reset prevPlaying so we don't fire onFinish for the previous animation
    local eid = toId(entity)
    _prevPlaying[eid] = true
end

function Anim.stop(entity)
    local comp = getAnimComp(entity)
    if comp then comp.playing = false end
end

function Anim.setSpeed(entity, speed)
    local eid  = toId(entity)
    local comp = getAnimComp(entity)
    if not comp then return end
    -- Store the original frameDuration the first time
    if not _baseFrameDur[eid] then
        _baseFrameDur[eid] = comp.frameDuration
    end
    comp.frameDuration = _baseFrameDur[eid] / math.max(speed, 0.001)
end

function Anim.current(entity)
    local comp = getAnimComp(entity)
    return comp and comp.animationID or nil
end

function Anim.onFrame(entity, frame, fn)
    local eid = toId(entity)
    local cb  = ensureCb(eid)
    cb.onFrame[frame] = fn
end

function Anim.onFinish(entity, fn)
    local eid = toId(entity)
    local cb  = ensureCb(eid)
    cb.onFinish = fn
end

function Anim.clearCallbacks(entity)
    local eid = toId(entity)
    _callbacks[eid]    = nil
    _prevFrame[eid]    = nil
    _prevPlaying[eid]  = nil
    _baseFrameDur[eid] = nil
end

function Anim._update(dt)
    for entityId, cb in pairs(_callbacks) do
        local ent = Registry:getEntity(entityId)
        if not ent then
            -- Entité détruite — nettoyage
            _callbacks[entityId]    = nil
            _prevFrame[entityId]    = nil
            _prevPlaying[entityId]  = nil
            _baseFrameDur[entityId] = nil
        else
            local comp = ent:getAnimation()
            if comp then
                local frame   = comp.currentFrame
                local playing = comp.playing

                -- Callbacks onFrame (one-shot par frame)
                local prevF = _prevFrame[entityId] or -1
                if frame ~= prevF then
                    local fn = cb.onFrame[frame]
                    if fn then
                        cb.onFrame[frame] = nil  -- one-shot
                        fn()
                    end
                    _prevFrame[entityId] = frame
                end

                -- Callback onFinish : animation non-loop passée à playing=false
                local prevP = _prevPlaying[entityId]
                if prevP == true and not playing and not comp.loop then
                    if cb.onFinish then
                        local fn = cb.onFinish
                        cb.onFinish = nil  -- one-shot
                        fn()
                    end
                end
                _prevPlaying[entityId] = playing
            end
        end
    end
end

return Anim
