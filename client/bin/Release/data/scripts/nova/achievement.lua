-- nova/achievement.lua
-- Achievement system — define, unlock, and persist game achievements.
-- Depends on: EventBus, Persist (auto-loaded globals).
-- Available without require: Achievement is an auto-loaded global.
--
-- API:
--   Achievement.define(id, opts)
--       Declares a new achievement. Idempotent — safe to call multiple times.
--       opts: {
--           name   = string,            -- display name
--           desc   = string,            -- description shown to the player
--           hidden = bool,              -- hide until unlocked (default: false)
--           cond   = function(ctx),     -- optional condition checked externally
--       }
--
--   Achievement.unlock(id)             -- unlocks achievement (no-op if already unlocked)
--   Achievement.isUnlocked(id)         -- returns true/false
--   Achievement.getAll()               -- { [id] = { name, desc, hidden, unlocked, unlockedAt } }
--   Achievement.getUnlocked()          -- sorted list of unlocked ids
--   Achievement.reset()                -- resets all achievements (for testing only)
--   Achievement.save()                 -- persist state via Persist.set("achievements", ...)
--   Achievement.load()                 -- reload state from Persist
--
-- EventBus events emitted:
--   "achievement_unlocked"  { id, name }
--
-- Examples:
--   Achievement.define("first_kill", {
--       name = "First Blood",
--       desc = "Defeat an enemy for the first time.",
--   })
--
--   Achievement.define("secret_room", {
--       name   = "???",
--       desc   = "You found the hidden room.",
--       hidden = true,
--   })
--
--   Achievement.define("pacifist", {
--       name = "Pacifist",
--       desc = "Complete the game without killing anyone.",
--       cond = function(ctx) return ctx.killCount == 0 end,
--   })
--
--   -- Unlock from game logic:
--   function OnEnemyDied()
--       Achievement.unlock("first_kill")
--   end
--
--   -- Query state:
--   if Achievement.isUnlocked("first_kill") then
--       -- show badge ...
--   end
--
--   -- Populate an achievements screen:
--   for id, data in pairs(Achievement.getAll()) do
--       if not data.hidden or data.unlocked then
--           -- render data.name, data.desc, data.unlocked, data.unlockedAt
--       end
--   end
--
--   -- Reset for a test suite:
--   Achievement.reset()

local Achievement = {}

-- ─── Internal state ───────────────────────────────────────────────────────────

-- Definitions registered via Achievement.define()
-- [id] = { name, desc, hidden, cond }
local _defs = {}

-- Runtime unlock state (merged with persisted data on load)
-- [id] = { unlocked = bool, unlockedAt = number|nil }
local _state = {}

-- ─── Persistence ──────────────────────────────────────────────────────────────

local PERSIST_KEY = "achievements"

--- Saves the current unlock state via Persist.
function Achievement.save()
    local snapshot = {}
    for id, s in pairs(_state) do
        snapshot[id] = {
            unlocked   = s.unlocked   or false,
            unlockedAt = s.unlockedAt or nil,
        }
    end
    Persist.set(PERSIST_KEY, snapshot)
end

--- Reloads unlock state from Persist and merges it into _state.
--- Definitions that exist but were not previously saved retain their defaults.
function Achievement.load()
    local saved = Persist.get(PERSIST_KEY, {})
    for id, s in pairs(saved) do
        if not _state[id] then _state[id] = {} end
        _state[id].unlocked   = s.unlocked   or false
        _state[id].unlockedAt = s.unlockedAt or nil
    end
end

-- ─── Public API ───────────────────────────────────────────────────────────────

--- Declares an achievement. Can be called more than once for the same id;
--- subsequent calls update the definition but do not reset unlock state.
--- opts: { name, desc, hidden, cond }
function Achievement.define(id, opts)
    opts = opts or {}
    _defs[id] = {
        name   = opts.name   or id,
        desc   = opts.desc   or "",
        hidden = opts.hidden == true,
        cond   = opts.cond   or nil,
    }
    -- Initialise state slot only if not already present (preserve existing unlock)
    if not _state[id] then
        _state[id] = { unlocked = false, unlockedAt = nil }
    end
end

--- Unlocks achievement `id` if it exists and has not already been unlocked.
--- Automatically saves state and emits "achievement_unlocked" { id, name }.
function Achievement.unlock(id)
    if not _defs[id] then
        Log.warn("Achievement.unlock: unknown achievement '" .. tostring(id) .. "'")
        return
    end

    if _state[id] and _state[id].unlocked then
        return  -- already unlocked, nothing to do
    end

    if not _state[id] then _state[id] = {} end
    _state[id].unlocked   = true
    _state[id].unlockedAt = os.time()

    Achievement.save()

    EventBus.emit("achievement_unlocked", {
        id   = id,
        name = _defs[id].name,
    })
end

--- Returns true if achievement `id` has been unlocked, false otherwise.
function Achievement.isUnlocked(id)
    return _state[id] ~= nil and _state[id].unlocked == true
end

--- Returns a snapshot table: { [id] = { name, desc, hidden, unlocked, unlockedAt } }
--- for every defined achievement.
function Achievement.getAll()
    local result = {}
    for id, def in pairs(_defs) do
        local s = _state[id] or {}
        result[id] = {
            name       = def.name,
            desc       = def.desc,
            hidden     = def.hidden,
            unlocked   = s.unlocked   or false,
            unlockedAt = s.unlockedAt or nil,
        }
    end
    return result
end

--- Returns a sorted list of ids for every unlocked achievement.
function Achievement.getUnlocked()
    local list = {}
    for id, s in pairs(_state) do
        if s.unlocked then
            table.insert(list, id)
        end
    end
    table.sort(list)
    return list
end

--- Resets all achievement state and clears persisted data.
--- Intended for test suites only — does not remove definitions.
function Achievement.reset()
    for id in pairs(_state) do
        _state[id] = { unlocked = false, unlockedAt = nil }
    end
    Persist.set(PERSIST_KEY, {})
end

-- ─── Auto-load ────────────────────────────────────────────────────────────────

Achievement.load()

return Achievement
