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
#include <unordered_set>
#include <utility>
#include <filesystem>
#include <chrono>

namespace NovaEngine {

// ============================================================================
// ScriptSystem — moteur de scripting Lua trois couches
//
// COUCHE 1 — Bindings C++ (dans LuaBindings.hpp) :
//   Entity, Registry, Input, Audio, Resources, Scene, Vec2f, Color, composants
//
// COUCHE 2 — Modules Lua nova (data/scripts/nova/) auto-chargés comme globals :
//   EventBus, Timer, Scheduler, StateMachine, Vec2, World
//   Tween, Class, Quest, Persist, Game, Stats
//   Camera, InputEx, Effect, Cooldown, Sound, Inventory, Notify, Physics, Flag, Scene
//   Math, Table, Random, Color (étend le type C++), Sequence, Conversation, Anim
//   Trigger, InputBind, Loot, Nav, Particles, Projectile, EntityState, SceneFX
//
// COUCHE 3 — Scripts jeu (data/scripts/) :
//   Scripts entité  : init(entity) + update(entity, dt)  — sandbox isolé par entité
//   Scripts de scène : init() + update(dt) — chargé quand la scène devient active
//   Scripts globaux : init() + update(dt) via loadGlobalScript() — persistent toujours
//
// ── NAMED EVENT HANDLERS ─────────────────────────────────────────────────────
//   Auto-branchés par __wireNamedHandlers sans EventBus.on() manuel.
//   Tous les handlers sont trackés et désabonnés automatiquement à la destruction
//   de l'entité (event entity_destroyed). Ne pas utiliser EventBus.on() directement
//   dans un script d'entité — utiliser ces named handlers ou Scene.listen().
//
//     function OnActivate(activatorId, actionID)      end
//     function OnDeactivate(activatorId, actionID)    end
//     function OnKeyDown(key)                         end
//     function OnKeyUp(key)                           end
//     function OnMouseDown(button, x, y)              end
//     function OnMouseUp(button, x, y)                end
//     function OnAnimationEvent(animID, frame)        end
//     function OnAnimationChanged(newID, prevID)      end
//     function OnMessage(msg, payload)                end
//     function OnQuestComplete(questId)               end
//     function OnQuestAdvanced(questId, stage)        end
//     function OnStatZeroed(entityId, stat)           end
//     function OnItemAdded(item, count)               end
//     function OnItemRemoved(item, count)             end
//     function OnEffectApplied(effectId, entityId)    end
//     function OnEffectExpired(effectId, entityId)    end
//     function OnFlagSet(name)                        end
//     function OnFlagUnset(name)                      end
//     function OnTriggerEnter(triggerId, entityId)    end
//     function OnTriggerExit(triggerId, entityId)     end
//     function OnSceneChanged(name)                   end
//     function OnConversationNode(nodeId, spkr, text) end
//     function OnConversationEnd(conversationId)      end
//     function OnUIAction(action, value, componentId) end
//     function OnCollisionEnter(otherEntityId)        end
//     function OnCollisionExit(otherEntityId)         end
//
// ── NETTOYAGE AUTOMATIQUE ────────────────────────────────────────────────────
//   Destruction d'entité (Registry.destroyEntity) :
//     → entity_destroyed émis AVANT la suppression
//     → tous les named handlers ET les EventBus.on() appelés dans init() désabonnés
//     → ScriptRegistry._envs[id] = nil (env Lua collectable par le GC)
//     → Stats.clear / Effect.clear / Cooldown.resetAll / Anim.clearCallbacks
//
//   Changement de scène (Scene.setActive) :
//     → Timer.cancelScope(prev) — timers de la scène sortante annulés
//     → Scheduler.cancelScope(prev) — coroutines annulées
//     → Scene.listen() handlers de la scène sortante désabonnés
//     → Named handlers des scripts de scène (isSceneScript=true) désabonnés
//     → Sequence, Particles, Projectile réinitialisés via scene_changed
//
//   Timer et Scheduler : auto-scopés à Scene.current() si pas de scope fourni.
//   Impossible de créer accidentellement un timer qui fuite entre scènes.
//
// ── PROPAGATION D'ERREURS ───────────────────────────────────────────────────
//   Toute erreur script (load / init / update) émet "script_error" :
//     { entityId, path, error, phase }  — exploitable via Debug.watch ou EventBus.on
//
// ── NETTOYAGE SCÈNE ───────────────────────────────────────────────────────────
//   Les scripts de scène (chargés via ScriptSystem) reçoivent entityId=0 lors du
//   wiring. Ils sont distingués des scripts globaux par isSceneScript=true dans
//   GlobalScript. __wireNamedHandlers reçoit isSceneScript en troisième argument :
//   les named handlers sont alors nettoyés sur "scene_changing".
//
// ── CONTRÔLE D'UPDATE ────────────────────────────────────────────────────────
//     RegisterForUpdate(0.5)   -- change la fréquence d'update à 0.5s
//     UnregisterForUpdate()    -- suspend les updates (sans désactiver init)
//     ResumeUpdate()           -- reprend les updates
//
// ── INTER-SCRIPT ─────────────────────────────────────────────────────────────
//     ScriptRegistry.call(entityId, "fn", arg1, arg2)
//     ScriptRegistry.sendMessage(entityId, "msg", payload)
//     entity:sendMessage("msg", payload)
//
// ── GLOBALS DISPONIBLES ──────────────────────────────────────────────────────
//     EventBus, Timer, Scheduler, StateMachine, Vec2, World
//     Tween, Class, Quest, Persist, Game, Stats, Stat, Effect, Cooldown
//     Camera, InputEx, Sound, Inventory, Notify, Physics, Flag, Scene
//     Sequence, Conversation, Anim, Trigger, InputBind, Loot, Nav
//     Particles, Projectile, Pool, Spatial, Easing, I18n, Achievement
//     Data, Debug, SceneFX, EntityState, ScriptRegistry, BT
//     Registry, Input, Log, print, Audio, Resources
// ============================================================================

class ScriptSystem : public System {
public:
    explicit ScriptSystem(SceneManager* sm = nullptr) : m_sceneManager(sm) {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::io,
            sol::lib::os,
            sol::lib::coroutine,
            sol::lib::package,
            sol::lib::debug   // requis pour debug.traceback dans les erreurs de script
        );

