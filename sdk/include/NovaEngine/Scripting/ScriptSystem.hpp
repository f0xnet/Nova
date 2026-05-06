#pragma once

#include "../ECS/System.hpp"
#include "../ECS/EntityRegistry.hpp"
#include "../ECS/SceneManager.hpp"
#include "../ECS/ComponentFactory.hpp"
#include "../ECS/Components.hpp"
#include "../Core/Logger.hpp"
#include "ScriptComponent.hpp"
#include "LuaBindings.hpp"
#include <sol/sol.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace NovaEngine {

// ============================================================================
// ScriptSystem — moteur de scripting Lua trois couches
//
// COUCHE 1 — Bindings C++ (dans LuaBindings.hpp) :
//   Entity, Registry, Input, Audio, Resources, Scene, Vec2f, Color, composants
//
// COUCHE 2 — Modules Lua nova (data/scripts/nova/) auto-chargés comme globals :
//   EventBus   — bus d'événements pub/sub inter-scripts
//   Timer      — timers one-shot et répétitifs
//   Scheduler  — coroutines asynchrones (équivalent Utility.Wait de Papyrus)
//   StateMachine — FSM légère pour les IA et états
//   Vec2       — utilitaires math 2D
//   World      — helpers entités (findByTag, spawn, destroy, nearest...)
//
// COUCHE 3 — Scripts jeu (data/scripts/) :
//   Scripts entité : init(entity) + update(entity, dt) par ScriptComponent
//   Scripts globaux : init() + update(dt) via loadGlobalScript()
//
// FONCTIONNALITÉS :
//   - Update interval par entité (ScriptComponent.updateInterval)
//   - Bridge automatique ActivatorComponent → EventBus
//   - emit() pour envoyer des événements C++ → Lua
//   - loadGlobalScript() pour scripts non-entité (quêtes, IA globale, etc.)
//   - Sandboxes par entité (erreur d'un script n'affecte pas les autres)
//   - package.path configuré pour require("nova/...") et require("game/...")
//
// CONTRAT SCRIPT ENTITÉ (data/scripts/player.lua) :
//   function init(entity)         -- appelé une fois au chargement
//   function update(entity, dt)   -- appelé chaque frame (ou à updateInterval)
//
// CONTRAT SCRIPT GLOBAL (data/scripts/game/quests.lua) :
//   function init()               -- appelé une fois
//   function update(dt)           -- appelé chaque frame
//
// GLOBALS DISPONIBLES DANS TOUS LES SCRIPTS :
//   EventBus, Timer, Scheduler, StateMachine, Vec2, World
//   Registry, Input, Log, print, Audio, Resources, Scene (si SceneManager fourni)
//   dialogueActive (mis à jour par Game.cpp)
// ============================================================================

class ScriptSystem : public System {
public:
    // -------------------------------------------------------------------------
    // Constructeur — sm optionnel, expose "Scene" en Lua si fourni
    // -------------------------------------------------------------------------
    explicit ScriptSystem(SceneManager* sm = nullptr) : m_sceneManager(sm) {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::io,
            sol::lib::coroutine,
            sol::lib::package
        );

        configurePackagePath();
        LuaBindings::registerAll(m_lua);
        if (m_sceneManager)
            LuaBindings::registerSceneManager(m_lua, *m_sceneManager);

        loadNovaModules();
        REGISTER_COMPONENT(ScriptComponent);

        LOG_INFO("[ScriptSystem] Lua {}.{}.{} — EventBus/Timer/Scheduler/StateMachine/Vec2/World chargés",
            LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    }

