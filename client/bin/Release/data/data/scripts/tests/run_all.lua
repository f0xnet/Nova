-- tests/run_all.lua
-- Point d'entrée de la suite de tests Nova.
-- Charge via ScriptSystem comme script global :
--   m_scriptSystem->loadGlobalScript("data/scripts/tests/run_all.lua")
--
-- Chaque fichier de tests retourne { pass, fail }.
-- Ce runner agrège les résultats et affiche un bilan final.
--
-- Modules couverts (41 total) :
--   test_core.lua     → EventBus · Timer · Scheduler · Tween
--   test_math.lua     → Math · Table · Random · Easing · Color (ext)
--   test_gameplay.lua → Stats · Effect · Inventory · Quest · Flag · Loot · Persist
--   test_systems.lua  → Sequence · Conversation · InputBind · I18n · Achievement · Data
--   test_engine.lua   → Anim · Trigger · Nav · Spatial · Pool · Debug · UI · Camera · Scene

local suites = {
    "tests/test_core",
    "tests/test_math",
    "tests/test_gameplay",
    "tests/test_systems",
    "tests/test_engine",
}

Log.info("══════════════════════════════════════════════")
Log.info("  NOVA TEST SUITE")
Log.info("══════════════════════════════════════════════")

local totalPass = 0
local totalFail = 0
local errors    = {}

for _, path in ipairs(suites) do
    local ok, result = pcall(require, path)
    if ok and type(result) == "table" then
        totalPass = totalPass + (result.pass or 0)
        totalFail = totalFail + (result.fail or 0)
    elseif not ok then
        table.insert(errors, { path = path, err = tostring(result) })
        Log.error("[TEST RUNNER] Failed to load '" .. path .. "': " .. tostring(result))
    end
end

Log.info("──────────────────────────────────────────────")

if #errors > 0 then
    Log.error(string.format("[TEST RUNNER] %d suite(s) failed to load:", #errors))
    for _, e in ipairs(errors) do
        Log.error("  ✗ " .. e.path .. " → " .. e.err)
    end
end

local total = totalPass + totalFail
if totalFail == 0 and #errors == 0 then
    Log.info(string.format(
        "[TEST RUNNER] ✓ ALL PASS  %d/%d tests", totalPass, total))
else
    Log.warn(string.format(
        "[TEST RUNNER] ✗ %d/%d pass  |  %d fail  |  %d suite error(s)",
        totalPass, total, totalFail, #errors))
end

Log.info("══════════════════════════════════════════════")