        // Sandbox global : commandes système uniquement.
        // load/loadfile/dofile sont laissés globaux (requis par les modules nova comme Persist).
        // Ils sont neutralisés par-environnement dans loadEntityScript / loadGlobalScriptImpl.
        m_lua["io"]["popen"]   = sol::lua_nil;
        m_lua["os"]["execute"] = sol::lua_nil;
        m_lua["os"]["exit"]    = sol::lua_nil;

        std::filesystem::create_directories("data/saves");

        configurePackagePath();
        LuaBindings::registerAll(m_lua);
        if (m_sceneManager)
            LuaBindings::registerSceneManager(m_lua, *m_sceneManager);

        // Pré-alimente les tables d'état des bridges (évite find() + insert chaque frame)
        {
            const auto& km = LuaBindings::getKeyMap();
            m_prevKeyState.reserve(km.size());
            for (auto& [k, _] : km) m_prevKeyState.emplace(k, false);
        }
        for (const char* b : { "left", "right", "middle" }) m_prevMouseState.emplace(b, false);

        initScriptRegistry();
        initNamedHandlersWirer();
        loadNovaModules();  // appelle cacheRuntimeUpdates() en fin de chargement
        REGISTER_COMPONENT(ScriptComponent);

        LOG_INFO("[ScriptSystem] Lua {}.{}.{} — stdlib complète chargée",
            LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    }

    // -------------------------------------------------------------------------
    // System interface
    // -------------------------------------------------------------------------
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return { ScriptComponent::staticTypeID() };
    }

    void update(float deltaTime, EntityRegistry& registry) override {
        m_registry = &registry;
        m_lua["Registry"] = &registry;

        // Réinitialise les états "just pressed/released" avant la détection
        if (m_inputExClearFn.valid()) {
            auto r = m_inputExClearFn(0.0f);
            if (!r.valid()) reportScriptError(r, "update", "InputEx._clear");
        }

        updateActivatorBridge(registry);
        updateCollisionBridge(registry);
        updateInputBridge();

        // Game._update reçoit le dt brut (il accumule _time = _time + dt * _timescale).
        if (m_gameUpdateFn.valid()) {
            auto r = m_gameUpdateFn(deltaTime);
            if (!r.valid()) reportScriptError(r, "update", "Game._update");
        }

        // Lire le timescale courant une fois par frame pour mettre à l'échelle tous les scripts.
        float timescale = 1.0f;
        if (m_gameGetTimescaleFn.valid()) {
            auto r = m_gameGetTimescaleFn();
            if (r.valid()) timescale = r.get<float>(0);
            else           reportScriptError(r, "update", "Game.getTimescale");
        }
        float scaledDt = deltaTime * timescale;

        updateNovaRuntime(scaledDt);  // Timer, Effect, Tween… respectent le timescale

        for (auto& gs : m_globalScripts)
            updateGlobalScript(gs, scaledDt);

        // Détection de changement de scène → charge/décharge le script de scène
        if (m_sceneManager) {
            Scene* activeScene = m_sceneManager->getActiveScene();
            if (activeScene != m_lastActiveScene) {
                m_lastActiveScene = activeScene;
                m_sceneScripts.clear();
                if (activeScene && !activeScene->getScriptPath().empty()) {
                    m_sceneScripts.push_back({ activeScene->getScriptPath(), {}, {}, false, false, 0.0f, 0.0f, true });
                    LOG_INFO("[ScriptSystem] Scene script : '{}'", activeScene->getScriptPath());
                }
            }
        }
        for (auto& ss : m_sceneScripts)
            updateGlobalScript(ss, scaledDt);

        auto entities = registry.getEntitiesWith(getRequiredComponents());
        float frameScriptMs = 0.0f;
        int   frameScriptN  = 0;
        for (auto* entity : entities) {
            auto* script = entity->getComponent<ScriptComponent>();
            if (!script || !script->enabled || script->errored) continue;

            if (!script->loaded)
                loadEntityScript(entity, script);

            if (script->errored || !script->fnUpdate.valid()) continue;

            script->m_updateAccum += scaledDt;
            if (script->updateInterval > 0.0f &&
                script->m_updateAccum < script->updateInterval) continue;

            float effectiveDt     = script->m_updateAccum;
            script->m_updateAccum = 0.0f;

            auto t0     = std::chrono::high_resolution_clock::now();
            auto result = script->fnUpdate(entity, effectiveDt);
            auto t1     = std::chrono::high_resolution_clock::now();
            float ms    = std::chrono::duration<float, std::milli>(t1 - t0).count();
            frameScriptMs += ms;
            ++frameScriptN;
            if (ms > 2.0f)
                LOG_WARN("[ScriptSystem] Script lent '{}'  {:.2f}ms", script->scriptPath, ms);

            if (!result.valid()) {
                reportScriptError(result, "update", script->scriptPath, entity->getID());
                script->errored = true;
            }
        }
        if (frameScriptMs > 5.0f)
            LOG_WARN("[ScriptSystem] Scripts : {:.2f}ms/frame ({} entités actives)", frameScriptMs, frameScriptN);

        updateAnimationBridge(registry);
    }

