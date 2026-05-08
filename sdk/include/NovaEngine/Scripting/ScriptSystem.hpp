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
#include <set>
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
//   Trigger, InputBind, Loot, Nav
//
// COUCHE 3 — Scripts jeu (data/scripts/) :
//   Scripts entité : init(entity) + update(entity, dt)
//   Scripts globaux : init() + update(dt) via loadGlobalScript()
//
// FONCTIONNALITÉS PAPYRUS :
//
//   NAMED EVENT HANDLERS — auto-branchés sans EventBus.on() manuel :
//     function OnActivate(activatorId, actionID)   end
//     function OnDeactivate(activatorId, actionID) end
//     function OnKeyDown(key)                      end
//     function OnKeyUp(key)                        end
//     function OnMouseDown(button, x, y)           end
//     function OnAnimationEvent(animID, frame)     end
//     function OnAnimationChanged(newID, prevID)   end
//     function OnMessage(msg, payload)             end
//     function OnQuestComplete(questId)            end
//     function OnQuestAdvanced(questId, stage)     end
//
//   CONTROL D'UPDATE DEPUIS LE SCRIPT :
//     RegisterForUpdate(0.5)   -- change la fréquence d'update à 0.5s
//     UnregisterForUpdate()    -- suspend les updates (sans désactiver init)
//     ResumeUpdate()           -- reprend les updates
//
//   INTER-SCRIPT :
//     ScriptRegistry.call(entityId, "fn", arg1, arg2)
//     ScriptRegistry.sendMessage(entityId, "msg", payload)
//     entity:sendMessage("msg", payload)  -- équivalent sur l'entité directe
//
//   GLOBALS DISPONIBLES :
//     EventBus, Timer, Scheduler, StateMachine, Vec2, World
//     Tween, Class, Quest, Persist, Game, Stats
//     ScriptRegistry
//     Registry, Input, Log, print, Audio, Resources, Scene
//     dialogueActive
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
            sol::lib::package
        );

        std::filesystem::create_directories("data/saves");

        configurePackagePath();
        LuaBindings::registerAll(m_lua);
        if (m_sceneManager)
            LuaBindings::registerSceneManager(m_lua, *m_sceneManager);

        initScriptRegistry();
        initNamedHandlersWirer();
        loadNovaModules();
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
        m_lua["Registry"] = &registry;

        // Réinitialise les états "just pressed/released" avant la détection
        callTableMethod("InputEx", "_clear", 0.0f);

        updateActivatorBridge(registry);
        updateCollisionBridge(registry);
        updateInputBridge();
        updateNovaRuntime(deltaTime);

        for (auto& gs : m_globalScripts)
            updateGlobalScript(gs, deltaTime);

        // Détection de changement de scène → charge/décharge le script de scène
        if (m_sceneManager) {
            Scene* activeScene = m_sceneManager->getActiveScene();
            if (activeScene != m_lastActiveScene) {
                m_lastActiveScene = activeScene;
                m_sceneScripts.clear();
                if (activeScene && !activeScene->getScriptPath().empty()) {
                    m_sceneScripts.push_back({ activeScene->getScriptPath(), {}, {}, false, false, 0.0f, 0.0f });
                    LOG_INFO("[ScriptSystem] Scene script : '{}'", activeScene->getScriptPath());
                }
            }
        }
        for (auto& ss : m_sceneScripts)
            updateGlobalScript(ss, deltaTime);

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

            auto t0     = std::chrono::high_resolution_clock::now();
            auto result = script->fnUpdate(entity, effectiveDt);
            auto t1     = std::chrono::high_resolution_clock::now();
            float ms    = std::chrono::duration<float, std::milli>(t1 - t0).count();
            if (ms > 2.0f)
                LOG_WARN("[ScriptSystem] Script lent '{}'  {:.2f}ms", script->scriptPath, ms);

            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("[ScriptSystem] update '{}': {}", script->scriptPath, err.what());
                script->errored = true;
            }
        }

        updateAnimationBridge(registry);
    }

    // -------------------------------------------------------------------------
    // API publique
    // -------------------------------------------------------------------------
    sol::state& getLua() { return m_lua; }

    // Appelé par Game.cpp::onRender() pour dessiner les primitives debug en overlay
    void renderDebug() {
        callTableMethod("Debug", "_flush", 0.0f);
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

    void reloadScript(ScriptComponent* script) {
        if (!script) return;
        script->loaded        = false;
        script->errored       = false;
        script->m_updateAccum = 0.0f;
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
    };

    sol::state    m_lua;
    SceneManager* m_sceneManager = nullptr;
    std::vector<GlobalScript>               m_globalScripts;
    std::vector<GlobalScript>               m_sceneScripts;
    Scene*                                  m_lastActiveScene = nullptr;
    std::unordered_map<u64, bool>           m_activatorPrevState;
    std::unordered_map<std::string, bool>   m_prevKeyState;
    std::unordered_map<std::string, bool>   m_prevMouseState;
    std::unordered_map<u64, u32>            m_animPrevFrame;
    std::unordered_map<u64, std::string>    m_animPrevAnimID;
    std::set<std::pair<u64,u64>>            m_prevCollisions;

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

        // Envoie un message à l'entité → déclenche OnMessage(msg, payload) dans son script
        reg["sendMessage"] = [](sol::this_state ts, u64 id,
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
            fn("script_message_" + std::to_string(id), data);
        };

        m_lua["ScriptRegistry"] = reg;
    }

    // -------------------------------------------------------------------------
    // Named handlers wirer — __wireNamedHandlers(env, entityId)
    //
    // Branche automatiquement les fonctions "magiques" d'un script
    // sans que le scripter ait à écrire EventBus.on() manuellement.
    // Inspiré de l'auto-wiring des Events Papyrus (OnActivate, OnHit...).
    // -------------------------------------------------------------------------
    void initNamedHandlersWirer() {
        auto r = m_lua.safe_script(R"(
function __wireNamedHandlers(env, entityId)
    local function safe(fn, ...)
        local ok, err = pcall(fn, ...)
        if not ok then Log.error("[AutoWire] " .. tostring(err)) end
    end

    -- OnActivate(activatorId, actionID) / OnDeactivate(...)
    if type(env.OnActivate) == "function" then
        EventBus.on("activator_on", function(data)
            if data.entityId == entityId then
                safe(env.OnActivate, data.entityId, data.actionID)
            end
        end)
    end
    if type(env.OnDeactivate) == "function" then
        EventBus.on("activator_off", function(data)
            if data.entityId == entityId then
                safe(env.OnDeactivate, data.entityId, data.actionID)
            end
        end)
    end

    -- OnKeyDown(key) / OnKeyUp(key)
    -- Appelé pour TOUTES les touches — filtrer dans le handler si besoin
    if type(env.OnKeyDown) == "function" then
        EventBus.on("key_down", function(data) safe(env.OnKeyDown, data.key) end)
    end
    if type(env.OnKeyUp) == "function" then
        EventBus.on("key_up",   function(data) safe(env.OnKeyUp,   data.key) end)
    end

    -- OnMouseDown(button, x, y) / OnMouseUp(button, x, y)
    if type(env.OnMouseDown) == "function" then
        EventBus.on("mouse_down", function(data)
            safe(env.OnMouseDown, data.button, data.x, data.y)
        end)
    end
    if type(env.OnMouseUp) == "function" then
        EventBus.on("mouse_up", function(data)
            safe(env.OnMouseUp, data.button, data.x, data.y)
        end)
    end

    -- OnAnimationEvent(animationID, frame)
    if type(env.OnAnimationEvent) == "function" then
        EventBus.on("animation_frame", function(data)
            if data.entityId == entityId then
                safe(env.OnAnimationEvent, data.animationID, data.frame)
            end
        end)
    end

    -- OnAnimationChanged(newAnimID, previousID)
    if type(env.OnAnimationChanged) == "function" then
        EventBus.on("animation_changed", function(data)
            if data.entityId == entityId then
                safe(env.OnAnimationChanged, data.animationID, data.previousID)
            end
        end)
    end

    -- OnMessage(msg, payload) — via entity:sendMessage() ou ScriptRegistry.sendMessage()
    if type(env.OnMessage) == "function" then
        EventBus.on("script_message_" .. entityId, function(data)
            safe(env.OnMessage, data.msg, data.payload)
        end)
    end

    -- OnQuestComplete(questId)
    if type(env.OnQuestComplete) == "function" then
        EventBus.on("quest_completed", function(data)
            safe(env.OnQuestComplete, data.questId)
        end)
    end

    -- OnQuestAdvanced(questId, stageIndex)
    if type(env.OnQuestAdvanced) == "function" then
        EventBus.on("quest_advanced", function(data)
            safe(env.OnQuestAdvanced, data.questId, data.stageIndex)
        end)
    end

    -- OnStatZeroed(entityId, stat) — émis par Stats quand une stat atteint 0
    if type(env.OnStatZeroed) == "function" then
        EventBus.on("stat_zeroed", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnStatZeroed, data.entityId, data.stat)
            end
        end)
    end

    -- OnItemAdded(item, count) / OnItemRemoved(item, count)
    if type(env.OnItemAdded) == "function" then
        EventBus.on("item_added", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnItemAdded, data.item, data.count)
            end
        end)
    end
    if type(env.OnItemRemoved) == "function" then
        EventBus.on("item_removed", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnItemRemoved, data.item, data.count)
            end
        end)
    end

    -- OnEffectApplied(effectId, entityId) / OnEffectExpired(effectId, entityId)
    if type(env.OnEffectApplied) == "function" then
        EventBus.on("effect_applied", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnEffectApplied, data.effectId, data.entityId)
            end
        end)
    end
    if type(env.OnEffectExpired) == "function" then
        EventBus.on("effect_expired", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnEffectExpired, data.effectId, data.entityId)
            end
        end)
    end

    -- OnFlagSet(name) / OnFlagUnset(name)
    if type(env.OnFlagSet) == "function" then
        EventBus.on("flag_set",   function(data) safe(env.OnFlagSet,   data.name) end)
    end
    if type(env.OnFlagUnset) == "function" then
        EventBus.on("flag_unset", function(data) safe(env.OnFlagUnset, data.name) end)
    end

    -- OnTriggerEnter(triggerId, entityId) / OnTriggerExit(triggerId, entityId)
    if type(env.OnTriggerEnter) == "function" then
        EventBus.on("trigger_enter", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnTriggerEnter, data.id, data.entityId)
            end
        end)
    end
    if type(env.OnTriggerExit) == "function" then
        EventBus.on("trigger_exit", function(data)
            if entityId == 0 or data.entityId == entityId then
                safe(env.OnTriggerExit, data.id, data.entityId)
            end
        end)
    end

    -- OnSceneChanged(name) — émis lors des changements de scène
    if type(env.OnSceneChanged) == "function" then
        EventBus.on("scene_changed", function(data) safe(env.OnSceneChanged, data.name) end)
    end

    -- OnConversationNode(nodeId, speaker, text)
    if type(env.OnConversationNode) == "function" then
        EventBus.on("conversation_node", function(data)
            safe(env.OnConversationNode, data.nodeId, data.speaker, data.text)
        end)
    end

    -- OnConversationEnd(conversationId)
    if type(env.OnConversationEnd) == "function" then
        EventBus.on("conversation_end", function(data)
            safe(env.OnConversationEnd, data.id)
        end)
    end

    -- OnUIAction(action, value, componentId) — toute action UI (clic bouton, etc.)
    if type(env.OnUIAction) == "function" then
        EventBus.on("ui_action", function(data)
            safe(env.OnUIAction, data.action, data.value, data.id)
        end)
    end

    -- OnCollisionEnter(otherEntityId) / OnCollisionExit(otherEntityId)
    if type(env.OnCollisionEnter) == "function" then
        EventBus.on("collision_enter_" .. tostring(entityId), function(data)
            safe(env.OnCollisionEnter, data.entityId)
        end)
    end
    if type(env.OnCollisionExit) == "function" then
        EventBus.on("collision_exit_" .. tostring(entityId), function(data)
            safe(env.OnCollisionExit, data.entityId)
        end)
    end