    // -------------------------------------------------------------------------
    // System interface
    // -------------------------------------------------------------------------
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return { ScriptComponent::staticTypeID() };
    }

    void update(float deltaTime, EntityRegistry& registry) override {
        // 1. Met à jour le global Registry (toujours en premier)
        m_lua["Registry"] = &registry;

        // 2. Bridge activateurs → EventBus (sans toucher au moteur)
        updateActivatorBridge(registry);

        // 3. Met à jour la stdlib nova (Timer, Scheduler)
        updateNovaRuntime(deltaTime);

        // 4. Scripts globaux (quêtes, IA de fond, cycle jour/nuit...)
        for (auto& gs : m_globalScripts)
            updateGlobalScript(gs, deltaTime);

        // 5. Scripts entités
        auto entities = registry.getEntitiesWith(getRequiredComponents());
        for (auto* entity : entities) {
            auto* script = entity->getComponent<ScriptComponent>();
            if (!script || !script->enabled || script->errored) continue;

            if (!script->loaded)
                loadEntityScript(entity, script);

            if (script->errored || !script->fnUpdate.valid()) continue;

            // Update interval : passe le dt accumulé pour garder la physique correcte
            script->m_updateAccum += deltaTime;
            if (script->updateInterval > 0.0f &&
                script->m_updateAccum < script->updateInterval) continue;

            float effectiveDt    = script->m_updateAccum;
            script->m_updateAccum = 0.0f;

            auto result = script->fnUpdate(entity, effectiveDt);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("[ScriptSystem] update '{}': {}", script->scriptPath, err.what());
                script->errored = true;
            }
        }
    }

    // -------------------------------------------------------------------------
    // API publique
    // -------------------------------------------------------------------------

    // Accès direct au state Lua — pour enregistrer des bindings custom
    sol::state& getLua() { return m_lua; }

    // Charge un script global (non-entité) : quête, logique de fond, etc.
    // updateInterval = 0 → chaque frame ; > 0 → toutes les N secondes
    void loadGlobalScript(const std::string& path, float updateInterval = 0.0f) {
        m_globalScripts.push_back({ path, {}, {}, false, false, updateInterval, 0.0f });
        LOG_DEBUG("[ScriptSystem] Global script enregistré : '{}'", path);
    }

    // Émet un événement vers le bus Lua (sans données)
    void emit(const std::string& eventName) {
        callEventBusEmit(eventName, sol::lua_nil);
    }

    // Émet un événement avec une table de données
    void emit(const std::string& eventName, sol::table data) {
        callEventBusEmit(eventName, data);
    }

    // Crée une table Lua — utile pour construire des données avant emit()
    sol::table createTable() { return m_lua.create_table(); }

    // Force le rechargement d'un script au prochain update
    void reloadScript(ScriptComponent* script) {
        if (!script) return;
        script->loaded       = false;
        script->errored      = false;
        script->m_updateAccum = 0.0f;
    }