    // -------------------------------------------------------------------------
    // API publique
    // -------------------------------------------------------------------------
    sol::state& getLua() { return m_lua; }

    // Appelé par Game.cpp::onRender() pour dessiner les primitives debug en overlay
    void renderDebug() {
        if (!m_debugFlushFn.valid()) return;
        auto r = m_debugFlushFn(0.0f);
        if (!r.valid()) reportScriptError(r, "render", "Debug._flush");
    }

    void loadGlobalScript(const std::string& path, float updateInterval = 0.0f) {
        m_globalScripts.push_back({ path, {}, {}, false, false, updateInterval, 0.0f });
        LOG_DEBUG("[ScriptSystem] Global script enregistré : '{}'", path);
    }

    void emit(const std::string& eventName) {
        callEventBusEmit(eventName, sol::lua_nil);
    }

    void emit(const std::string& eventName, sol::table data) {
        callEventBusEmit(eventName, data);
    }

    sol::table createTable() { return m_lua.create_table(); }

    // Réinitialise un ScriptComponent (C++ API).
    // Ne nettoie PAS les handlers EventBus ni les timers scopés.
    // Depuis Lua, préférer ScriptRegistry.reload(entity.id) qui fait le nettoyage complet.
    void reloadScript(ScriptComponent* script) {
        if (!script) return;
        script->loaded        = false;
        script->errored       = false;
        script->m_updateAccum = 0.0f;
        script->env           = sol::environment();
        script->fnInit        = sol::protected_function();
        script->fnUpdate      = sol::protected_function();
    }

private:
    struct GlobalScript {
        std::string path;
        sol::environment        env;
        sol::protected_function fnUpdate;
        bool  loaded         = false;
        bool  errored        = false;
        float updateInterval = 0.0f;
        float accumulator    = 0.0f;
        bool  isSceneScript  = false;
    };

    sol::state        m_lua;
    SceneManager*     m_sceneManager  = nullptr;
    EntityRegistry*   m_registry      = nullptr;   // mis à jour à chaque update()

    // Références mises en cache après loadNovaModules() — évite les lookups chaque frame
    std::vector<sol::protected_function>    m_runtimeFns;
    sol::protected_function                 m_inputExClearFn;
    sol::protected_function                 m_debugFlushFn;
    sol::protected_function                 m_eventBusEmitFn;
    sol::protected_function                 m_gameUpdateFn;        // Game._update(rawDt)
    sol::protected_function                 m_gameGetTimescaleFn;  // Game.getTimescale()
    std::vector<GlobalScript>               m_globalScripts;
    std::vector<GlobalScript>               m_sceneScripts;
    Scene*                                  m_lastActiveScene = nullptr;
    struct U64PairHash {
        size_t operator()(const std::pair<u64,u64>& p) const noexcept {
            return std::hash<u64>{}(p.first) * 2654435761u ^ std::hash<u64>{}(p.second) * 2246822519u;
        }
    };

    std::unordered_map<u64, bool>                            m_activatorPrevState;
    std::unordered_map<std::string, bool>                    m_prevKeyState;
    std::unordered_map<std::string, bool>                    m_prevMouseState;
    std::unordered_map<u64, u32>                             m_animPrevFrame;
    std::unordered_map<u64, std::string>                     m_animPrevAnimID;
    std::unordered_set<std::pair<u64,u64>, U64PairHash>      m_prevCollisions;

    // -------------------------------------------------------------------------
    // Initialisation
    // -------------------------------------------------------------------------
    void configurePackagePath() {
        std::string cur = m_lua["package"]["path"].get_or(std::string(""));
        m_lua["package"]["path"] =
            cur + ";data/scripts/?.lua;data/scripts/?/init.lua";
    }

    // -------------------------------------------------------------------------
    // ScriptRegistry
    //   ScriptRegistry.call(entityId, "fn", ...)
    //   ScriptRegistry.sendMessage(entityId, "msg", payload)
    //   ScriptRegistry.has(entityId)
    //   ScriptRegistry.getEnv(entityId)
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

        // Appelle une fonction dans le sandbox d'une autre entité
        reg["call"] = [this](sol::this_state ts, u64 id,
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
            auto r = pf(sol::as_args(v));
            if (!r.valid()) {
                reportScriptError(r, "call", "ScriptRegistry.call:" + fn, id);
                return sol::lua_nil;
            }
            return r;
        };

        // Envoie un message à l'entité → déclenche OnMessage(msg, payload) dans son script
        reg["sendMessage"] = [this](sol::this_state ts, u64 id,
                                    const std::string& msg,
                                    sol::object payload) {
            sol::state_view lua(ts);
            sol::object bus = lua["EventBus"];
            if (!bus.valid() || bus.get_type() != sol::type::table) return;
            sol::protected_function fn = lua["EventBus"]["emit"];
            if (!fn.valid()) return;
            auto data        = lua.create_table();
            data["msg"]      = msg;
            data["payload"]  = payload;
            auto r = fn("script_message_" + std::to_string(id), data);
            if (!r.valid())
                reportScriptError(r, "sendMessage", "ScriptRegistry.sendMessage:" + msg, id);
        };