end
        )", sol::script_pass_on_error);
        if (!r.valid()) {
            sol::error err = r;
            LOG_ERROR("[ScriptSystem] __wireNamedHandlers init : {}", err.what());
        }
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
    // Runtime nova
    // -------------------------------------------------------------------------
    void updateNovaRuntime(float dt) {
        callTableMethod("Timer",        "update",  dt);
        callTableMethod("Scheduler",    "update",  dt);
        callTableMethod("Tween",        "update",  dt);
        callTableMethod("Game",         "_update", dt);
        callTableMethod("Camera",       "_update", dt);
        callTableMethod("World",        "_update", dt);
        callTableMethod("Effect",       "update",  dt);
        callTableMethod("Cooldown",     "update",  dt);
        callTableMethod("Sequence",     "_update", dt);
        callTableMethod("Anim",         "_update", dt);
        callTableMethod("Trigger",      "_update", dt);
        callTableMethod("Nav",          "_update", dt);
        callTableMethod("Spatial",      "_update", dt);
        callTableMethod("SceneFX",      "_update", dt);
        callTableMethod("Particles",    "_update", dt);
        callTableMethod("Projectile",   "_update", dt);
        callTableMethod("Debug",        "_update", dt);
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
        std::set<std::pair<u64,u64>> current;

        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
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
            auto it      = m_prevKeyState.find(name);
            bool wasPrev = (it != m_prevKeyState.end()) ? it->second : false;
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
                sol::error err = res;
                LOG_ERROR("[ScriptSystem] Chargement '{}' : {}", script->scriptPath, err.what());
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
                if (!wr.valid()) {
                    sol::error err = wr;
                    LOG_WARN("[ScriptSystem] wireHandlers '{}' : {}", script->scriptPath, err.what());
                }
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
    // Scripts globaux
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

            // Wire les named handlers pour les scripts globaux (entityId=0 = tous)
            sol::protected_function wire = m_lua["__wireNamedHandlers"];
            if (wire.valid()) {
                auto wr = wire(gs.env, u64{0});
                if (!wr.valid()) {
                    sol::error err = wr;
                    LOG_WARN("[ScriptSystem] wireHandlers global '{}' : {}", gs.path, err.what());
                }
            }

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
    // Helpers
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