private:
    // -------------------------------------------------------------------------
    // Types internes
    // -------------------------------------------------------------------------
    struct GlobalScript {
        std::string path;
        sol::environment        env;
        sol::protected_function fnUpdate;
        bool  loaded         = false;
        bool  errored        = false;
        float updateInterval = 0.0f;
        float accumulator    = 0.0f;
    };

    // -------------------------------------------------------------------------
    // Membres
    // -------------------------------------------------------------------------
    sol::state    m_lua;
    SceneManager* m_sceneManager = nullptr;
    std::vector<GlobalScript>          m_globalScripts;
    std::unordered_map<u64, bool>      m_activatorPrevState;

    // -------------------------------------------------------------------------
    // Initialisation
    // -------------------------------------------------------------------------
    void configurePackagePath() {
        std::string cur = m_lua["package"]["path"].get_or(std::string(""));
        m_lua["package"]["path"] =
            cur + ";data/scripts/?.lua;data/scripts/?/init.lua";
    }

    // Charge les modules nova comme globals — dégradation gracieuse si absents
    void loadNovaModules() {
        const std::pair<const char*, const char*> modules[] = {
            { "EventBus",     "nova/event_bus"    },
            { "Timer",        "nova/timer"        },
            { "Scheduler",    "nova/scheduler"    },
            { "StateMachine", "nova/state_machine"},
            { "Vec2",         "nova/vec2"         },
            { "World",        "nova/world"        },
        };
        for (auto& [global, mod] : modules) {
            std::string code = std::string(global) + " = require('" + mod + "')";
            auto result = m_lua.safe_script(code, sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                LOG_WARN("[ScriptSystem] Module '{}' non trouvé : {}", mod, err.what());
            }
        }
    }

    // -------------------------------------------------------------------------
    // Runtime nova — appelé chaque frame avant les scripts
    // -------------------------------------------------------------------------
    void updateNovaRuntime(float dt) {
        callTableMethod("Timer",     "update", dt);
        callTableMethod("Scheduler", "update", dt);
    }

    void callTableMethod(const char* table, const char* method, float dt) {
        sol::object t = m_lua[table];
        if (!t.valid() || t.get_type() != sol::type::table) return;
        sol::protected_function fn = m_lua[table][method];
        if (!fn.valid()) return;
        auto r = fn(dt);
        if (!r.valid()) {
            sol::error err = r;
            LOG_WARN("[ScriptSystem] {}.{}(dt) error: {}", table, method, err.what());
        }
    }

    // -------------------------------------------------------------------------
    // Bridge ActivatorComponent → EventBus
    // Détecte les changements d'état et émet les événements correspondants.
    // Utilise onActivateEvent / onDeactivateEvent de ActivatorComponent si définis,
    // sinon "activator_on" / "activator_off" avec {entityId, actionID, active}.
    // -------------------------------------------------------------------------
    void updateActivatorBridge(EntityRegistry& registry) {
        auto activators = registry.getEntitiesWith({ ActivatorComponent::staticTypeID() });
        for (auto* entity : activators) {
            auto* act = entity->getComponent<ActivatorComponent>();
            if (!act) continue;

            u64  id   = entity->getID();
            bool prev = m_activatorPrevState.count(id) ? m_activatorPrevState[id] : false;

            if (act->isActive == prev) continue;
            m_activatorPrevState[id] = act->isActive;

            const std::string& eventName = act->isActive
                ? (act->onActivateEvent.empty()   ? "activator_on"  : act->onActivateEvent)
                : (act->onDeactivateEvent.empty() ? "activator_off" : act->onDeactivateEvent);

            auto data     = m_lua.create_table();
            data["entityId"] = id;
            data["actionID"] = act->actionID;
            data["active"]   = act->isActive;
            callEventBusEmit(eventName, data);
        }
    }

    // -------------------------------------------------------------------------
    // Chargement scripts entité
    // -------------------------------------------------------------------------
    void loadEntityScript(Entity* entity, ScriptComponent* script) {
        if (script->scriptPath.empty()) {
            LOG_WARN("[ScriptSystem] Entity {} : scriptPath vide", entity->getID());
            script->errored = true;
            return;
        }
        try {
            script->env = sol::environment(m_lua, sol::create, m_lua.globals());
            auto res = m_lua.script_file(script->scriptPath, script->env,
                                         sol::script_pass_on_error);
            if (!res.valid()) {
                sol::error err = res;
                LOG_ERROR("[ScriptSystem] Chargement '{}' : {}", script->scriptPath, err.what());
                script->errored = true;
                return;
            }
            script->fnInit   = script->env["init"];
            script->fnUpdate = script->env["update"];
            script->loaded   = true;
            LOG_DEBUG("[ScriptSystem] Chargé '{}'", script->scriptPath);

            if (script->fnInit.valid()) {
                auto r = script->fnInit(entity);
                if (!r.valid()) {
                    sol::error err = r;
                    LOG_ERROR("[ScriptSystem] init '{}' : {}", script->scriptPath, err.what());
                    script->errored = true;
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[ScriptSystem] Exception '{}' : {}", script->scriptPath, e.what());
            script->errored = true;
        }
    }

    // -------------------------------------------------------------------------
    // Chargement et update scripts globaux
    // -------------------------------------------------------------------------
    void loadGlobalScriptImpl(GlobalScript& gs) {
        try {
            gs.env = sol::environment(m_lua, sol::create, m_lua.globals());
            auto res = m_lua.script_file(gs.path, gs.env, sol::script_pass_on_error);
            if (!res.valid()) {
                sol::error err = res;
                LOG_ERROR("[ScriptSystem] Global '{}' : {}", gs.path, err.what());
                gs.errored = true;
                return;
            }
            gs.fnUpdate = gs.env["update"];
            gs.loaded   = true;
            LOG_DEBUG("[ScriptSystem] Global chargé '{}'", gs.path);

            sol::protected_function fnInit = gs.env["init"];
            if (fnInit.valid()) {
                auto r = fnInit();
                if (!r.valid()) {
                    sol::error err = r;
                    LOG_ERROR("[ScriptSystem] Global init '{}' : {}", gs.path, err.what());
                    gs.errored = true;
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[ScriptSystem] Exception global '{}' : {}", gs.path, e.what());
            gs.errored = true;
        }
    }

    void updateGlobalScript(GlobalScript& gs, float deltaTime) {
        if (!gs.loaded)  loadGlobalScriptImpl(gs);
        if (gs.errored || !gs.fnUpdate.valid()) return;

        gs.accumulator += deltaTime;
        if (gs.updateInterval > 0.0f && gs.accumulator < gs.updateInterval) return;

        float effectiveDt = gs.accumulator;
        gs.accumulator    = 0.0f;

        auto r = gs.fnUpdate(effectiveDt);
        if (!r.valid()) {
            sol::error err = r;
            LOG_ERROR("[ScriptSystem] Global update '{}' : {}", gs.path, err.what());
            gs.errored = true;
        }
    }

    // -------------------------------------------------------------------------
    // Helpers internes
    // -------------------------------------------------------------------------
    void callEventBusEmit(const std::string& eventName, sol::object data) {
        sol::object bus = m_lua["EventBus"];
        if (!bus.valid() || bus.get_type() != sol::type::table) return;
        sol::protected_function fn = m_lua["EventBus"]["emit"];
        if (!fn.valid()) return;
        auto r = fn(eventName, data);
        if (!r.valid()) {
            sol::error err = r;
            LOG_ERROR("[ScriptSystem] EventBus.emit('{}') : {}", eventName, err.what());
        }
    }
};

} // namespace NovaEngine