        // Hot-reload d'un script d'entité par son entityId.
        // Nettoie les handlers, annule les timers scopés, remet le script à l'état initial.
        // Utilisable depuis Lua : ScriptRegistry.reload(entity.id)
        reg["reload"] = [this](sol::this_state ts, u64 entityId) -> bool {
            if (!m_registry) return false;
            sol::state_view lua(ts);

            // Trouver le ScriptComponent
            ScriptComponent* script = nullptr;
            for (auto* e : m_registry->getEntitiesWith({ScriptComponent::staticTypeID()})) {
                if (e->getID() == entityId) { script = e->getComponent<ScriptComponent>(); break; }
            }
            if (!script) return false;

            // Nettoyer les handlers trackés
            sol::protected_function cleanFn = lua["__cleanEntityHandlers"];
            if (cleanFn.valid()) {
                auto r = cleanFn(entityId);
                if (!r.valid()) reportScriptError(r, "reload", "__cleanEntityHandlers", entityId);
            }

            // Annuler les timers/coroutines scopés à cette entité
            std::string scope = std::to_string(entityId);
            sol::table timer = lua["Timer"];
            if (timer.valid()) {
                sol::protected_function f = timer["cancelScope"];
                if (f.valid()) {
                    auto r = f(scope);
                    if (!r.valid()) reportScriptError(r, "reload", "Timer.cancelScope", entityId);
                }
            }
            sol::table sched = lua["Scheduler"];
            if (sched.valid()) {
                sol::protected_function f = sched["cancelScope"];
                if (f.valid()) {
                    auto r = f(scope);
                    if (!r.valid()) reportScriptError(r, "reload", "Scheduler.cancelScope", entityId);
                }
            }

            // Retirer de ScriptRegistry
            sol::table envs = lua["ScriptRegistry"]["_envs"];
            envs[entityId]  = sol::lua_nil;

            // Remettre le ScriptComponent à l'état initial — rechargé au prochain update
            script->loaded        = false;
            script->errored       = false;
            script->m_updateAccum = 0.0f;
            script->env           = sol::environment();
            script->fnInit        = sol::protected_function();
            script->fnUpdate      = sol::protected_function();

            LOG_INFO("[ScriptSystem] Hot-reload : '{}'", script->scriptPath);
            return true;
        };

        // Hot-reload de tous les scripts en erreur.
        // Retourne le nombre de scripts rechargés.
        reg["reloadAll"] = [this](sol::this_state ts) -> int {
            if (!m_registry) return 0;
            sol::state_view lua(ts);
            sol::protected_function cleanFn = lua["__cleanEntityHandlers"];
            sol::table timer = lua["Timer"];
            sol::table sched = lua["Scheduler"];

            int count = 0;
            for (auto* e : m_registry->getEntitiesWith({ScriptComponent::staticTypeID()})) {
                auto* script = e->getComponent<ScriptComponent>();
                if (!script || !script->errored) continue;
                u64 id = e->getID();

                if (cleanFn.valid()) {
                    auto r = cleanFn(id);
                    if (!r.valid()) reportScriptError(r, "reloadAll", "__cleanEntityHandlers", id);
                }
                std::string scope = std::to_string(id);
                if (timer.valid()) {
                    sol::protected_function f = timer["cancelScope"];
                    if (f.valid()) {
                        auto r = f(scope);
                        if (!r.valid()) reportScriptError(r, "reloadAll", "Timer.cancelScope", id);
                    }
                }
                if (sched.valid()) {
                    sol::protected_function f = sched["cancelScope"];
                    if (f.valid()) {
                        auto r = f(scope);
                        if (!r.valid()) reportScriptError(r, "reloadAll", "Scheduler.cancelScope", id);
                    }
                }

                sol::table envs = lua["ScriptRegistry"]["_envs"];
                envs[id] = sol::lua_nil;

                script->loaded        = false;
                script->errored       = false;
                script->m_updateAccum = 0.0f;
                script->env           = sol::environment();
                script->fnInit        = sol::protected_function();
                script->fnUpdate      = sol::protected_function();
                ++count;
            }
            if (count > 0)
                LOG_INFO("[ScriptSystem] Hot-reload : {} script(s) rechargés.", count);
            return count;
        };

        m_lua["ScriptRegistry"] = reg;

