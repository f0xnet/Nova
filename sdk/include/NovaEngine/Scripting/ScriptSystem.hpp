#pragma once

#include "../ECS/System.hpp"
#include "../ECS/EntityRegistry.hpp"
#include "../ECS/SceneManager.hpp"
#include "../ECS/ComponentFactory.hpp"
#include "../ECS/Components.hpp"
#include "../Core/Logger.hpp"
#include "../Backend/BackendManager.hpp"
#include "ScriptComponent.hpp"
#include "LuaBindings.hpp"
#include <sol/sol.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace NovaEngine {

// ============================================================================
// ScriptSystem — moteur de scripting Lua trois couches
//
// COUCHE 1 — Bindings C++ (dans LuaBindings.hpp) :
//   Entity, Registry, Input, Audio, Resources, Scene, Vec2f, Color, composants
//
// COUCHE 2 — Modules Lua nova (data/scripts/nova/) auto-chargés comme globals :
//   EventBus    — bus d'événements pub/sub inter-scripts
//   Timer       — timers one-shot et répétitifs
//   Scheduler   — coroutines asynchrones (équivalent Utility.Wait de Papyrus)
//   StateMachine — FSM légère pour les IA et états
//   Vec2        — utilitaires math 2D
//   World       — helpers entités (findByTag, spawn, destroy, nearest...)
//   Tween       — interpolation fluide (position, alpha, couleur...)
//   Class       — système OOP avec héritage prototype
//   Quest       — quêtes, étapes, objectifs, récompenses
//   Persist     — données persistantes (survit aux changements de scène)
//
// COUCHE 3 — Scripts jeu (data/scripts/) :
//   Scripts entité : init(entity) + update(entity, dt) par ScriptComponent
//   Scripts globaux : init() + update(dt) via loadGlobalScript()
//
// FONCTIONNALITÉS AVANCÉES :
//   - Properties : données JSON par instance injectées en `self` avant init()
//   - ScriptRegistry : ScriptRegistry.call(entityId, "fn", args) inter-scripts
//   - Input events : key_down_X / key_up_X / mouse_down_X via EventBus
//   - Animation events : animation_changed / animation_frame via EventBus
//   - Update interval par entité (ScriptComponent.updateInterval)
//   - Bridge automatique ActivatorComponent → EventBus
//   - emit() pour envoyer des événements C++ → Lua
//   - loadGlobalScript() pour scripts non-entité (quêtes, IA globale, etc.)
//   - Sandboxes par entité (erreur d'un script n'affecte pas les autres)
//   - package.path configuré pour require("nova/...") et require("game/...")
//
// CONTRAT SCRIPT ENTITÉ (data/scripts/goblin.lua) :
//   function init(entity)         -- appelé une fois (self = properties JSON)
//   function update(entity, dt)   -- appelé chaque frame (ou à updateInterval)
//
// CONTRAT SCRIPT GLOBAL (data/scripts/game/quests.lua) :
//   function init()               -- appelé une fois
//   function update(dt)           -- appelé chaque frame
//
// GLOBALS DISPONIBLES DANS TOUS LES SCRIPTS :
//   EventBus, Timer, Scheduler, StateMachine, Vec2, World
//   Tween, Class, Quest, Persist
//   ScriptRegistry
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
            sol::lib::os,
            sol::lib::coroutine,
            sol::lib::package
        );

        // Ensure saves directory exists for Persist module
        std::filesystem::create_directories("data/saves");

        configurePackagePath();
        LuaBindings::registerAll(m_lua);
        if (m_sceneManager)
            LuaBindings::registerSceneManager(m_lua, *m_sceneManager);

        initScriptRegistry();
        loadNovaModules();
        REGISTER_COMPONENT(ScriptComponent);

        LOG_INFO("[ScriptSystem] Lua {}.{}.{} — EventBus/Timer/Scheduler/StateMachine/Vec2/World/"
                 "Tween/Class/Quest/Persist/ScriptRegistry chargés",
            LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    }

    // -------------------------------------------------------------------------
    // System interface
    // -------------------------------------------------------------------------
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return { ScriptComponent::staticTypeID() };
    }

    void update(float deltaTime, EntityRegistry& registry) override {
        // 1. Met à jour le global Registry
        m_lua["Registry"] = &registry;

        // 2. Bridge activateurs → EventBus
        updateActivatorBridge(registry);

        // 3. Input events → EventBus (key_down_X / key_up_X)
        updateInputBridge();

        // 4. Met à jour la stdlib nova (Timer, Scheduler, Tween)
        updateNovaRuntime(deltaTime);

        // 5. Scripts globaux (quêtes, IA de fond, cycle jour/nuit...)
        for (auto& gs : m_globalScripts)
            updateGlobalScript(gs, deltaTime);

        // 6. Scripts entités
        auto entities = registry.getEntitiesWith(getRequiredComponents());
        for (auto* entity : entities) {
            auto* script = entity->getComponent<ScriptComponent>();
            if (!script || !script->enabled || script->errored) continue;

            if (!script->loaded)
                loadEntityScript(entity, script);

            if (script->errored || !script->fnUpdate.valid()) continue;

            script->m_updateAccum += deltaTime;
            if (script->updateInterval > 0.0f &&
                script->m_updateAccum < script->updateInterval) continue;

            float effectiveDt     = script->m_updateAccum;
            script->m_updateAccum = 0.0f;

            auto result = script->fnUpdate(entity, effectiveDt);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("[ScriptSystem] update '{}': {}", script->scriptPath, err.what());
                script->errored = true;
            }
        }

        // 7. Animation events (après update scripts pour frame cohérente)
        updateAnimationBridge(registry);
    }

    // -------------------------------------------------------------------------
    // API publique
    // -------------------------------------------------------------------------

    sol::state& getLua() { return m_lua; }

    // Charge un script global (non-entité) : quête, logique de fond, etc.
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

    sol::table createTable() { return m_lua.create_table(); }

    // Force le rechargement d'un script au prochain update
    void reloadScript(ScriptComponent* script) {
        if (!script) return;
        script->loaded        = false;
        script->errored       = false;
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
    std::unordered_map<std::string, bool> m_prevKeyState;
    std::unordered_map<std::string, bool> m_prevMouseState;
    std::unordered_map<u64, u32>       m_animPrevFrame;
    std::unordered_map<u64, std::string> m_animPrevAnimID;

    // -------------------------------------------------------------------------
    // Initialisation
    // -------------------------------------------------------------------------
    void configurePackagePath() {
        std::string cur = m_lua["package"]["path"].get_or(std::string(""));
        m_lua["package"]["path"] =
            cur + ";data/scripts/?.lua;data/scripts/?/init.lua";
    }

    // -------------------------------------------------------------------------
    // ScriptRegistry — ScriptRegistry.call(entityId, "fn", arg1, arg2, ...)
    //                   ScriptRegistry.has(entityId)
    //                   ScriptRegistry.getEnv(entityId)
    // -------------------------------------------------------------------------
    void initScriptRegistry() {
        sol::table reg  = m_lua.create_table();
        sol::table envs = m_lua.create_table();
        reg["_envs"] = envs;

        reg["has"] = [](sol::this_state ts, u64 id) -> bool {
            sol::state_view lua(ts);
            sol::object e = lua["ScriptRegistry"]["_envs"][id];
            return e.valid() && e.get_type() != sol::type::lua_nil;
        };

        reg["getEnv"] = [](sol::this_state ts, u64 id) -> sol::object {
            sol::state_view lua(ts);
            return lua["ScriptRegistry"]["_envs"][id];
        };

        // ScriptRegistry.call(entityId, "functionName", arg1, arg2, ...)
        // Appelle une fonction dans le sandbox d'une autre entité.
        reg["call"] = [](sol::this_state ts, u64 id,
                         const std::string& fn,
                         sol::variadic_args args) -> sol::object {
            sol::state_view lua(ts);
            sol::object envObj = lua["ScriptRegistry"]["_envs"][id];
            if (!envObj.valid() || envObj.get_type() != sol::type::table)
                return sol::lua_nil;
            sol::table env = envObj;
            sol::object fnObj = env[fn];
            if (!fnObj.valid() || fnObj.get_type() != sol::type::function)
                return sol::lua_nil;
            sol::protected_function pf = fnObj;
            std::vector<sol::object> v(args.begin(), args.end());
            sol::protected_function_result r;
            switch (v.size()) {
                case 0:  r = pf(); break;
                case 1:  r = pf(v[0]); break;
                case 2:  r = pf(v[0], v[1]); break;
                case 3:  r = pf(v[0], v[1], v[2]); break;
                default: r = pf(v[0], v[1], v[2], v[3]); break;
            }
            if (!r.valid()) {
                sol::error err = r;
                LOG_WARN("[ScriptRegistry] call({}, '{}'): {}", id, fn, err.what());
                return sol::lua_nil;
            }
            return r;
        };

        m_lua["ScriptRegistry"] = reg;
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
            { "Tween",        "nova/tween"        },
            { "Class",        "nova/class"        },
            { "Quest",        "nova/quest"        },
            { "Persist",      "nova/persist"      },
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
        callTableMethod("Tween",     "update", dt);
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

            auto data        = m_lua.create_table();
            data["entityId"] = id;
            data["actionID"] = act->actionID;
            data["active"]   = act->isActive;
            callEventBusEmit(eventName, data);
        }
    }

    // -------------------------------------------------------------------------
    // Bridge Input → EventBus
    // Émet key_down_X / key_up_X et mouse_down_X / mouse_up_X
    //
    // Usage Lua :
    //   EventBus.on("key_down_E", function(data) startInteraction() end)
    //   EventBus.on("mouse_down_left", function(data) attack() end)
    // -------------------------------------------------------------------------
    void updateInputBridge() {
        const auto& keyMap = LuaBindings::getKeyMap();
        for (const auto& [name, code] : keyMap) {
            bool isNow   = INPUT().isKeyPressed(code);
            auto it      = m_prevKeyState.find(name);
            bool wasPrev = (it != m_prevKeyState.end()) ? it->second : false;
            if (isNow == wasPrev) continue;

            m_prevKeyState[name] = isNow;
            auto data    = m_lua.create_table();
            data["key"]  = name;
            callEventBusEmit(isNow ? ("key_down_" + name) : ("key_up_" + name), data);
        }

        static const std::pair<const char*, MouseButton> s_buttons[] = {
            {"left",   MouseButton::Left},
            {"right",  MouseButton::Right},
            {"middle", MouseButton::Middle},
        };
        for (auto& [name, btn] : s_buttons) {
            bool isNow   = INPUT().isMouseButtonPressed(btn);
            bool wasPrev = m_prevMouseState[name];
            if (isNow == wasPrev) continue;

            m_prevMouseState[name] = isNow;
            auto data       = m_lua.create_table();
            data["button"]  = name;
            auto pos        = INPUT().getMousePosition();
            data["x"]       = pos.x;
            data["y"]       = pos.y;
            callEventBusEmit(isNow ? (std::string("mouse_down_") + name)
                                   : (std::string("mouse_up_")   + name), data);
        }
    }

    // -------------------------------------------------------------------------
    // Bridge AnimationComponent → EventBus
    // Émet animation_changed (quand animationID change) et animation_frame
    // (quand currentFrame change)
    //
    // Usage Lua :
    //   EventBus.on("animation_frame", function(data)
    //       if data.entityId == entity.id
    //          and data.animationID == "attack"
    //          and data.frame == 5 then
    //           spawnHitbox()
    //       end
    //   end)
    // -------------------------------------------------------------------------
    void updateAnimationBridge(EntityRegistry& registry) {
        auto animated = registry.getEntitiesWith({ AnimationComponent::staticTypeID() });
        for (auto* entity : animated) {
            auto* anim = entity->getComponent<AnimationComponent>();
            if (!anim) continue;
            u64 id = entity->getID();

            // animation_changed
            auto prevIDIt = m_animPrevAnimID.find(id);
            const std::string& prevID = (prevIDIt != m_animPrevAnimID.end())
                ? prevIDIt->second : std::string("");
            if (anim->animationID != prevID) {
                m_animPrevAnimID[id] = anim->animationID;
                auto data              = m_lua.create_table();
                data["entityId"]       = id;
                data["animationID"]    = anim->animationID;
                data["previousID"]     = prevID;
                callEventBusEmit("animation_changed", data);
            }

            // animation_frame
            auto prevFrIt = m_animPrevFrame.find(id);
            u32 prevFrame = (prevFrIt != m_animPrevFrame.end()) ? prevFrIt->second : UINT32_MAX;
            if (anim->currentFrame != prevFrame) {
                m_animPrevFrame[id]  = anim->currentFrame;
                auto data            = m_lua.create_table();
                data["entityId"]     = id;
                data["frame"]        = (int)anim->currentFrame;
                data["animationID"]  = anim->animationID;
                callEventBusEmit("animation_frame", data);
            }
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

            // Injecte les properties JSON en tant que table `self` dans l'env
            auto self = m_lua.create_table();
            for (auto& [key, val] : script->properties.items()) {
                if      (val.is_string())         self[key] = val.get<std::string>();
                else if (val.is_number_float())   self[key] = val.get<double>();
                else if (val.is_number_integer()) self[key] = val.get<int64_t>();
                else if (val.is_boolean())        self[key] = val.get<bool>();
            }
            script->env["self"] = self;

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

            // Enregistre dans ScriptRegistry avant init() pour les inter-appels
            sol::object reg = m_lua["ScriptRegistry"];
            if (reg.valid() && reg.get_type() == sol::type::table) {
                sol::table envs = m_lua["ScriptRegistry"]["_envs"];
                envs[entity->getID()] = script->env;
            }

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