        // Nettoie les maps C++ indexées par entityId à la destruction d'une entité.
        // Appelé depuis _wire_handlers.lua cleanupFn via __cleanBridgeMaps(entityId).
        m_lua["__cleanBridgeMaps"] = [this](u64 entityId) {
            m_animPrevFrame.erase(entityId);
            m_animPrevAnimID.erase(entityId);
            m_activatorPrevState.erase(entityId);
        };
    }

    // -------------------------------------------------------------------------
    // Named handlers wirer — chargé depuis data/scripts/nova/_wire_handlers.lua
    // Définit __wireNamedHandlers(env, entityId, isSceneScript) et
    // __cleanEntityHandlers(entityId) comme globaux Lua.
    // -------------------------------------------------------------------------
    void initNamedHandlersWirer() {
        auto r = m_lua.safe_script_file("data/scripts/nova/_wire_handlers.lua",
                                        sol::script_pass_on_error);
        if (!r.valid()) reportScriptError(r, "load", "nova/_wire_handlers.lua");
    }

    void loadNovaModules() {
        const std::pair<const char*, const char*> modules[] = {
            { "EventBus",      "nova/event_bus"     },
            { "Timer",         "nova/timer"         },
            { "Scheduler",     "nova/scheduler"     },
            { "StateMachine",  "nova/state_machine" },
            { "Vec2",          "nova/vec2"          },
            { "World",         "nova/world"         },
            { "Tween",         "nova/tween"         },
            { "Class",         "nova/class"         },
            { "Quest",         "nova/quest"         },
            { "Persist",       "nova/persist"       },
            { "Game",          "nova/game"          },
            { "Stats",         "nova/stats"         },
            { "Stat",          "nova/stat_names"    },
            { "Camera",        "nova/camera"        },
            { "InputEx",       "nova/input_ext"     },
            { "Effect",        "nova/effect"        },
            { "Cooldown",      "nova/cooldown"      },
            { "Sound",         "nova/sound"         },
            { "Inventory",     "nova/inventory"     },
            { "Notify",        "nova/notify"        },
            { "Physics",       "nova/physics"       },
            { "Flag",          "nova/flag"          },
            // Utilitaires purs Lua
            { "Math",          "nova/math_ext"      },
            { "Table",         "nova/table_ext"     },
            { "Random",        "nova/random"        },
            // Étend le type Color C++ avec lerp/fromHex/fromHSV/etc. (retourne Color)
            { "Color",         "nova/color_ext"     },
            // Modules haut niveau
            { "Sequence",      "nova/sequence"      },
            { "Conversation",  "nova/conversation"  },
            { "Anim",          "nova/anim"          },
            { "Trigger",       "nova/trigger"       },
            { "InputBind",     "nova/input_bind"    },
            { "Loot",          "nova/loot"          },
            { "Nav",           "nova/nav"           },
            // Outils de données et debug
            { "Data",          "nova/data"          },
            { "Debug",         "nova/debug"         },
            { "Spatial",       "nova/spatial"       },
            { "Pool",          "nova/pool"          },
            { "Easing",        "nova/easing"        },
            { "I18n",          "nova/i18n"          },
            { "Achievement",   "nova/achievement"   },
            // Effets visuels écran (fondu, flash) — doit précéder Scene
            { "SceneFX",       "nova/scene_fx"      },
            // Système de particules 2D via DebugDraw
            { "Particles",     "nova/particles"     },
            // Projectiles via Pool — doit être chargé après Pool
            { "Projectile",    "nova/projectile"    },
            // Persistance d'état des entités pré-placées — doit être chargé après Persist et EventBus
            { "EntityState",   "nova/entity_state"  },
            // Override le global Scene C++ avec le wrapper Lua (capture _sm = Scene avant)
            // Doit être chargé après Nav (qui utilise Scene.findPath) et après SceneFX
            { "Scene",         "nova/scene"         },
            // Behavior Trees pour l'IA — Sequence, Selector, Parallel, Action, Condition
            { "BT",            "nova/behavior_tree" },
        };
        for (auto& [global, mod] : modules) {
            std::string code = std::string(global) + " = require('" + mod + "')";
            auto result = m_lua.safe_script(code, sol::script_pass_on_error);
            if (!result.valid()) reportScriptError(result, "load", mod);
        }
        // Toujours recacher après chargement — sûr à rappeler si les modules sont rechargés
        cacheRuntimeUpdates();
    }

    // -------------------------------------------------------------------------
    // Runtime nova
    // -------------------------------------------------------------------------
    // Met en cache les références aux méthodes update des modules nova.
    // Appelé une fois après loadNovaModules() — évite les lookups Lua chaque frame.
    // -------------------------------------------------------------------------
    void cacheRuntimeUpdates() {
        // Game._update est exclu : il reçoit le dt brut (il gère _timescale en interne).
        // Tous les autres modules reçoivent le dt mis à l'échelle via updateNovaRuntime().
        static constexpr std::pair<const char*, const char*> s_list[] = {
            { "Timer",      "update"  }, { "Scheduler",  "update"  },
            { "Tween",      "update"  },
            { "Camera",     "_update" }, { "World",      "_update" },
            { "Effect",     "update"  }, { "Cooldown",   "update"  },
            { "Sequence",   "_update" }, { "Anim",       "_update" },
            { "Trigger",    "_update" }, { "Nav",        "_update" },
            { "Spatial",    "_update" }, { "SceneFX",    "_update" },
            { "Particles",  "_update" }, { "Projectile", "_update" },
            { "Debug",      "_update" },
        };
        m_runtimeFns.clear();
        for (auto& [tbl, mtd] : s_list) {
            sol::object t = m_lua[tbl];
            if (!t.valid() || t.get_type() != sol::type::table) continue;
            sol::protected_function fn = m_lua[tbl][mtd];
            if (fn.valid()) m_runtimeFns.push_back(std::move(fn));
        }
        // Game._update reçoit le dt brut — mis en cache séparément
        sol::object game = m_lua["Game"];
        if (game.valid() && game.get_type() == sol::type::table) {
            sol::protected_function fu = m_lua["Game"]["_update"];
            if (fu.valid()) m_gameUpdateFn = std::move(fu);
            sol::protected_function ft = m_lua["Game"]["getTimescale"];
            if (ft.valid()) m_gameGetTimescaleFn = std::move(ft);
        }
        // Cache séparé pour InputEx._clear (appelé avant les bridges) et Debug._flush (render)
        sol::object iex = m_lua["InputEx"];
        if (iex.valid() && iex.get_type() == sol::type::table) {
            sol::protected_function f = m_lua["InputEx"]["_clear"];
            if (f.valid()) m_inputExClearFn = std::move(f);
        }
        sol::object dbg = m_lua["Debug"];
        if (dbg.valid() && dbg.get_type() == sol::type::table) {
            sol::protected_function f = m_lua["Debug"]["_flush"];
            if (f.valid()) m_debugFlushFn = std::move(f);
        }
        // EventBus.emit — appelé à chaque bridge event, chaque frame
        sol::object bus = m_lua["EventBus"];
        if (bus.valid() && bus.get_type() == sol::type::table) {
            sol::protected_function f = m_lua["EventBus"]["emit"];
            if (f.valid()) m_eventBusEmitFn = std::move(f);
        }
    }

    void updateNovaRuntime(float dt) {
        for (auto& fn : m_runtimeFns) {
            auto r = fn(dt);
            if (!r.valid()) reportScriptError(r, "update", "nova/runtime");
        }
    }

    // -------------------------------------------------------------------------
    // Bridge ColliderComponent → EventBus (AABB + circle, enter/exit)
    // Émet par entité : collision_enter_<id> / collision_exit_<id>  { entityId }
    // Émet global    : collision_enter / collision_exit  { entityIdA, entityIdB }
    // -------------------------------------------------------------------------
    static bool s_collidersOverlap(
        const TransformComponent* ta, const ColliderComponent* ca,
        const TransformComponent* tb, const ColliderComponent* cb)
    {
        using CT = ColliderComponent::ColliderType;
        bool aBox = (ca->type == CT::Box);
        bool bBox = (cb->type == CT::Box);

        if (aBox && bBox) {
            Vec2f a = ta->position + ca->offset;
            Vec2f b = tb->position + cb->offset;
            return a.x < b.x + cb->size.x && a.x + ca->size.x > b.x &&
                   a.y < b.y + cb->size.y && a.y + ca->size.y > b.y;
        }
        if (!aBox && !bBox) {
            Vec2f d = (ta->position + ca->offset) - (tb->position + cb->offset);
            float r = ca->radius + cb->radius;
            return d.x*d.x + d.y*d.y < r * r;
        }
        // Box vs Circle
        const TransformComponent* tBox  = aBox ? ta : tb;
        const ColliderComponent*  cBox  = aBox ? ca : cb;
        const TransformComponent* tCirc = aBox ? tb : ta;
        const ColliderComponent*  cCirc = aBox ? cb : ca;
        Vec2f mn  = tBox->position  + cBox->offset;
        Vec2f mx  = { mn.x + cBox->size.x, mn.y + cBox->size.y };
        Vec2f ctr = tCirc->position + cCirc->offset;
        Vec2f cl  = { std::max(mn.x, std::min(ctr.x, mx.x)),
                      std::max(mn.y, std::min(ctr.y, mx.y)) };
        Vec2f d   = ctr - cl;
        return d.x*d.x + d.y*d.y < cCirc->radius * cCirc->radius;
    }

    void updateCollisionBridge(EntityRegistry& registry) {
        auto entities = registry.getEntitiesWith({"TransformComponent", "ColliderComponent"});

        // ── Broadphase spatial hash ──────────────────────────────────────────
        // Chaque entité est insérée dans toutes les cellules que son AABB recouvre.
        // Seules les paires partageant une cellule sont ensuite testées en narrowphase.
        // Complexity: O(n * k) où k = nb cellules/entité (typiquement 1-4 pour 256px).
        struct Cell { int x, y; bool operator==(const Cell& o) const noexcept { return x==o.x && y==o.y; } };
        struct CellHash {
            size_t operator()(const Cell& c) const noexcept {
                return std::hash<int>{}(c.x) * 2654435761u ^ std::hash<int>{}(c.y) * 2246822519u;
            }
        };
        struct IdxPairHash {
            size_t operator()(const std::pair<size_t,size_t>& p) const noexcept {
                return std::hash<size_t>{}(p.first) ^ (std::hash<size_t>{}(p.second) * 2654435761u);
            }
        };
        constexpr float CELL = 256.0f;

        std::unordered_map<Cell, std::vector<size_t>, CellHash> grid;
        grid.reserve(entities.size() * 2);

        for (size_t i = 0; i < entities.size(); ++i) {
            auto* c = entities[i]->getComponent<ColliderComponent>();
            if (!c->enabled) continue;
            auto* t = entities[i]->getComponent<TransformComponent>();
            float minX, minY, maxX, maxY;
            if (c->type == ColliderComponent::ColliderType::Box) {
                minX = t->position.x + c->offset.x;       minY = t->position.y + c->offset.y;
                maxX = minX + c->size.x;                   maxY = minY + c->size.y;
            } else {
                float cx = t->position.x + c->offset.x,   cy = t->position.y + c->offset.y;
                minX = cx - c->radius; minY = cy - c->radius;
                maxX = cx + c->radius; maxY = cy + c->radius;
            }
            for (int gx = (int)std::floor(minX/CELL); gx <= (int)std::floor(maxX/CELL); ++gx)
                for (int gy = (int)std::floor(minY/CELL); gy <= (int)std::floor(maxY/CELL); ++gy)
                    grid[{gx,gy}].push_back(i);
        }

        // ── Narrowphase ──────────────────────────────────────────────────────
        std::unordered_set<std::pair<size_t,size_t>, IdxPairHash> checked;
        std::unordered_set<std::pair<u64,u64>, U64PairHash> current;

        for (auto& [cell, indices] : grid) {
            for (size_t k = 0; k < indices.size(); ++k) {
                for (size_t l = k + 1; l < indices.size(); ++l) {
                    size_t i = indices[k], j = indices[l];
                    auto key = (i < j) ? std::make_pair(i,j) : std::make_pair(j,i);
                    if (!checked.insert(key).second) continue;

                    auto* ta = entities[i]->getComponent<TransformComponent>();
                    auto* ca = entities[i]->getComponent<ColliderComponent>();
                    auto* tb = entities[j]->getComponent<TransformComponent>();
                    auto* cb = entities[j]->getComponent<ColliderComponent>();
                    if (!ca->enabled || !cb->enabled) continue;
                    if (!s_collidersOverlap(ta, ca, tb, cb)) continue;

                    u64 a = entities[i]->getID(), b = entities[j]->getID();
                    if (a > b) std::swap(a, b);
                    current.insert({a, b});
                }
            }
        }

        auto emitPair = [&](const char* event, u64 a, u64 b) {
            auto dA = m_lua.create_table();
            dA["entityId"]  = b; dA["entityIdA"] = a; dA["entityIdB"] = b;
            callEventBusEmit(std::string(event) + "_" + std::to_string(a), dA);
            auto dB = m_lua.create_table();
            dB["entityId"]  = a; dB["entityIdA"] = a; dB["entityIdB"] = b;
            callEventBusEmit(std::string(event) + "_" + std::to_string(b), dB);
            callEventBusEmit(event, dA);
        };

        for (auto& [a, b] : current)
            if (!m_prevCollisions.count({a, b})) emitPair("collision_enter", a, b);
        for (auto& [a, b] : m_prevCollisions)
            if (!current.count({a, b}))           emitPair("collision_exit",  a, b);

        m_prevCollisions = std::move(current);
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
    // Émet deux événements par touche :
    //   key_down_E  (spécifique)   EventBus.on("key_down_E", fn)
    //   key_down    (générique)    function OnKeyDown(key) ... end
    // -------------------------------------------------------------------------
    void updateInputBridge() {
        const auto& keyMap = LuaBindings::getKeyMap();
        for (const auto& [name, code] : keyMap) {
            bool isNow   = INPUT().isKeyPressed(code);
            bool wasPrev = m_prevKeyState[name];
            if (isNow == wasPrev) continue;

            m_prevKeyState[name] = isNow;
            auto data   = m_lua.create_table();
            data["key"] = name;
            // Événement spécifique : key_down_E
            callEventBusEmit(isNow ? ("key_down_" + name) : ("key_up_" + name), data);
            // Événement générique : key_down / key_up  →  OnKeyDown(key)
            callEventBusEmit(isNow ? std::string("key_down") : std::string("key_up"), data);
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
            auto pos       = INPUT().getMousePosition();
            auto data      = m_lua.create_table();
            data["button"] = name;
            data["x"]      = pos.x;
            data["y"]      = pos.y;
            // Spécifique : mouse_down_left
            callEventBusEmit(isNow ? (std::string("mouse_down_") + name)
                                   : (std::string("mouse_up_")   + name), data);
            // Générique : mouse_down / mouse_up  →  OnMouseDown(button, x, y)
            callEventBusEmit(isNow ? std::string("mouse_down") : std::string("mouse_up"), data);
        }
    }

    // -------------------------------------------------------------------------
    // Bridge AnimationComponent → EventBus
    // -------------------------------------------------------------------------
    void updateAnimationBridge(EntityRegistry& registry) {
        auto animated = registry.getEntitiesWith({ AnimationComponent::staticTypeID() });
        for (auto* entity : animated) {
            auto* anim = entity->getComponent<AnimationComponent>();
            if (!anim) continue;
            u64 id = entity->getID();

            auto prevIDIt = m_animPrevAnimID.find(id);
            const std::string& prevID = (prevIDIt != m_animPrevAnimID.end())
                ? prevIDIt->second : std::string("");
            if (anim->animationID != prevID) {
                m_animPrevAnimID[id] = anim->animationID;
                auto data           = m_lua.create_table();
                data["entityId"]    = id;
                data["animationID"] = anim->animationID;
                data["previousID"]  = prevID;
                callEventBusEmit("animation_changed", data);
            }

            auto prevFrIt = m_animPrevFrame.find(id);
            u32 prevFrame = (prevFrIt != m_animPrevFrame.end()) ? prevFrIt->second : UINT32_MAX;
            if (anim->currentFrame != prevFrame) {
                m_animPrevFrame[id]  = anim->currentFrame;
                auto data           = m_lua.create_table();
                data["entityId"]    = id;
                data["frame"]       = (int)anim->currentFrame;
                data["animationID"] = anim->animationID;
                callEventBusEmit("animation_frame", data);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Chargement script entité
    // -------------------------------------------------------------------------
    void loadEntityScript(Entity* entity, ScriptComponent* script) {
        if (script->scriptPath.empty()) {
            LOG_WARN("[ScriptSystem] Entity {} : scriptPath vide", entity->getID());
            script->errored = true;
            return;
        }
        try {
            script->env = sol::environment(m_lua, sol::create, m_lua.globals());

            // Injecte les properties JSON en `self`
            auto self = m_lua.create_table();
            for (auto& [key, val] : script->properties.items()) {
                if      (val.is_string())         self[key] = val.get<std::string>();
                else if (val.is_number_float())   self[key] = val.get<double>();
                else if (val.is_number_integer()) self[key] = val.get<int64_t>();
                else if (val.is_boolean())        self[key] = val.get<bool>();
            }
            script->env["self"] = self;

            // `entity` global — accessible depuis TOUTES les fonctions du script
            // (init, update, OnKeyDown, OnMessage, etc.) sans le passer en paramètre.
            // C'est l'équivalent de `self` qui est l'ObjectReference en Papyrus.
            script->env.set("entity", entity);

            // Sandbox per-env : masque load/loadfile/dofile dans les scripts jeu.
            // Les modules nova (ex: Persist) conservent l'accès via l'état global.
            script->env["load"]     = sol::lua_nil;
            script->env["loadfile"] = sol::lua_nil;
            script->env["dofile"]   = sol::lua_nil;

            // RegisterForUpdate(interval) / UnregisterForUpdate() / ResumeUpdate()
            // Permettent au script de contrôler sa propre fréquence d'update
            script->env["RegisterForUpdate"] = [script](float interval) {
                script->updateInterval = std::max(interval, 0.0f);
                script->m_updateAccum  = 0.0f;
            };
            script->env["UnregisterForUpdate"] = [script]() {
                script->updateInterval = 1e30f;   // n'update plus jamais
            };
            script->env["ResumeUpdate"] = [script]() {
                script->updateInterval = 0.0f;    // reprend chaque frame
                script->m_updateAccum  = 0.0f;
            };

            auto res = m_lua.script_file(script->scriptPath, script->env,
                                         sol::script_pass_on_error);
            if (!res.valid()) {
                reportScriptError(res, "load", script->scriptPath, entity->getID());
                script->errored = true;
                return;
            }
            script->fnInit   = script->env["init"];
            script->fnUpdate = script->env["update"];
            script->loaded   = true;
            LOG_DEBUG("[ScriptSystem] Chargé '{}'", script->scriptPath);

            // Enregistre dans ScriptRegistry
            sol::object reg = m_lua["ScriptRegistry"];
            if (reg.valid() && reg.get_type() == sol::type::table) {
                sol::table envs = m_lua["ScriptRegistry"]["_envs"];
                envs[entity->getID()] = script->env;
            }

            // Branche automatiquement les named handlers (OnActivate, OnKeyDown, etc.)
            sol::protected_function wire = m_lua["__wireNamedHandlers"];
            if (wire.valid()) {
                auto wr = wire(script->env, entity->getID());
                if (!wr.valid())
                    reportScriptError(wr, "wireHandlers", script->scriptPath, entity->getID());
            }

            if (script->fnInit.valid()) {
                auto r = script->fnInit(entity);
                if (!r.valid()) {
                    reportScriptError(r, "init", script->scriptPath, entity->getID());
                    script->errored = true;
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[ScriptSystem] Exception '{}' : {}", script->scriptPath, e.what());
            script->errored = true;
        }
    }

    // -------------------------------------------------------------------------
    // Scripts globaux
    // -------------------------------------------------------------------------
    void loadGlobalScriptImpl(GlobalScript& gs) {
        try {
            gs.env = sol::environment(m_lua, sol::create, m_lua.globals());
            // Sandbox per-env : masque load/loadfile/dofile dans les scripts jeu.
            gs.env["load"]     = sol::lua_nil;
            gs.env["loadfile"] = sol::lua_nil;
            gs.env["dofile"]   = sol::lua_nil;
            auto res = m_lua.script_file(gs.path, gs.env, sol::script_pass_on_error);
            if (!res.valid()) {
                reportScriptError(res, "load", gs.path);
                gs.errored = true;
                return;
            }
            gs.fnUpdate = gs.env["update"];
            gs.loaded   = true;
            LOG_DEBUG("[ScriptSystem] Global chargé '{}'", gs.path);

            // Wire les named handlers (entityId=0, isSceneScript distingue scène vs global)
            sol::protected_function wire = m_lua["__wireNamedHandlers"];
            if (wire.valid()) {
                auto wr = wire(gs.env, u64{0}, gs.isSceneScript);
                if (!wr.valid()) reportScriptError(wr, "wireHandlers", gs.path);
            }

            sol::protected_function fnInit = gs.env["init"];
            if (fnInit.valid()) {
                auto r = fnInit();
                if (!r.valid()) {
                    reportScriptError(r, "init", gs.path);
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
            reportScriptError(r, "update", gs.path);
            gs.errored = true;
        }
    }

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    // Garde-fou contre la récursion : si reportScriptError échoue à émettre
    // "script_error" via EventBus, on ne tente pas de re-rapporter cette erreur.
    bool m_reportingScriptError = false;

    // Rapport unifié d'erreur Lua : LOG_ERROR + propagation EventBus("script_error").
    // À utiliser pour tout sol::protected_function_result invalide. Garantit que
    // les erreurs ne soient jamais silencieusement avalées (cf. bug DevConsole).
    //
    // phase    : "load", "init", "update", "render", "wireHandlers", "call", etc.
    // context  : chemin du script, nom du module ou de la fonction concernée
    // entityId : id de l'entité associée (0 si script global / scène / interne)
    template<typename Result>
    void reportScriptError(Result& result,
                           const char* phase,
                           const std::string& context,
                           u64 entityId = 0)
    {
        sol::error err = result;
        LOG_ERROR("[ScriptSystem] {} '{}' : {}", phase, context, err.what());

        if (!m_eventBusEmitFn.valid() || m_reportingScriptError) return;
        m_reportingScriptError = true;
        auto ed = m_lua.create_table();
        ed["entityId"] = entityId;
        ed["path"]     = context;
        ed["error"]    = std::string(err.what());
        ed["phase"]    = phase;
        auto er = m_eventBusEmitFn("script_error", ed);
        if (!er.valid()) {
            sol::error e2 = er;
            LOG_ERROR("[ScriptSystem] EventBus.emit('script_error') : {}", e2.what());
        }
        m_reportingScriptError = false;
    }

    void callEventBusEmit(const std::string& eventName, sol::object data) {
        if (!m_eventBusEmitFn.valid()) return;
        auto r = m_eventBusEmitFn(eventName, data);
        if (!r.valid()) {
            sol::error err = r;
            LOG_ERROR("[ScriptSystem] EventBus.emit('{}') : {}", eventName, err.what());
        }
    }
};

} // namespace NovaEngine
